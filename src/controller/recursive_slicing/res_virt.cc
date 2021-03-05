/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under
 * the OAI Public License, Version 1.1  (the "License"); you may not use this file
 * except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.openairinterface.org/?page_id=698
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *-------------------------------------------------------------------------------
 * For more information about the OpenAirInterface (OAI) Software Alliance:
 *      contact@openairinterface.org
 */

#include <cassert>
#include <cmath>
#include <iostream>
#include <algorithm>
#include <limits>
#include "res_virt.h"

namespace flexric {
namespace recursive_slicing {

static
bool areEqual(float a, float b)
{
  /* a, b in [0,1] */
  return fabs(a - b) <= 0.00005;
}

res_virt::res_virt(int ep_id, float res_share, float max_thr)
  : _p(res_share), _max_thr(max_thr)
{
  assert(ep_id > 0);
  _ep_id = (uint32_t) ep_id;
}

bool res_virt::real_slice_id_belongs(uint32_t real) const
{
  return real >= _ep_id * 10 && real < (_ep_id + 1) * 10;
}

uint32_t res_virt::slice_id_virt(uint32_t real) const
{
  assert(real_slice_id_belongs(real));
  return real - _ep_id * 10;
}

uint32_t res_virt::slice_id_real(uint32_t virt) const
{
  assert(virt < 10);
  return virt + _ep_id * 10;
}

std::vector<uint32_t> res_virt::reset()
{
  std::vector<uint32_t> del;
  for (const nvs& s : _virt_slices)
    if (s.id != 0)
      del.push_back(slice_id_real(s.id));
  _virt_slices.clear();
  std::for_each(_virt_ue_assoc.begin(), _virt_ue_assoc.end(),
      [](ue_assoc& uea) { uea.dl = 0; }
  );
  return del;
}

void res_virt::update(const std::vector<nvs>& new_slices)
{
  std::vector<nvs> real_slices;
  for (const auto &ns : new_slices)
    if (real_slice_id_belongs(ns.id))
      real_slices.push_back(ns);

  /* no virtual slice -> should only have one real slice with resources close
   * to res_share */
  if (_virt_slices.size() == 0) {
    assert(real_slices.size() == 1);
    assert(real_slices.at(0).id == _ep_id * 10);
    assert(real_slices.at(0).type == nvs_type::capacity);
    assert(areEqual(real_slices.at(0).cap.pctReserved, _p));
    return;
  }

  if (_virt_slices.size() != real_slices.size()) {
    /* it might happen that virtual slices and real slices are inconsistent:
     * e.g., when deleting a slice that necessitates UE reassociation, in which
     * case the rslicing manager would need to send two commands while we
     * update here "atomically" */
    std::cerr << "*** inconsistent number of virtual slices (" << _virt_slices.size()
              << ") and real slices (" << real_slices.size() << "), aborting update\n";
    return;
  }
  auto itv = _virt_slices.begin();
  for (auto itr = real_slices.begin(); itr != real_slices.end(); itr++, itv++) {
    const nvs& r = *itr;
    const nvs& v = *itv;
    assert(virtualize(r).id == v.id);
    assert(virtualize(r).type == v.type);
    switch (v.type) {
      case nvs_type::rate:
        assert(virtualize(r).rate == v.rate);
        break;
      case nvs_type::capacity:
        assert(virtualize(r).cap == v.cap);
        break;
      default:
        assert(0 && "illegal slice type");
        break;
    }
  }
}

void res_virt::update_ues(const std::vector<ue_assoc>& virt_ues)
{
  for (const ue_assoc& uea : virt_ues) {
    assert(uea.dl < 10);
    const auto it = std::find_if(_virt_ue_assoc.begin(), _virt_ue_assoc.end(),
        [&uea] (const ue_assoc& vue_assoc) {
          return uea.rnti == vue_assoc.rnti;
        }
    );
    if (it == _virt_ue_assoc.end())
      _virt_ue_assoc.push_back(uea);
    else
      it->dl = uea.dl;
  }
}

bool res_virt::verify_ues(const std::vector<ue_assoc>& real_ues)
{
  for (const ue_assoc& uea : real_ues) {
    std::cout << __func__ << "(): rnti " << uea.rnti << " dl " << uea.dl << "\n";
    if (real_slice_id_belongs(uea.dl)) {
      /* this UE belongs to us, verify that we know it */
      const auto it = std::find_if(_virt_ue_assoc.begin(), _virt_ue_assoc.end(),
          [&uea] (const ue_assoc& vue_assoc) {
            return uea.rnti == vue_assoc.rnti;
          }
      );
      if(it == _virt_ue_assoc.end())
        return false;
    }
  }
  return true;
}

bool res_virt::admission_control(const std::vector<nvs>& virt_updates)
{
  for (const auto& v : virt_updates) {
    if (!admission_control_vslice(v))
      return false;
  }
  std::vector<nvs> resulting = merge(_virt_slices, virt_updates);
  float sum = 0.0f;
  for (const auto& r : resulting)
    sum += r.get_resources();
  if (sum > 1) {
    std::cerr << " sum of all slices " << sum << " > 1\n";
    return false;
  }
  return true;
}

bool res_virt::check_slices_exist(const std::vector<uint32_t>& virt_updates)
{
  for (const uint32_t& dl : virt_updates) {
    const auto it = std::find_if(_virt_slices.begin(), _virt_slices.end(),
        [dl](const nvs& s) { return s.id == dl; }
    );
    if (it == _virt_slices.end())
      return false; // do not know this slice
  }
  return true; // know all these slices
}

bool res_virt::check_slices_exist(const std::vector<ue_assoc>& virt_updates)
{
  std::vector<uint32_t> list;
  for (const ue_assoc& uea : virt_updates)
    list.push_back(uea.dl);
  return check_slices_exist(list);
}

std::vector<nvs> res_virt::get_real_slices()
{
  std::vector<nvs> real;
  if (_virt_slices.size() == 0) {
    /* no virtual slice -> a single NVS capacity slice in real */
    real.emplace_back(_ep_id * 10, _p);
  } else {
    for (const auto &v : _virt_slices)
      real.push_back(realize(v));
  }
  return real;
}

std::vector<nvs> res_virt::get_virt_slices()
{
  return _virt_slices;
}

void res_virt::apply(const std::vector<nvs>& virt_updates)
{
  _virt_slices = merge(_virt_slices, virt_updates);
}

std::vector<ue_assoc> res_virt::apply_delete(const std::vector<uint32_t>& virt_updates)
{
  std::vector<ue_assoc> reassoc;
  for (const uint32_t& dl : virt_updates) {
    const auto it = std::find_if(_virt_slices.begin(), _virt_slices.end(),
        [dl](const nvs& s) { return s.id == dl; }
    );
    assert(it != _virt_slices.end());
    _virt_slices.erase(it);

    for (ue_assoc& uea : _virt_ue_assoc) {
      if (uea.dl == dl) {
        // put back in (virtual) default slice
        reassoc.emplace_back(uea.rnti, slice_id_real(0));
        uea.dl = 0;
      }
    }
  }
  return reassoc;
}

std::vector<ue_assoc> res_virt::get_real_ue_assoc() const
{
  std::vector<ue_assoc> real_assoc;
  for (const ue_assoc& uea : _virt_ue_assoc)
    real_assoc.emplace_back(uea.rnti, slice_id_real(uea.dl));
  return real_assoc;
}

std::vector<ue_assoc> res_virt::get_virt_ue_assoc() const
{
  return _virt_ue_assoc;
}

nvs res_virt::realize(const nvs& v)
{
  nvs r = v * _p;
  r.id = slice_id_real(v.id);
  return r;
}

nvs res_virt::virtualize(const nvs& r)
{
  nvs v = r / _p;
  v.id = slice_id_virt(r.id);
  return v;
}

void res_virt::dump_virt_slices()
{
  std::cout << "dump_virt_slices(): ep_id " << _ep_id << " res_share "
            << _p << " max_thr " << _max_thr << "\n";
  for (const auto& v: _virt_slices)
    std::cout << "  " << v << "\n";
}

void res_virt::dump_virt_ue_assoc() const
{
  std::cout << __func__ << "():\n";
  for (const ue_assoc& uea: _virt_ue_assoc)
    std::cout << "  RNTI " << uea.rnti << " DL " << uea.dl << "\n";
}

void res_virt::dump_real_slices()
{
  std::cout << "dump_real_slices(): ep_id " << _ep_id << " res_share "
            << _p << " max_thr " << _max_thr << "\n";
  for (const auto& v: _virt_slices)
    std::cout << "  " << realize(v) << "\n";
}

bool res_virt::admission_control_vslice(const nvs& slice)
{
  if (slice.id >= 10) {
    std::cerr << "slice ID " << slice.id << " too high\n";
    return false;
  }
  if (slice.get_resources() > 1) {
    std::cerr << "slice ID " << slice.id << " total resources " << slice.get_resources() << " > 1\n";
    return false;
  }
  switch (slice.type) {
    case nvs_type::rate:
      if (slice.rate.mbpsReference > _p * _max_thr) {
        std::cerr << "slice ID " << slice.id << " mbpsReference " << slice.rate.mbpsReference
                  << " > _p * _max_thr = " << _p * _max_thr << "\n";
        return false;
      }
      if (slice.rate.mbpsRequired > slice.rate.mbpsReference) {
        std::cerr << "slice ID " << slice.id << " mbpsRequired " << slice.rate.mbpsReference
                  << " > mbpsReference\n";
        return false;
      }
      break;
    case nvs_type::capacity:
      if (slice.cap.pctReserved > 1) {
        std::cerr << "slice ID " << slice.id << " pctReserved " << slice.cap.pctReserved << " > 1\n";
        return false;
      }
      break;
    default:
      assert(0 && "illegal slice type");
      break;
  }
  return true;
}

std::vector<nvs> res_virt::merge(std::vector<nvs> ex, std::vector<nvs> upd)
{
  // copy of the arguments so we can sort without problem
  std::sort(upd.begin(), upd.end());
  std::sort(ex.begin(), ex.end());
  std::vector<nvs> result;
  std::set_union(upd.begin(), upd.end(), ex.begin(), ex.end(), std::back_inserter(result));
  return result;
}

}
}
