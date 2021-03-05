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

#include "rslicing_manager.h"
#include "rslicing_verifier.h"
extern "C" {
#include "rslicing_msg_enc.h"
#include "rslicing_msg_dec.h"
}

namespace flexric {
namespace flexran {
namespace app {

rslicing_manager::rslicing_manager(e2ap_ric_t& ric, e2sm_rrc_event_app_t& ue_events, const rib::Rib& rib, uint16_t ric_req)
  : component(ric, ue_events, rib, ric_req)
{
  ed_t ev1 = subscribe_e2node_connect(&ric.events,
      [](const e2node_event_t* event, void* object) {
        static_cast<rslicing_manager*>(object)->node_connection(event);
      }, this);
  (void) ev1;
}

void rslicing_manager::apply_slice_config_policy(
    const std::string& bs_, const std::string& policy)
{
  ran_t* ran = get_ran_from_string(bs_);
  if (!ran)
    throw std::invalid_argument("node " + bs_ + " does not exist");

  assert(ran->entities.split == SPLIT_NONE);
  const entity_info_t* e = ran->entities.nosplit.full;
  const int assoc_id = e->assoc_id;
  const auto rf_id = get_rf_id_with_rslicing(e);
  if (!rf_id.has_value())
    throw std::invalid_argument("node " + bs_ + " has no rslicing SM");

  flatcc_builder_t builder;
  flatcc_builder_t* B = &builder;
  flatcc_builder_init(B);

  flatcc_builder_ref_t root = fb_parse_json(B, mac_rslicing_SliceConfig_parse_json_table, policy.c_str());
  mac_rslicing_AddModSliceCommand_ref_t amsc = mac_rslicing_AddModSliceCommand_create(B, root);
  mac_rslicing_ControlCommand_union_ref_t u = mac_rslicing_ControlCommand_as_addModSlice(amsc);
  mac_rslicing_ControlMessage_create_as_root(B, u);

  size_t size;
  uint8_t* buf = (uint8_t*) flatcc_builder_finalize_buffer(B, &size);
  flatcc_builder_clear(B);

  int ret;
  if ((ret = mac_rslicing_ControlMessage_verify_as_root(buf, size))) {
    printf("ControlMessage is invalid: %s\n", flatcc_verify_error_string(ret));
    abort();
  }

  /* TODO: do checks */
  byte_array_t hdr = mac_rslicing_encode_empty_control_header();
  byte_array_t msg = { .len = size, .buf = buf };
  ric_gen_id_t ric_id = {
    .ric_req_id = _ric_req_id,
    .ric_inst_id = _ric_inst_id,
    .ran_func_id = *rf_id
  };
  ric_control_request_t cr = {
    .ric_id = ric_id,
    .call_process_id = NULL,
    .hdr = hdr,
    .msg = msg,
    .ack_req = NULL
  };
  cb_ctrl_t ctrl_cb = {
    .ctrl_ack = [](const ric_control_acknowledge_t* ca, void* object) {
      static_cast<rslicing_manager*>(object)->print_ctrl_acknowledge(ca);
    },
    .ctrl_fail = [](const ric_control_failure_t* cf, void* object) {
      static_cast<rslicing_manager*>(object)->print_ctrl_failure(cf);
    }
  };
  e2ap_control_req(&_ric, assoc_id, &cr, ctrl_cb, this);

  const std::string json = fb_print_json(mac_rslicing_ControlMessage_print_json_as_root, msg.buf, msg.len);
  std::cout << "sent AddModSliceCommand to assoc_id " << assoc_id
            << ":\n" << json;

  free_byte_array(hdr);
  free_byte_array(msg);
}

void rslicing_manager::remove_slice(
    const std::string& bs_, const std::string& policy)
{
  ran_t* ran = get_ran_from_string(bs_);
  if (!ran)
    throw std::invalid_argument("node " + bs_ + " does not exist");

  assert(ran->entities.split == SPLIT_NONE);
  const entity_info_t* e = ran->entities.nosplit.full;
  const int assoc_id = e->assoc_id;
  const auto rf_id = get_rf_id_with_rslicing(e);
  if (!rf_id.has_value())
    throw std::invalid_argument("node " + bs_ + " has no rslicing SM");

  flatcc_builder_t builder;
  flatcc_builder_t* B = &builder;
  flatcc_builder_init(B);

  flatcc_builder_ref_t root = fb_parse_json(B, mac_rslicing_DelSliceConfig_parse_json_table, policy.c_str());
  mac_rslicing_DelSliceCommand_ref_t dsc = mac_rslicing_DelSliceCommand_create(B, root);
  mac_rslicing_ControlCommand_union_ref_t u = mac_rslicing_ControlCommand_as_delSlice(dsc);
  mac_rslicing_ControlMessage_create_as_root(B, u);

  size_t size;
  uint8_t* buf = (uint8_t*) flatcc_builder_finalize_buffer(B, &size);
  flatcc_builder_clear(B);

  int ret;
  if ((ret = mac_rslicing_ControlMessage_verify_as_root(buf, size))) {
    printf("ControlMessage is invalid: %s\n", flatcc_verify_error_string(ret));
    abort();
  }

  /* TODO: do checks */
  byte_array_t hdr = mac_rslicing_encode_empty_control_header();
  byte_array_t msg = { .len = size, .buf = buf };
  ric_gen_id_t ric_id = {
    .ric_req_id = _ric_req_id,
    .ric_inst_id = _ric_inst_id,
    .ran_func_id = *rf_id
  };
  ric_control_request_t cr = {
    .ric_id = ric_id,
    .call_process_id = NULL,
    .hdr = hdr,
    .msg = msg,
    .ack_req = NULL
  };
  cb_ctrl_t ctrl_cb = {
    .ctrl_ack = [](const ric_control_acknowledge_t* ca, void* object) {
      static_cast<rslicing_manager*>(object)->print_ctrl_acknowledge(ca);
    },
    .ctrl_fail = [](const ric_control_failure_t* cf, void* object) {
      static_cast<rslicing_manager*>(object)->print_ctrl_failure(cf);
    }
  };
  e2ap_control_req(&_ric, assoc_id, &cr, ctrl_cb, this);

  const std::string json = fb_print_json(mac_rslicing_ControlMessage_print_json_as_root, msg.buf, msg.len);
  std::cout << "sent DelSliceCommand to assoc_id " << assoc_id
            << ":\n" << json;

  free_byte_array(hdr);
  free_byte_array(msg);
}

void rslicing_manager::change_ue_slice_association(
    const std::string& bs_, const std::string& policy)
{
  ran_t* ran = get_ran_from_string(bs_);
  if (!ran)
    throw std::invalid_argument("node " + bs_ + " does not exist");

  assert(ran->entities.split == SPLIT_NONE);
  const entity_info_t* e = ran->entities.nosplit.full;
  const int assoc_id = e->assoc_id;
  const auto rf_id = get_rf_id_with_rslicing(e);
  if (!rf_id.has_value())
    throw std::invalid_argument("node " + bs_ + " has no rslicing SM");

  flatcc_builder_t builder;
  flatcc_builder_t* B = &builder;
  flatcc_builder_init(B);

  flatcc_builder_ref_t root = fb_parse_json(B, mac_rslicing_UeSliceConfig_parse_json_table, policy.c_str());
  mac_rslicing_UeSliceAssocCommand_ref_t dsc = mac_rslicing_UeSliceAssocCommand_create(B, root);
  mac_rslicing_ControlCommand_union_ref_t u = mac_rslicing_ControlCommand_as_ueSliceAssoc(dsc);
  mac_rslicing_ControlMessage_create_as_root(B, u);

  size_t size;
  uint8_t* buf = (uint8_t*) flatcc_builder_finalize_buffer(B, &size);
  flatcc_builder_clear(B);

  int ret;
  if ((ret = mac_rslicing_ControlMessage_verify_as_root(buf, size))) {
    printf("ControlMessage is invalid: %s\n", flatcc_verify_error_string(ret));
    abort();
  }

  /* TODO: do checks */
  byte_array_t hdr = mac_rslicing_encode_empty_control_header();
  byte_array_t msg = { .len = size, .buf = buf };
  ric_gen_id_t ric_id = {
    .ric_req_id = _ric_req_id,
    .ric_inst_id = _ric_inst_id,
    .ran_func_id = *rf_id
  };
  ric_control_request_t cr = {
    .ric_id = ric_id,
    .call_process_id = NULL,
    .hdr = hdr,
    .msg = msg,
    .ack_req = NULL
  };
  cb_ctrl_t ctrl_cb = {
    .ctrl_ack = [](const ric_control_acknowledge_t* ca, void* object) {
      static_cast<rslicing_manager*>(object)->print_ctrl_acknowledge(ca);
    },
    .ctrl_fail = [](const ric_control_failure_t* cf, void* object) {
      static_cast<rslicing_manager*>(object)->print_ctrl_failure(cf);
    }
  };
  e2ap_control_req(&_ric, assoc_id, &cr, ctrl_cb, this);

  const std::string json = fb_print_json(mac_rslicing_ControlMessage_print_json_as_root, msg.buf, msg.len);
  std::cout << "sent UeSliceAssocCommand to assoc_id " << assoc_id
            << ":\n" << json;

  free_byte_array(hdr);
  free_byte_array(msg);
}

void rslicing_manager::node_connection(const e2node_event_t* event)
{
  const entity_info_t* e = ran_mgmt_get_entity(event->ran, event->assoc_id);

  for (size_t i = 0; i < e->e2_setup_request.len_rf; ++i) {
    const ran_function_t* rf = &e->e2_setup_request.ran_func_item[i];
    if (ran_function_has_oid(rf, rslicing_oid))
      handle_rslicing(event->ran->generated_ran_key, event->assoc_id, rf);
  }
}

bool rslicing_manager::ran_function_has_oid(const ran_function_t* rf, const char* oid)
{
  if (!rf->oid)
    return false;
  const std::string rf_oid{(char*)rf->oid->buf, rf->oid->len};
  return rf_oid == oid;
}

void rslicing_manager::handle_rslicing(uint64_t ran_key, int assoc_id, const ran_function_t* rf)
{
  const mac_rslicing_ReportOccasion_enum_t occ = mac_rslicing_ReportOccasion_onChange;
  mac_rslicing_SliceAlgorithm_enum_t* supported_styles;
  const size_t n = mac_rslicing_decode_ran_function(rf->def, &supported_styles);

  if (n <= 0) {
    std::cout << "no algorithms supported by rslicing SM\n";
    return;
  }

  for (size_t i = 0; i < n; ++i)
    _supported_algos.push_back(supported_styles[i]);
  free(supported_styles);

  std::cout << "request slicing configuration for RF " << rf->id << " on assoc_id " << assoc_id << "\n";

  byte_array_t event = mac_rslicing_encode_event_trigger(occ);
  ric_action_t action = {
    .id = 0,
    .type = RIC_ACT_REPORT,
    .definition = NULL,
    .subseq_action = NULL
  };
  ric_gen_id_t ric_id = {
    .ric_req_id = _ric_req_id,
    .ric_inst_id = _ric_inst_id,
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
      static_cast<rib::node_rib_info*>(object)->update_mac_rslicing(i);
    }
  };
  e2ap_subscribe(&_ric, assoc_id, &sr, cb, nri.get());
  free_byte_array(event);
}

void rslicing_manager::print_sub_success(const ric_subscription_response_t* r, void* object)
{
  (void) object;
  std::cout << "subscription to RF " << r->ric_id.ran_func_id << " succeeded\n";
}

void rslicing_manager::print_sub_failure(const ric_subscription_failure_t* f, void* object)
{
  (void) object;
  std::cout << "subscription to RF " << f->ric_id.ran_func_id << " failed\n";
}

void rslicing_manager::print_ctrl_acknowledge(const ric_control_acknowledge_t* ca)
{
  std::cout << "control ack\n";
}

void rslicing_manager::print_ctrl_failure(const ric_control_failure_t* cf)
{
  std::cout << "control fail\n";
}

flatcc_builder_ref_t rslicing_manager::fb_parse_json(flatcc_builder_t* B, fb_parse_func func, const char* txt)
{
  flatcc_builder_ref_t root;

  const char* txt_end = txt + strlen(txt);
  flatcc_json_parser_t ctx;
  int flags = flatcc_json_parser_f_force_add;
  flatcc_json_parser_init(&ctx, B, txt, txt_end, flags);
  const char* end = func(&ctx, txt, txt_end, &root);

  if(end < txt_end) {
    flatcc_builder_clear(B);
    const std::string symb = end;
    const std::string err = "error: superfluous symbols at end: '" + symb + "'";
    throw std::invalid_argument(err);
  }
  if (ctx.error) {
    flatcc_builder_clear(B);
    const std::string err = "error at line " + std::to_string(ctx.line)
                          + " column " + std::to_string(ctx.pos)
                          + ": " + flatcc_json_parser_error_string(ctx.error);
    throw std::invalid_argument(err);
  }
  return root;
}

std::string rslicing_manager::fb_print_json(fb_print_func func, uint8_t* buf, size_t len)
{
  flatcc_json_printer_t printer_ctx;
  flatcc_json_printer_init_dynamic_buffer(&printer_ctx, 0);
  printer_ctx.indent = 2;
  func(&printer_ctx, buf, len, NULL);
  size_t buf_size;
  char *txt = (char*)flatcc_json_printer_get_buffer(&printer_ctx, &buf_size);
  std::string json(txt);
  flatcc_json_printer_clear(&printer_ctx);
  return json;
}

ran_t* rslicing_manager::get_ran_from_string(const std::string& bs)
{
  const size_t n = ran_mgmt_get_ran_num(&_ric.ran_mgmt);
  if (n == 0)
    return NULL;

  ran_t* rans[n];
  if (n > 0) {
    const size_t m = ran_mgmt_get_rans(&_ric.ran_mgmt, rans, n);
    assert(n == m);
  }

  if (bs.empty())
    return rans[n-1];

  uint64_t ran_key;
  try {
    ran_key = std::stoll(bs);
  } catch (const std::invalid_argument& e) {
    return NULL;
  }

  for (size_t i = 0; i < n; ++i) {
    if (rans[i]->generated_ran_key == ran_key)
      return rans[i];
  }

  return NULL;
}

std::optional<uint16_t> rslicing_manager::get_rf_id_with_rslicing(const entity_info_t* e)
{
  const e2_setup_request_t* sr = &e->e2_setup_request;
  for (size_t i = 0; i < sr->len_rf; ++i) {
    if (ran_function_has_oid(&sr->ran_func_item[i], rslicing_oid))
      return sr->ran_func_item[i].id;
  }
  return {};
}

}
}
}
