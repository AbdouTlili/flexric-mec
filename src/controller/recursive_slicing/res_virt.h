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

#ifndef RES_VIRT_H
#define RES_VIRT_H

#include <vector>
#include <cstdint>
#include "nvs.h"

namespace flexric {
namespace recursive_slicing {

struct ue_assoc {
  ue_assoc(uint16_t rnti, uint32_t dl) : rnti(rnti), dl(dl) {}
  uint16_t rnti;
  uint32_t dl;
};

class res_virt {
public:
  res_virt(int ep_id, float res_share, float max_thr);

  bool real_slice_id_belongs(uint32_t real) const;

  /* slice ID conversion */
  uint32_t slice_id_virt(uint32_t real) const;
  uint32_t slice_id_real(uint32_t virt) const;

  /* reset slicing algorithm, returns real slice IDs that have to be deleted */
  std::vector<uint32_t> reset();

  /* update with real slices coming from the UP */
  void update(const std::vector<nvs>& new_slices);

  /* update with virtual slices ids coming from controller */
  void update_ues(const std::vector<ue_assoc>& virt_ues);
  /* verify with real slice ids coming from UP */
  bool verify_ues(const std::vector<ue_assoc>& real_ues);

  bool check_slices_exist(const std::vector<uint32_t>& virt_updates);
  bool check_slices_exist(const std::vector<ue_assoc>& virt_updates);

  /* admission control check */
  bool admission_control(const std::vector<nvs>& virt_updates);
  /* application of virtual updates (assumes AC passes) */
  void apply(const std::vector<nvs>& virt_updates);
  /* application of delete operation */
  std::vector<ue_assoc> apply_delete(const std::vector<uint32_t>& virt_updates);

  /* getters for real slices */
  std::vector<nvs> get_real_slices();
  float get_real_load();
  void dump_real_slices();

  /* getters for virtual slices */
  std::vector<nvs> get_virt_slices();
  float get_virt_load();
  void dump_virt_slices();

  std::vector<ue_assoc> get_real_ue_assoc() const;
  std::vector<ue_assoc> get_virt_ue_assoc() const;
  void dump_virt_ue_assoc() const;
  
  // public for testing
  static std::vector<nvs> merge(const std::vector<nvs> ex, const std::vector<nvs> upd);

private:
  bool admission_control_vslice(const nvs& slice);

  nvs realize(const nvs& v);
  nvs virtualize(const nvs& r);

  uint32_t _ep_id;
  float _p;
  float _max_thr;

  std::vector<nvs> _virt_slices;
  std::vector<ue_assoc> _virt_ue_assoc;
};

}
}

#endif /* RES_VIRT_H */
