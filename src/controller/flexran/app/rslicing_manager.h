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

#ifndef RSLICING_MANAGER_H_
#define RSLICING_MANAGER_H_

#include <set>
#include <cstdint>

#include "component.h"
#include "rslicing_reader.h"
#include "rslicing_builder.h"
#include "rslicing_json_parser.h"
#include "rslicing_json_printer.h"

namespace flexric {
namespace flexran {
namespace app {

typedef const char *(*fb_parse_func)(flatcc_json_parser_t *, const char *, const char *, flatcc_builder_ref_t *);
typedef int (*fb_print_func)(flatcc_json_printer_t *ctx, const void *, size_t, const char *);

class rslicing_manager : public component {
public:
  rslicing_manager(e2ap_ric_t& ric, e2sm_rrc_event_app_t& ue_events, const rib::Rib& rib, uint16_t ric_req);

  void apply_slice_config_policy(const std::string& bs, const std::string& policy);
  void remove_slice(const std::string& bs, const std::string& policy);
  void change_ue_slice_association(const std::string& bs, const std::string& policy);

private:
  void node_connection(const e2node_event_t* event);

  static bool ran_function_has_oid(const ran_function_t* rf, const char* oid);

  void handle_rslicing(uint64_t ran_key, int assoc_id, const ran_function_t* rf);

  static void print_sub_success(const ric_subscription_response_t* r, void* object);
  static void print_sub_failure(const ric_subscription_failure_t* r, void* object);

  void print_ctrl_acknowledge(const ric_control_acknowledge_t* ca);
  void print_ctrl_failure(const ric_control_failure_t* cf);

  std::vector<mac_rslicing_ReportOccasion_enum_t> _supported_algos;

  static flatcc_builder_ref_t fb_parse_json(flatcc_builder_t* B, fb_parse_func func, const char* txt);
  static std::string fb_print_json(fb_print_func func, uint8_t* buf, size_t len);

  ran_t* get_ran_from_string(const std::string& bs);
  std::optional<uint16_t> get_rf_id_with_rslicing(const entity_info_t* e);
};

}
}
}

#endif /* RSLICING_MANAGER_H_ */
