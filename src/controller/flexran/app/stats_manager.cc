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
#include <string>
#include <iomanip>
#include <sstream>

#include "stats_manager.h"
extern "C" {
#include "mac_stats_msg_enc.h"
#include "mac_stats_msg_dec.h"
#include "pdcp_stats_msg_enc.h"
#include "pdcp_stats_msg_dec.h"
#include "rlc_stats_msg_enc.h"
#include "rlc_stats_msg_dec.h"
#include "rrc_stats_msg_enc.h"
#include "rrc_stats_msg_dec.h"
#include "rrc_conf_msg_enc.h"
#include "rrc_conf_msg_dec.h"
#include "rrc_event_reader.h"
}

namespace flexric {
namespace flexran {
namespace app {

stats_manager::stats_manager(e2ap_ric_t& ric, e2sm_rrc_event_app_t& ue_events, const rib::Rib& rib, uint16_t ric_req)
  : component(ric, ue_events, rib, ric_req)
{
  ed_t ev1 = subscribe_e2node_connect(&ric.events,
      [](const e2node_event_t* event, void* object) {
        static_cast<stats_manager*>(object)->node_connection(event);
      }, this);
  (void) ev1;
  ue_ed_t ev2 = subscribe_ue_attach(&_ue_events,
      [] (const ran_t* ran, uint16_t rnti, byte_array_t msg, void* object) {
        static_cast<stats_manager*>(object)->ue_attach(ran, rnti, msg);
      }, this);
  (void) ev2;
}

void stats_manager::node_connection(const e2node_event_t* event)
{
  std::cout << "new RAN with ID " << event->ran->generated_ran_key
            << " on assoc ID " << event->assoc_id << " connected\n";

  const entity_info_t* e = ran_mgmt_get_entity(event->ran, event->assoc_id);

  for (size_t i = 0; i < e->e2_setup_request.len_rf; ++i) {
    const ran_function_t* rf = &e->e2_setup_request.ran_func_item[i];
    std::cout << "checking RAN func " << rf->id << "\n";
    if (ran_function_has_oid(rf, mac_stats_oid))
      handle_mac_stats(event->ran->generated_ran_key, event->assoc_id, rf);
    if (ran_function_has_oid(rf, pdcp_stats_oid))
      handle_pdcp_stats(event->ran->generated_ran_key, event->assoc_id, rf);
    if (ran_function_has_oid(rf, rlc_stats_oid))
      handle_rlc_stats(event->ran->generated_ran_key, event->assoc_id, rf);
    if (ran_function_has_oid(rf, rrc_stats_oid))
      handle_rrc_stats(event->ran->generated_ran_key, event->assoc_id, rf);
    if (ran_function_has_oid(rf, rrc_conf_oid))
      handle_rrc_conf(event->ran->generated_ran_key, event->assoc_id, rf);
  }
}

void stats_manager::ue_attach(const ran_t* ran, uint16_t rnti, byte_array_t msg)
{
  rrc_event_IndicationMessage_table_t im = rrc_event_IndicationMessage_as_root(msg.buf);
  int selectedPlmnId = rrc_event_IndicationMessage_selectedPlmnId(im);
  std::cout << "UE RNTI " << std::hex << rnti << std::dec << " selectedPlmnId " << selectedPlmnId << "\n";
}

bool stats_manager::ran_function_has_oid(const ran_function_t* rf, const char* oid)
{
  if (!rf->oid)
    return false;
  const std::string rf_oid{(char*)rf->oid->buf, rf->oid->len};
  return rf_oid == oid;
}

void stats_manager::handle_mac_stats(uint64_t ran_key, int assoc_id, const ran_function_t* rf)
{
  const uint32_t mac_stats_period = 100; // ms
  mac_stats_report_style_t* supported_styles;
  const size_t n = mac_stats_decode_ran_function(rf->def, &supported_styles);

  if (n <= 0) {
    std::cout << "no mac stats styles in mac stats SM\n";
    return;
  }

  const mac_stats_report_style_t selected_style = supported_styles[n-1];
  free(supported_styles);

  std::cout << "sending mac_stats subscription (style " << selected_style.type
            << ") for RF " << rf->id << " to assoc_id " << assoc_id << "\n";

  byte_array_t event = mac_stats_encode_event_trigger(mac_stats_period);
  byte_array_t action_def = mac_stats_encode_action_definition(selected_style);
  ric_action_t action = {
    .id = 0,
    .type = RIC_ACT_REPORT,
    .definition = &action_def,
    .subseq_action = NULL
  };
  ric_gen_id_t ric_id = {
    .ric_req_id = _ric_req_id,
    .ric_inst_id = _ric_inst_id++,
    .ran_func_id = rf->id
  };
  ric_subscription_request_t sr = {
    .ric_id = ric_id,
    .event_trigger = event,
    .action = &action,
    .len_action = 1
  };
  std::shared_ptr<rib::node_rib_info> nri = _rib.get_node(ran_key);
  cb_sub_t cb = {
    .sub_resp = print_sub_success,
    .sub_fail = print_sub_failure,
    .ind = [](const auto *i, void* object) {
      static_cast<rib::node_rib_info*>(object)->update_mac_stats(i);
    }
  };
  e2ap_subscribe(&_ric, assoc_id, &sr, cb, nri.get());
  free_byte_array(event);
  free_byte_array(action_def);
}

void stats_manager::handle_pdcp_stats(uint64_t ran_key, int assoc_id, const ran_function_t* rf)
{
  const uint32_t pdcp_stats_period = 100; // ms
  pdcp_stats_report_style_t* supported_styles;
  const size_t n = pdcp_stats_decode_ran_function(rf->def, &supported_styles);

  if (n <= 0) {
    std::cout << "no pdcp stats styles in pdcp stats SM\n";
    return;
  }

  const pdcp_stats_report_style_t selected_style = supported_styles[n-1];
  free(supported_styles);

  std::cout << "sending pdcp_stats subscription (style " << selected_style.type
            << ") for RF " << rf->id << " to assoc_id " << assoc_id << "\n";

  byte_array_t event = pdcp_stats_encode_event_trigger(pdcp_stats_period);
  byte_array_t action_def = pdcp_stats_encode_action_definition(selected_style);
  ric_action_t action = {
    .id = 0,
    .type = RIC_ACT_REPORT,
    .definition = &action_def,
    .subseq_action = NULL
  };
  ric_gen_id_t ric_id = {
    .ric_req_id = _ric_req_id,
    .ric_inst_id = _ric_inst_id++,
    .ran_func_id = rf->id
  };
  ric_subscription_request_t sr = {
    .ric_id = ric_id,
    .event_trigger = event,
    .action = &action,
    .len_action = 1
  };
  std::shared_ptr<rib::node_rib_info> nri = _rib.get_node(ran_key);
  cb_sub_t cb = {
    .sub_resp = print_sub_success,
    .sub_fail = print_sub_failure,
    .ind = [](const auto *i, void* object) {
      static_cast<rib::node_rib_info*>(object)->update_pdcp_stats(i);
    }
  };
  e2ap_subscribe(&_ric, assoc_id, &sr, cb, nri.get());
  free_byte_array(event);
  free_byte_array(action_def);
}

void stats_manager::handle_rlc_stats(uint64_t ran_key, int assoc_id, const ran_function_t* rf)
{
  const uint32_t rlc_stats_period = 100; // ms
  rlc_stats_report_style_t* supported_styles;
  const size_t n = rlc_stats_decode_ran_function(rf->def, &supported_styles);

  if (n <= 0) {
    std::cout << "no rlc stats styles in rlc stats SM\n";
    return;
  }

  const rlc_stats_report_style_t selected_style = supported_styles[n-1];
  free(supported_styles);

  std::cout << "sending rlc_stats subscription (style " << selected_style.type
            << ") for RF " << rf->id << " to assoc_id " << assoc_id << "\n";

  byte_array_t event = rlc_stats_encode_event_trigger(rlc_stats_period);
  byte_array_t action_def = rlc_stats_encode_action_definition(selected_style);
  ric_action_t action = {
    .id = 0,
    .type = RIC_ACT_REPORT,
    .definition = &action_def,
    .subseq_action = NULL
  };
  ric_gen_id_t ric_id = {
    .ric_req_id = _ric_req_id,
    .ric_inst_id = _ric_inst_id++,
    .ran_func_id = rf->id
  };
  ric_subscription_request_t sr = {
    .ric_id = ric_id,
    .event_trigger = event,
    .action = &action,
    .len_action = 1
  };
  std::shared_ptr<rib::node_rib_info> nri = _rib.get_node(ran_key);
  cb_sub_t cb = {
    .sub_resp = print_sub_success,
    .sub_fail = print_sub_failure,
    .ind = [](const auto *i, void* object) {
      static_cast<rib::node_rib_info*>(object)->update_rlc_stats(i);
    }
  };
  e2ap_subscribe(&_ric, assoc_id, &sr, cb, nri.get());
  free_byte_array(event);
  free_byte_array(action_def);
}

void stats_manager::handle_rrc_stats(uint64_t ran_key, int assoc_id, const ran_function_t* rf)
{
  const rrc_stats_report_occasion_e occ = RRC_STATS_REPORT_OCCASION_ONCHANGE;
  rrc_stats_report_style_t* supported_styles;
  const size_t n = rrc_stats_decode_ran_function(rf->def, &supported_styles);

  if (n <= 0) {
    std::cout << "no rrc stats styles in rrc stats SM\n";
    return;
  }

  const rrc_stats_report_style_t selected_style = supported_styles[n-1];
  free(supported_styles);

  std::cout << "sending rrc_stats subscription (style " << selected_style.type
            << ") for RF " << rf->id << " to assoc_id " << assoc_id << "\n";

  byte_array_t event = rrc_stats_encode_event_trigger(occ);
  byte_array_t action_def = rrc_stats_encode_action_definition(selected_style);
  ric_action_t action = {
    .id = 0,
    .type = RIC_ACT_REPORT,
    .definition = &action_def,
    .subseq_action = NULL
  };
  ric_gen_id_t ric_id = {
    .ric_req_id = _ric_req_id,
    .ric_inst_id = _ric_inst_id++,
    .ran_func_id = rf->id
  };
  ric_subscription_request_t sr = {
    .ric_id = ric_id,
    .event_trigger = event,
    .action = &action,
    .len_action = 1
  };
  std::shared_ptr<rib::node_rib_info> nri = _rib.get_node(ran_key);
  cb_sub_t cb = {
    .sub_resp = print_sub_success,
    .sub_fail = print_sub_failure,
    .ind = [](const auto *i, void* object) {
      static_cast<rib::node_rib_info*>(object)->update_rrc_stats(i);
    }
  };
  e2ap_subscribe(&_ric, assoc_id, &sr, cb, nri.get());
  free_byte_array(event);
  free_byte_array(action_def);

}

void stats_manager::handle_rrc_conf(uint64_t ran_key, int assoc_id, const ran_function_t* rf)
{
  const rrc_conf_report_occasion_e occ = RRC_CONF_REPORT_OCCASION_ONCHANGE;
  rrc_conf_report_style_t* supported_styles;
  const size_t n = rrc_conf_decode_ran_function(rf->def, &supported_styles);

  if (n <= 0) {
    std::cout << "no rrc conf styles in rrc conf SM\n";
    return;
  }

  const rrc_conf_report_style_t selected_style = supported_styles[n-1];
  free(supported_styles);

  std::cout << "sending rrc_conf subscription (style " << selected_style.type
            << ") for RF " << rf->id << " to assoc_id " << assoc_id << "\n";

  byte_array_t event = rrc_conf_encode_event_trigger(occ);
  byte_array_t action_def = rrc_conf_encode_action_definition(selected_style);
  ric_action_t action = {
    .id = 0,
    .type = RIC_ACT_REPORT,
    .definition = &action_def,
    .subseq_action = NULL
  };
  ric_gen_id_t ric_id = {
    .ric_req_id = _ric_req_id,
    .ric_inst_id = _ric_inst_id++,
    .ran_func_id = rf->id
  };
  ric_subscription_request_t sr = {
    .ric_id = ric_id,
    .event_trigger = event,
    .action = &action,
    .len_action = 1
  };
  std::shared_ptr<rib::node_rib_info> nri = _rib.get_node(ran_key);
  cb_sub_t cb = {
    .sub_resp = print_sub_success,
    .sub_fail = print_sub_failure,
    .ind = [](const auto *i, void* object) {
      static_cast<rib::node_rib_info*>(object)->update_rrc_conf(i);
    }
  };
  e2ap_subscribe(&_ric, assoc_id, &sr, cb, nri.get());
  free_byte_array(event);
  free_byte_array(action_def);

}

void stats_manager::print_sub_success(const ric_subscription_response_t* r, void* object)
{
  (void) object;
  std::cout << "subscription to RF " << r->ric_id.ran_func_id << " succeeded\n";
}

void stats_manager::print_sub_failure(const ric_subscription_failure_t* f, void* object)
{
  (void) object;
  std::cout << "subscription to RF " << f->ric_id.ran_func_id << " failed\n";
}

std::string stats_manager::format_date_time(std::chrono::time_point<std::chrono::system_clock> t)
{
  std::ostringstream oss;
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t.time_since_epoch()) % 1000;
  std::time_t time = std::chrono::system_clock::to_time_t(t);
  oss << std::put_time(std::localtime(&time), "%Y-%m-%dT%H:%M:%S.")
    << std::setfill('0') << std::setw(3) << ms.count();
  return oss.str();
}

std::string stats_manager::global_e2_node_id_as_json(const global_e2_node_id_t* id)
{
  (void) id;
  return "\"TODO\"";
}

std::string stats_manager::e2_setup_request_as_json(const e2_setup_request_t* sr)
{
  std::string json = "{\"id\":" + global_e2_node_id_as_json(&sr->id);
  json += ",\"ranFunctions\":[";
  for (size_t i = 0; i < sr->len_rf; ++i) {
    const ran_function_t* rf = &sr->ran_func_item[i];
    if (i > 0)
      json += ",";
    json += "{\"id\":" + std::to_string(rf->id);
    json += ",\"rev\":" + std::to_string(rf->rev);
    if (rf->oid) {
      const std::string oid((char*)rf->oid->buf, rf->oid->len);
      json += ",\"oid\":\"" + oid + "\"";
    }
    json += "}";
  }
  json += "]}";
  return json;
}

std::string stats_manager::entities_as_json(const entities_t* es)
{
  switch (es->split) {
    case SPLIT_NONE: {
        const entity_info_t* e = es->nosplit.full;
        return "[{\"role\":\"Full\",\"assoc_id\":" + std::to_string(e->assoc_id)
             + ",\"e2_setup_request\":" + e2_setup_request_as_json(&e->e2_setup_request)
             + "}]";
      }
      break;
    case SPLIT_F1: {
        const entity_info_t* cu = es->f1.cu;
        const entity_info_t* du = es->f1.du;
        return "[{\"role\":\"CU\",\"assoc_id\":" + std::to_string(cu->assoc_id)
             + ",\"e2_setup_request\":" + e2_setup_request_as_json(&cu->e2_setup_request)
             + "},{\"role\":\"DU\",\"assoc_id\":" + std::to_string(du->assoc_id)
             + ",\"e2_setup_request\":" + e2_setup_request_as_json(&du->e2_setup_request)
             + "}]";
      }
      break;
    case SPLIT_F1_E1: {
        const entity_info_t* cu_cp = es->f1e1.cu_cp;
        const entity_info_t* cu_up = es->f1e1.cu_up;
        const entity_info_t* du = es->f1e1.du;
        return "[{\"role\":\"CU-CP\",\"assoc_id\":" + std::to_string(cu_cp->assoc_id)
             + ",\"e2_setup_request\":" + e2_setup_request_as_json(&cu_cp->e2_setup_request)
             + "},{\"role\":\"CU-UP\",\"assoc_id\":" + std::to_string(cu_up->assoc_id)
             + ",\"e2_setup_request\":" + e2_setup_request_as_json(&cu_up->e2_setup_request)
             + "},{\"role\":\"DU\",\"assoc_id\":" + std::to_string(du->assoc_id)
             + ",\"e2_setup_request\":" + e2_setup_request_as_json(&du->e2_setup_request)
             + "}]";
      }
      break;
  }
  return "[]";
}

std::string stats_manager::ran_as_json(const ran_t* ran)
{
  const uint64_t key = ran->generated_ran_key;
  char* ran_name = generate_ran_name(key);
  const std::string rat = ran->rat == RAT_TYPE_LTE
                        ? "LTE"
                        : (ran->rat == RAT_TYPE_NR
                           ? "NR"
                           : "");
  const std::string entities = entities_as_json(&ran->entities);
  const std::string node_info = "{\"id\":" + std::to_string(key)
                              + ",\"name\":\"" + ran_name + "\""
                              + ",\"rat\":\"" + rat + "\""
                              + ",\"entities\":" + entities + "}";
  free(ran_name);
  return node_info;
}

std::string stats_manager::get_ran_as_json() const
{
  const size_t n = ran_mgmt_get_ran_num(&_ric.ran_mgmt);
  ran_t* rans[n];
  if (n > 0) {
    const size_t m = ran_mgmt_get_rans(&_ric.ran_mgmt, rans, n);
    assert(n == m);
  }
  std::string json = "{";
  auto now = std::chrono::system_clock::now();
  json += "\"date_time\":\"" + format_date_time(now) + "\",";
  json += "\"nodes\":[";
  bool prev = false;
  for (size_t i = 0; i < n; ++i) {
    if (!ran_mgmt_is_ran_formed(rans[i]))
      continue;
    const std::string node_info = ran_as_json(rans[i]);
    const auto node = _rib.get_node(rans[i]->generated_ran_key);
    if (prev)
      json += ",";
    json += "{\"node_info\":" + node_info + "," + node->to_json_inner() + "}";
    prev = true;
  }
  return json + "]}";
}

}
}
}
