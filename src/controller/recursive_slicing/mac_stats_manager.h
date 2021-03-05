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

#ifndef MAC_STATS_MANAGER_H
#define MAC_STATS_MANAGER_H

extern "C" {
#include "e2ap_server.h"
#include "flexric_agent.h"
}

#include <vector>

namespace flexric {
namespace recursive_slicing {

class mac_stats_manager {
public:
  mac_stats_manager(e2ap_ric_t& ric, int assoc_id, e2ap_agent_t* ag, ran_function_t* mac_rf);

private:
  bool handle_sub_req(e2ap_agent_t* ag, subscription_t* sub);
  void handle_sub_resp(const ric_subscription_response_t* r);
  void handle_ind(const ric_indication_t* ind);
  byte_array_t slice_indication_msg(subscription_t* sub, byte_array_t msg);

  std::vector<subscription_t*> _subs;

  e2ap_ric_t& _ric;
  int _assoc_id;
  e2ap_agent_t* _ag;
};

}
}

#endif /* MAC_STATS_MANAGER_H */
