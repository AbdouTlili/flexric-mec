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
#include <algorithm>
#include "res_virt.h"

using namespace flexric::recursive_slicing;

void test_sort()
{
  std::vector<nvs> nvs = {
    {2, 0.4},
    {1, 0.5}
  };
  std::sort(nvs.begin(), nvs.end());
  assert(nvs.at(0).id == 1);
  assert(nvs.at(1).id == 2);
}

void test_merge()
{
  std::vector<nvs> one = {
    {2, 0.4},
    {3, 0.5}
  };
  std::vector<nvs> two = {
    {2, 0.4},
    {1, 0.5}
  };
  std::vector<nvs> result = res_virt::merge(one, two);
  assert(result.size() == 3);
  assert(result.at(0).id == 1);
  assert(result.at(1).id == 2);
  assert(result.at(2).id == 3);

  std::vector<nvs> three = { { 0, 1.0 } };
  std::vector<nvs> four =  { { 1, 0.2 } };
  std::vector<nvs> result2 = res_virt::merge(three, four);
  assert(result2.size() == 2);
  assert(result2.at(0).id == 0);
  assert(result2.at(1).id == 1);
}

void test_admission_control1()
{
  res_virt rv(1, 1.0, 10);
  std::vector<nvs> real_slices = { {10, 1.0} };
  rv.update(real_slices);
  std::vector<nvs> u1 = { {0, 1.0 } };
  assert(rv.admission_control(u1));
  rv.apply(u1);

  std::vector<nvs> u2 = { {1, 0.2 } };
  assert(!rv.admission_control(u2));

  //rv.dump_real_slices();
  std::vector<nvs> real = rv.get_real_slices();
  assert(real.size() == 1);
  assert(real.at(0).id == 10);
  assert(real.at(0).type == nvs_type::capacity);
  assert(real.at(0).cap.pctReserved == 1.0);

  //rv.dump_virt_slices();
  std::vector<nvs> virt = rv.get_virt_slices();
  assert(virt.size() == 1);
  assert(virt.at(0).id == 0);
  assert(virt.at(0).type == nvs_type::capacity);
  assert(virt.at(0).cap.pctReserved == 1.0);
}

void test_admission_control2()
{
  res_virt rv(1, 0.5, 10);
  std::vector<nvs> real_slices = { { 10, 0.5 } };
  rv.update(real_slices);
  std::vector<nvs> u1 = { { 0, 0.5 }, { 1, 2.5, 5 } };
  assert(rv.admission_control(u1));
  rv.apply(u1);

  //rv.dump_real_slices();
  std::vector<nvs> real = rv.get_real_slices();
  assert(real.size() == 2);
  assert(real.at(0).id == 10);
  assert(real.at(0).type == nvs_type::capacity);
  assert(real.at(0).cap.pctReserved == 0.25);
  assert(real.at(1).id == 11);
  assert(real.at(1).type == nvs_type::rate);
  assert(real.at(1).rate.mbpsRequired == 2.5);
  assert(real.at(1).rate.mbpsReference == 10);

  //rv.dump_virt_slices();
  std::vector<nvs> virt = rv.get_virt_slices();
  assert(virt.size() == 2);
  assert(virt.at(0).id == 0);
  assert(virt.at(0).type == nvs_type::capacity);
  assert(virt.at(0).cap.pctReserved == 0.5);
  assert(virt.at(1).id == 1);
  assert(virt.at(1).type == nvs_type::rate);
  assert(virt.at(1).rate.mbpsRequired == 2.5);
  assert(virt.at(1).rate.mbpsReference == 5);
}

void test_no_slicing()
{
  res_virt rv(1, 0.5, 10);

  std::vector<nvs> real = rv.get_real_slices();
  assert(real.size() == 1);
  assert(real.at(0).id == 10);
  assert(real.at(0).type == nvs_type::capacity);
  assert(real.at(0).cap.pctReserved == 0.5);

  std::vector<nvs> virt = rv.get_virt_slices();
  assert(virt.size() == 0);
}

void test_update_ues()
{
  res_virt rv(1, 0.5, 10);
  std::vector<ue_assoc> upd = { { 0x1234, 0 }, { 0xdead, 2 } };
  rv.update_ues(upd);
  std::vector<ue_assoc> f1 = rv.get_virt_ue_assoc();
  assert(f1.size() == 2);
  assert(f1.at(0).rnti == 0x1234 && f1.at(0).dl == 0);
  assert(f1.at(1).rnti == 0xdead && f1.at(1).dl == 2);
  std::vector<ue_assoc> f2 = rv.get_real_ue_assoc();
  assert(f2.size() == 2);
  assert(f2.at(0).rnti == 0x1234 && f2.at(0).dl == 10);
  assert(f2.at(1).rnti == 0xdead && f2.at(1).dl == 12);
  std::vector<ue_assoc> ues = { { 0x1234, 10 }, {0xcafe, 23}, { 0xdead, 12 } };
  assert(rv.verify_ues(ues));

  std::vector<ue_assoc> upd2 = { { 0x1234, 2 } };
  rv.update_ues(upd2);
  std::vector<ue_assoc> f3 = rv.get_virt_ue_assoc();
  assert(f3.size() == 2);
  assert(f3.at(0).rnti == 0x1234 && f3.at(0).dl == 2);
  assert(f3.at(1).rnti == 0xdead && f3.at(1).dl == 2);
}

void test_delete()
{
  res_virt rv(1, 0.5, 10);
  std::vector<nvs> u1 = { { 0, 0.5 }, { 1, 2.5, 5 } };
  assert(rv.admission_control(u1));
  rv.apply(u1);

  std::vector<ue_assoc> upd = { { 0x1234, 0 }, { 0xdead, 2 } };
  assert(!rv.check_slices_exist(upd));
  upd.at(1).dl = 1;
  assert(rv.check_slices_exist(upd));
  rv.update_ues(upd);

  std::vector<uint32_t> del = { 1 };
  std::vector<uint32_t> nodel = { 2 };
  assert(!rv.check_slices_exist(nodel));
  assert(rv.check_slices_exist(del));
  std::vector<ue_assoc> reassoc = rv.apply_delete(del);
  assert(reassoc.size() == 1);
  assert(reassoc.at(0).rnti == 0xdead);
  assert(reassoc.at(0).dl == 10);
  std::vector<ue_assoc> f1 = rv.get_virt_ue_assoc();
  assert(f1.size() == 2);
  assert(f1.at(0).rnti == 0x1234 && f1.at(0).dl == 0);
  assert(f1.at(1).rnti == 0xdead && f1.at(1).dl == 0);
}

void test_reset()
{
  res_virt rv(1, 0.5, 10);
  std::vector<nvs> u1 = { { 0, 0.5 }, { 1, 2.5, 5 } };
  assert(rv.admission_control(u1));
  rv.apply(u1);

  std::vector<ue_assoc> upd = { { 0x1234, 0 }, { 0xdead, 1 } };
  assert(rv.check_slices_exist(upd));
  rv.update_ues(upd);

  std::vector<uint32_t> del = rv.reset();
  assert(del.size() == 1);
  assert(del.at(0) == 11);
  std::vector<ue_assoc> f1 = rv.get_virt_ue_assoc();
  assert(f1.size() == 2);
  assert(f1.at(0).rnti == 0x1234 && f1.at(0).dl == 0);
  assert(f1.at(1).rnti == 0xdead && f1.at(1).dl == 0);
}

int main()
{
  test_sort();
  test_merge();
  test_admission_control1();
  test_admission_control2();
  test_no_slicing();
  test_update_ues();
  test_delete();
  test_reset();
}
