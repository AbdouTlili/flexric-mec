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

#include <vector>
#include <string>

extern "C" {
#include "e2ap_server.h"
#include "rrc_event_app.h"
}

#include "conn_manager.h"

int main()
{
  e2ap_ric_t ric = {};
  e2ap_init_server(&ric, "127.0.0.1", 36421);

  endpoint_list_t endpoints;
  endpoints.emplace_back(std::make_pair("127.0.0.1", 10001));
  endpoints.emplace_back(std::make_pair("127.0.0.1", 10002));

  uint16_t ric_req_id = 0;
  e2sm_rrc_event_app_t ue_event_app;
  memset(&ue_event_app, 0, sizeof(ue_event_app));
  e2sm_rrc_event_app_init(&ue_event_app, &ric, ric_req_id++);

  auto conn_manager = flexric::recursive_slicing::conn_manager(ric, ue_event_app, endpoints);

  e2ap_start_server(&ric); // blocking
  e2ap_free_server(&ric);
  return 0;
}
