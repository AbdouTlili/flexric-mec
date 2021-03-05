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

#ifndef CONN_MANAGER_
#define CONN_MANAGER_

extern "C" {
#include "e2ap_server.h"
#include "flexric_agent.h"
#include "rrc_event_app.h"
}

#include "mac_stats_manager.h"
#include "rslicing_manager.h"

#include <vector>
#include <string>
#include <memory>
#include <thread>

typedef std::vector<std::pair<std::string, uint16_t>> endpoint_list_t;

namespace flexric {
namespace recursive_slicing {

class conn_manager {
public:
  conn_manager(e2ap_ric_t& ric, e2sm_rrc_event_app_t& ue_events, endpoint_list_t& endpoints);

private:
  void node_connection(const e2node_event_t* event);
  void ue_attach(const ran_t* ran, uint16_t rnti, byte_array_t msg);
  static bool ran_function_has_oid(const ran_function_t* rf, const char* oid);

  e2node_event_t _e2node;
  e2ap_agent_t* _agent;
  std::thread _agent_thread;

  std::unique_ptr<mac_stats_manager> _mac_stats_manager;
  std::unique_ptr<rslicing_manager> _rslicing_manager;

  e2sm_rrc_event_app_t& _ue_events;
  endpoint_list_t _endpoints;
  e2ap_ric_t& _ric;
};

}
}

#endif /* CONN_MANAGER_ */
