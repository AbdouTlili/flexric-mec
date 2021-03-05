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

#ifndef STATS_MANAGER_H_
#define STATS_MANAGER_H_

#include <set>
#include <cstdint>

#include "component.h"

namespace flexric {
namespace flexran {
namespace app {

class stats_manager : public component {
public:
  stats_manager(e2ap_ric_t& ric, e2sm_rrc_event_app_t& ue_events, const rib::Rib& rib, uint16_t ric_req);

  std::string get_ran_as_json() const;
  static std::string format_date_time(std::chrono::time_point<std::chrono::system_clock> t);

private:
  void node_connection(const e2node_event_t* event);
  void ue_attach(const ran_t* ran, uint16_t rnti, byte_array_t msg);

  static bool ran_function_has_oid(const ran_function_t* rf, const char* oid);

  static std::string ran_as_json(const ran_t* ran);
  static std::string global_e2_node_id_as_json(const global_e2_node_id_t* id);
  static std::string e2_setup_request_as_json(const e2_setup_request_t* sr);
  static std::string entities_as_json(const entities_t* es);

  void handle_mac_stats(uint64_t ran_key, int assoc_id, const ran_function_t* rf);
  void handle_pdcp_stats(uint64_t ran_key, int assoc_id, const ran_function_t* rf);
  void handle_rlc_stats(uint64_t ran_key, int assoc_id, const ran_function_t* rf);
  void handle_rrc_stats(uint64_t ran_key, int assoc_id, const ran_function_t* rf);
  void handle_rrc_conf(uint64_t ran_key, int assoc_id, const ran_function_t* rf);

  static void print_sub_success(const ric_subscription_response_t* r, void* object);
  static void print_sub_failure(const ric_subscription_failure_t* r, void* object);
};

}
}
}

#endif /* STATS_MANAGER_H_ */
