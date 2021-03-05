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

#include <iostream>
#include <thread>

#include "conn_manager.h"

#include "rrc_event_reader.h"

extern "C" {
#include "mac_stats_defs.h"
#include "rslicing_defs.h"
#include "rrc_event_defs.h"
#include "rrc_conf_defs.h"
#include "e2ap_multi_control.h"
}

namespace flexric {
namespace recursive_slicing {

conn_manager::conn_manager(e2ap_ric_t& ric, e2sm_rrc_event_app_t& ue_events, endpoint_list_t& endpoints)
  : _ue_events(ue_events), _ric(ric)
{
  _e2node.ran = NULL;
  _e2node.assoc_id = 0;
  assert(endpoints.size() == 2);
  for (const auto& e: endpoints)
    _endpoints.push_back(e);

  ed_t ev1 = subscribe_e2node_connect(&ric.events,
      [](const e2node_event_t* event, void* object) {
        static_cast<conn_manager*>(object)->node_connection(event);
      }, this);
  (void) ev1;
  ue_ed_t ev2 = subscribe_ue_attach(&_ue_events,
      [] (const ran_t* ran, uint16_t rnti, byte_array_t msg, void* object) {
        static_cast<conn_manager*>(object)->ue_attach(ran, rnti, msg);
      }, this);
  (void) ev2;
}

void conn_manager::node_connection(const e2node_event_t* event)
{
  std::cout << "new RAN with ID " << event->ran->generated_ran_key
            << " on assoc ID " << event->assoc_id << " connected\n";
  assert(_e2node.ran == NULL && "can only handle one agent currently");
  _e2node.ran = event->ran;
  _e2node.assoc_id = event->assoc_id;

  const ran_t* ran = _e2node.ran;
  assert(ran->entities.split == SPLIT_NONE && ran->entities.nosplit.full);

  /* TODO: clone e2_setup_request, filter SMs */
  const e2_setup_request_t* sr = &ran->entities.nosplit.full->e2_setup_request;
  ran_function_t* mac_stats = NULL;
  ran_function_t* rslicing = NULL;
  //ran_function_t* rrc_conf = NULL;
  for (size_t i = 0; i < sr->len_rf; ++i) {
    ran_function_t* rf = &sr->ran_func_item[i];
    if (ran_function_has_oid(rf, mac_stats_oid)) {
      assert(!mac_stats);
      mac_stats = rf;
    } else if (ran_function_has_oid(rf, rslicing_oid)) {
      assert(!rslicing);
      rslicing = rf;
    }
    //else if (ran_function_has_oid(rf, rrc_conf_oid)) {
    //  assert(!rrc_conf);
    //  rrc_conf = rf;
    //}
  }
  assert(mac_stats && "need mac_stats\n");
  const size_t len_rf = 2;
  ran_function_t rfs[len_rf];
  rfs[0] = *mac_stats;
  rfs[1] = *rslicing;
  //rfs[2] = *rrc_conf;
  e2_setup_request_t filtered_e2_sr = {
    .id = sr->id,
    .ran_func_item = rfs,
    .len_rf = len_rf,
    .comp_conf_update = NULL,
    .len_ccu = 0
  };

  /* main connection should actually only be a dummy without any connection.
   * Note: the IP should be reachable, or the agent will block on
   * sctp_sendmsg() */
  _agent = e2ap_init_agent("127.0.0.1", 1234, sr->id);
  _mac_stats_manager = std::make_unique<mac_stats_manager>(_ric, _e2node. assoc_id, _agent, mac_stats);
  _rslicing_manager = std::make_unique<rslicing_manager>(_ric, _ue_events, _e2node.assoc_id, _agent, rslicing);

  for (const auto& e: _endpoints) {
    ep_id_t ep_id = add_endpoint(_agent, e.first.c_str(), e.second, &filtered_e2_sr);
    float res = ep_id == 1 ? 0.25 : 0.74; // leave 0.01 for default slice
    _rslicing_manager->add_endpoint(ep_id, res, 17.5);
    std::cout << "add controller (" << e.first << ":" << e.second << "): ep_id " << ep_id << "\n";
  }
  _rslicing_manager->init_userplane();

  _agent_thread = std::thread([](e2ap_agent_t* agent) { e2ap_start_agent(agent); }, _agent);
}

void conn_manager::ue_attach(const ran_t* ran, uint16_t rnti, byte_array_t msg)
{
  rrc_event_IndicationMessage_table_t im = rrc_event_IndicationMessage_as_root(msg.buf);
  int selectedPlmnId = rrc_event_IndicationMessage_selectedPlmnId(im);
  assert(selectedPlmnId == 1 || selectedPlmnId == 2);
  endpoint_associate_rnti(_agent, selectedPlmnId, rnti);
  std::cout << "UE RNTI " << std::hex << rnti << std::dec
            << " selectedPlmnId " << selectedPlmnId
            << " associated to endpoint " << selectedPlmnId << "\n";
}

bool conn_manager::ran_function_has_oid(const ran_function_t* rf, const char* oid)
{
  if (!rf->oid)
    return false;
  const std::string rf_oid{(char*)rf->oid->buf, rf->oid->len};
  return rf_oid == oid;
}

}
}
