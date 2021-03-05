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

#ifndef RSLICING_MANAGER_H
#define RSLICING_MANAGER_H

extern "C" {
#include "e2ap_server.h"
#include "flexric_agent.h"
#include "rrc_event_app.h"
#include "rslicing_reader.h"
}

#include <unordered_map>
#include <memory>
#include "res_virt.h"
#include "nvs.h"

struct flatcc_builder;

namespace flexric {
namespace recursive_slicing {

typedef struct service {
  service(std::shared_ptr<res_virt> rv, subscription_t* sub) : rv(rv), sub(sub) {}
  std::shared_ptr<res_virt> rv;
  subscription_t* sub;
} service_t;

class rslicing_manager {
public:
  rslicing_manager(e2ap_ric_t& ric, e2sm_rrc_event_app_t& ue_events, int assoc_id, e2ap_agent_t* ag, ran_function_t* slicing_rf);
  void add_endpoint(ep_id_t ep_id, float max_resources, float max_throughput);
  void init_userplane();

private:
  void ue_attach(const ran_t* ran, uint16_t rnti, byte_array_t msg);

  bool handle_sub_req(e2ap_agent_t* ag, subscription_t* sub);
  void handle_ind(const ric_indication_t* ind);

  void handle_ctrl_req(e2ap_agent_t* ag, ep_id_t ep_id, const ric_control_request_t* cr);
  void handle_addmod_command(std::shared_ptr<res_virt> rv, mac_rslicing_AddModSliceCommand_table_t amsc);
  void handle_del_command(std::shared_ptr<res_virt> rv, mac_rslicing_DelSliceCommand_table_t dsc);
  void handle_ue_assoc_command(std::shared_ptr<res_virt> rv, mac_rslicing_UeSliceAssocCommand_table_t uesac);

  void send_addmod_command(const std::vector<nvs>& slices);
  void send_del_command(const std::vector<uint32_t>& del);
  void send_ue_assoc_command(const std::vector<ue_assoc>& assoc);

  byte_array_t create_indication(const std::vector<nvs>& slices, const std::vector<ue_assoc>& ues);
  byte_array_t create_addmod_slice_command(const std::vector<nvs>& addmod_slices);
  byte_array_t create_remove_slice_command(const std::vector<uint32_t>& remove_slices);
  byte_array_t create_ue_assoc_command(const std::vector<ue_assoc>& real_ue_assoc);

  nvs nvs_from_fb(uint32_t id, mac_rslicing_NvsSliceConfig_union_t unvs);
  std::vector<nvs> nvs_slices_from_fb(mac_rslicing_UlDlSliceConfig_table_t t);
  std::vector<ue_assoc> ue_assoc_from_fb(mac_rslicing_UeSliceConfig_table_t t);

  static void add_fb_slice(struct flatcc_builder* B, const nvs& s);
  static void add_sliceconfig_dl(struct flatcc_builder* B, const std::vector<nvs>& slices);

  std::unordered_map<ep_id_t, service_t> _services;

  e2ap_ric_t& _ric;
  e2sm_rrc_event_app_t& _ue_events;
  int _assoc_id;
  e2ap_agent_t* _ag;

  ric_gen_id_t _ric_id;
};

}
}

#endif /* RSLICING_MANAGER_H */
