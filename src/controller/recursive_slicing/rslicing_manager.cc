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
#include <cassert>

#include "rslicing_manager.h"
#include "rslicing_builder.h"
#include "rslicing_verifier.h"
#include "rslicing_json_printer.h"
extern "C" {
#include "rslicing_msg_enc.h"
#include "rslicing_msg_dec.h"
#include "rrc_event_reader.h"
#include "e2ap_multi_control.h"
}

namespace flexric {
namespace recursive_slicing {

rslicing_manager::rslicing_manager(e2ap_ric_t& ric, e2sm_rrc_event_app_t& ue_events, int assoc_id, e2ap_agent_t* ag, ran_function_t* slicing_rf)
  : _ric(ric), _ue_events(ue_events), _assoc_id(assoc_id), _ag(ag)
{
  _ric_id = { .ric_req_id = 12, .ric_inst_id = 122, .ran_func_id = slicing_rf->id };
  service_model_cb_t cbs = {
    .handle_subscription_request = +[](e2ap_agent_t* ag, subscription_t* s, void *object) {
      return static_cast<rslicing_manager*>(object)->handle_sub_req(ag, s);
    },
    .handle_subscription_delete_request = +[](e2ap_agent_t* ag, subscription_t* s, void *object) {
      (void) ag;
      (void) s;
      (void) object;
      assert(0);
      return false;
    },
    .handle_control_request = +[](e2ap_agent_t* ag, ep_id_t ep_id, const ric_control_request_t* cr, void *object) {
      static_cast<rslicing_manager*>(object)->handle_ctrl_req(ag, ep_id, cr);
    }
  };
  e2ap_register_ran_fun(_ag, slicing_rf, cbs, this);

  /* send subscription request to be informed about "what is going on" */
  const mac_rslicing_ReportOccasion_enum_t occ = mac_rslicing_ReportOccasion_onChange;
  byte_array_t event = mac_rslicing_encode_event_trigger(occ);
  ric_action_t action = {
    .id = 0,
    .type = RIC_ACT_REPORT,
    .definition = NULL,
    .subseq_action = NULL
  };
  ric_subscription_request_t sr = {
    .ric_id = _ric_id,
    .event_trigger = event,
    .action = &action,
    .len_action = 1
  };
  cb_sub_t cb = {
    .sub_resp = [](const auto* r, void *object) {
      (void) object;
      std::cout << "subscription to RF " << r->ric_id.ran_func_id << " succeeded\n";
    },
    .sub_fail = [](const auto* f, void* object) {
      (void) object;
      std::cerr << "subscription to RF " << f->ric_id.ran_func_id << " FAILED\n";
      assert(0);
    },
    .ind = [](const auto *i, void* object) {
      static_cast<rslicing_manager*>(object)->handle_ind(i);
    }
  };
  e2ap_subscribe(&_ric, assoc_id, &sr, cb, this);

  ue_ed_t ev2 = subscribe_ue_attach(&_ue_events,
      [] (const ran_t* ran, uint16_t rnti, byte_array_t msg, void* object) {
        static_cast<rslicing_manager*>(object)->ue_attach(ran, rnti, msg);
      }, this);
  (void) ev2;
}

void rslicing_manager::add_endpoint(ep_id_t ep_id, float max_resources, float max_throughput)
{
  service_t service(std::make_shared<res_virt>(ep_id, max_resources, max_throughput), NULL);
  _services.insert({ep_id, service});
}

void rslicing_manager::init_userplane()
{
  std::vector<nvs> rs = { {0, 0.175, 17.5} };
  for (auto& s: _services) {
    ep_id_t ep_id = s.first;
    service_t& serv = s.second;
    std::vector<nvs> nrs = serv.rv->get_real_slices();
    rs.insert(rs.end(), nrs.begin(), nrs.end());
    std::cout << "init_userplane():\n";
    serv.rv->dump_real_slices();
  }
  send_addmod_command(rs);
}

void rslicing_manager::ue_attach(const ran_t* ran, uint16_t rnti, byte_array_t msg)
{
  rrc_event_IndicationMessage_table_t im = rrc_event_IndicationMessage_as_root(msg.buf);
  int selectedPlmnId = rrc_event_IndicationMessage_selectedPlmnId(im);
  assert(selectedPlmnId == 1 || selectedPlmnId == 2);

  auto it = _services.find(selectedPlmnId);
  assert(it != _services.end());
  service_t& serv = it->second;

  const std::vector<ue_assoc> vuea = { { rnti, 0 } };
  serv.rv->update_ues(vuea);

  uint32_t real = serv.rv->slice_id_real(0); // by default is in 0
  const std::vector<ue_assoc> uea = { { rnti, real } };
  send_ue_assoc_command(uea);
}


bool rslicing_manager::handle_sub_req(e2ap_agent_t* ag, subscription_t* sub)
{
  /* A rslicing subscription is issued to receive updates of the slicing state
   * after control commands. The rslicing_manager class requests this at the
   * beginning by itself. Therefore, we acknowledge all requests, and forward
   * the indication messages in the beginning, when they arrive from the agent,
   * and whene there are new UEs. */
  assert(ag == _ag);

  auto it = _services.find(sub->ep_id);
  assert(it != _services.end());
  assert(it->first == sub->ep_id);
  assert(!it->second.sub);


  ep_id_t ep_id = it->first;
  service_t& serv = it->second;

  serv.sub = sub;

  std::vector<nvs> vs = serv.rv->get_virt_slices();
  /* ToDO: assume for the moment that there are no UEs yet when we get this
   * message */
  std::vector<ue_assoc> virt_ue_assoc;
  byte_array_t msg = create_indication(vs, virt_ue_assoc);
  byte_array_t hdr = mac_rslicing_encode_empty_indication_header();
  const ric_indication_t ind = {
    .ric_id = serv.sub->req->ric_id,
    .action_id = serv.sub->req->action[0].id,
    .sn = NULL,
    .type = RIC_IND_REPORT,
    .hdr = hdr,
    .msg = msg,
    .call_process_id = NULL
  };
  e2ap_send_indication_agent(_ag, serv.sub->ep_id, &ind);
  free_byte_array(msg);
  free_byte_array(hdr);
  return true;
}

void rslicing_manager::handle_ind(const ric_indication_t* i)
{
  flatcc_json_printer_t printer_ctx;
  flatcc_json_printer_init_dynamic_buffer(&printer_ctx, 0);
  printer_ctx.indent = 2;
  mac_rslicing_IndicationMessage_print_json_as_root(&printer_ctx, i->msg.buf, i->msg.len, NULL);
  size_t buf_size;
  char *txt = (char*)flatcc_json_printer_get_buffer(&printer_ctx, &buf_size);
  std::cout << "*** " << __func__ << "():\n" << txt;
  flatcc_json_printer_clear(&printer_ctx);

  mac_rslicing_IndicationMessage_table_t im = mac_rslicing_IndicationMessage_as_root(i->msg.buf);
  mac_rslicing_SliceConfig_table_t sc = mac_rslicing_IndicationMessage_sliceConfig(im);
  mac_rslicing_UlDlSliceConfig_table_t dl = mac_rslicing_SliceConfig_dl(sc);
  std::vector<nvs> real_slices = nvs_slices_from_fb(dl);
  /* in the beginning there might be a first message without any slices, in
   * this case just wait until we have slices */
  if (real_slices.size() == 0)
    return;
  mac_rslicing_UeSliceConfig_table_t uesc = mac_rslicing_IndicationMessage_ueSliceConfig(im);
  std::vector<ue_assoc> real_ue_assoc = ue_assoc_from_fb(uesc);
  for (auto& s: _services) {
    ep_id_t ep_id = s.first;
    service_t& serv = s.second;
    std::cout << "before new slice config:\n";
    serv.rv->dump_virt_slices();
    serv.rv->dump_virt_ue_assoc();
    serv.rv->update(real_slices);
    serv.rv->verify_ues(real_ue_assoc);
    if (serv.sub) {
      std::vector<nvs> vs = serv.rv->get_virt_slices();
      std::vector<ue_assoc> virt_ue_assoc = serv.rv->get_virt_ue_assoc();
      byte_array_t msg = create_indication(vs, virt_ue_assoc);
      byte_array_t hdr = mac_rslicing_encode_empty_indication_header();
      const ric_indication_t ind = {
        .ric_id = serv.sub->req->ric_id,
        .action_id = serv.sub->req->action[0].id,
        .sn = NULL,
        .type = RIC_IND_REPORT,
        .hdr = hdr,
        .msg = msg,
        .call_process_id = NULL
      };
      e2ap_send_indication_agent(_ag, serv.sub->ep_id, &ind);
      free_byte_array(msg);
      free_byte_array(hdr);
    }
  }
}

void rslicing_manager::handle_ctrl_req(e2ap_agent_t* ag, ep_id_t ep_id, const ric_control_request_t* cr)
{
  assert(ag == _ag);
  assert(ep_id == 1 || ep_id == 2);
  assert(cr);

  auto it = _services.find(ep_id);
  assert(it != _services.end());

  mac_rslicing_ControlMessage_table_t cm = mac_rslicing_ControlMessage_as_root(cr->msg.buf);
  mac_rslicing_ControlCommand_union_t cc = mac_rslicing_ControlMessage_command_union(cm);
  switch (cc.type) {
    case mac_rslicing_ControlCommand_addModSlice:
      handle_addmod_command(it->second.rv, (mac_rslicing_AddModSliceCommand_table_t) cc.value);
      break;
    case mac_rslicing_ControlCommand_delSlice:
      handle_del_command(it->second.rv, (mac_rslicing_DelSliceCommand_table_t) cc.value);
      break;
    case mac_rslicing_ControlCommand_ueSliceAssoc:
      handle_ue_assoc_command(it->second.rv, (mac_rslicing_UeSliceAssocCommand_table_t) cc.value);
      break;
    default:
      assert(0 && "illegal command");
      break;
  }
}

void rslicing_manager::handle_addmod_command(std::shared_ptr<res_virt> rv, mac_rslicing_AddModSliceCommand_table_t amsc)
{
  mac_rslicing_SliceConfig_table_t sc = mac_rslicing_AddModSliceCommand_sliceConfig(amsc); 
  if (mac_rslicing_SliceConfig_ul_is_present(sc))
    std::cerr << "*** ignoring UL configuration\n";
  mac_rslicing_UlDlSliceConfig_table_t dl = mac_rslicing_SliceConfig_dl(sc);
  //std::cout << "before new slice config:\n";
  //rv->dump_virt_slices();
  if (mac_rslicing_UlDlSliceConfig_algorithm(dl) == mac_rslicing_SliceAlgorithm_None) {
    std::vector<uint32_t> del_real = rv->reset();
    send_del_command(del_real);
  } else {
    std::vector<nvs> slices = nvs_slices_from_fb(dl);
    if (!rv->admission_control(slices)) {
      std::cerr << "error: cannot apply new config\n";
      // TODO: send failure
      return;
    }
    rv->apply(slices);
  }
  //std::cout << "after new slice config:\n";
  //rv->dump_virt_slices();
  std::vector<nvs> new_config = rv->get_real_slices();

  send_addmod_command(new_config);
}

void rslicing_manager::handle_del_command(std::shared_ptr<res_virt> rv, mac_rslicing_DelSliceCommand_table_t t)
{
  mac_rslicing_DelSliceConfig_table_t dsc = mac_rslicing_DelSliceCommand_delSliceConfig(t);
  flatbuffers_uint32_vec_t ul_ids = mac_rslicing_DelSliceConfig_ul(dsc);
  size_t n_ul_ids = flatbuffers_uint32_vec_len(ul_ids);
  if (n_ul_ids > 0)
    std::cerr << "ignoring UL configs\n";

  flatbuffers_uint32_vec_t dl_ids = mac_rslicing_DelSliceConfig_dl(dsc);
  size_t n_dl_ids = flatbuffers_uint32_vec_len(dl_ids);
  std::vector<uint32_t> del;
  for (size_t i = 0; i < n_dl_ids; ++i)
    del.push_back(dl_ids[i]);
  if (!rv->check_slices_exist(del)) {
    std::cerr << "error: cannot delete slices: some slices don't exist\n";
    // TODO: send failure
    return;
  }
  std::vector<ue_assoc> reassoc = rv->apply_delete(del);
  if (reassoc.size() > 0) {
    send_ue_assoc_command(reassoc);
    // TODO: hack: we should have a queue to only send one command at a time
    usleep(10000);
  }

  // TODO this should go into res_virt
  std::vector<uint32_t> del_real;
  for (uint32_t dl : del )
    del_real.push_back(rv->slice_id_real(dl));

  send_del_command(del_real);
}

void rslicing_manager::handle_ue_assoc_command(std::shared_ptr<res_virt> rv, mac_rslicing_UeSliceAssocCommand_table_t uesac)
{
  mac_rslicing_UeSliceConfig_table_t usc = mac_rslicing_UeSliceAssocCommand_ueSliceConfig(uesac);
  mac_rslicing_UeSliceAssoc_vec_t ues = mac_rslicing_UeSliceConfig_ues(usc);
  size_t n_new_ues = mac_rslicing_UeSliceAssoc_vec_len(ues);
  std::vector<ue_assoc> ue_assocs;
  for (size_t i = 0; i < n_new_ues; ++i) {
    mac_rslicing_UeSliceAssoc_table_t uesa = mac_rslicing_UeSliceAssoc_vec_at(ues, i);
    uint16_t rnti = mac_rslicing_UeSliceAssoc_rnti(uesa);
    if (mac_rslicing_UeSliceAssoc_ulId_is_present(uesa))
      std::cerr << "ignoring Ul uesliceassoc for RNTI " << rnti << "\n";
    if (!mac_rslicing_UeSliceAssoc_dlId_is_present(uesa)) {
      std::cerr << "no dl UeSliceAssoc for RNTI " << rnti << ", aborting\n";
      return;
    }
    uint32_t id = mac_rslicing_UeSliceAssoc_dlId(uesa);
    ue_assocs.emplace_back(rnti, id);
  }

  if (!rv->check_slices_exist(ue_assocs)) {
    std::cerr << "error: cannot delete slices: some slices don't exist\n";
    // TODO: send failure
    return;
  }
  rv->update_ues(ue_assocs);

  std::vector<ue_assoc> real_assocs = rv->get_real_ue_assoc();
  send_ue_assoc_command(real_assocs);
}

void rslicing_manager::send_addmod_command(const std::vector<nvs>& slices)
{
  byte_array_t msg = create_addmod_slice_command(slices);
  byte_array_t hdr = mac_rslicing_encode_empty_control_header();
  ric_control_ack_req_t ack = RIC_CONTROL_REQUEST_ACK;
  ric_control_request_t cr = {
    .ric_id = _ric_id,
    .call_process_id = NULL,
    .hdr = hdr,
    .msg = msg,
    .ack_req = &ack
  };
  cb_ctrl_t ctrl_cb = {
    .ctrl_ack = [](const ric_control_acknowledge_t* ca, void* object) {
      (void) ca;
      (void) object;
      std::cout << "addmod succeeded\n";
    },
    .ctrl_fail = [](const ric_control_failure_t* cf, void* object) {
      (void) cf;
      (void) object;
      assert(0 && "addmod control failed");
    }
  };
  e2ap_control_req(&_ric, _assoc_id, &cr, ctrl_cb, this);
  free_byte_array(msg);
  free_byte_array(hdr);
}

void rslicing_manager::send_del_command(const std::vector<uint32_t>& del)
{
  byte_array_t msg = create_remove_slice_command(del);
  byte_array_t hdr = mac_rslicing_encode_empty_control_header();
  ric_control_ack_req_t ack = RIC_CONTROL_REQUEST_ACK;
  ric_control_request_t cr = {
    .ric_id = _ric_id,
    .call_process_id = NULL,
    .hdr = hdr,
    .msg = msg,
    .ack_req = &ack
  };
  cb_ctrl_t ctrl_cb = {
    .ctrl_ack = [](const ric_control_acknowledge_t* ca, void* object) {
      (void) ca;
      (void) object;
      std::cout << "delete succeeded\n";
    },
    .ctrl_fail = [](const ric_control_failure_t* cf, void* object) {
      (void) cf;
      (void) object;
      assert(0 && "delete control failed");
    }
  };
  e2ap_control_req(&_ric, _assoc_id, &cr, ctrl_cb, this);
  free_byte_array(hdr);
  free_byte_array(msg);
}

void rslicing_manager::send_ue_assoc_command(const std::vector<ue_assoc>& assoc)
{
  byte_array_t msg = create_ue_assoc_command(assoc);
  byte_array_t hdr = mac_rslicing_encode_empty_control_header();
  ric_control_ack_req_t ack = RIC_CONTROL_REQUEST_ACK;
  ric_control_request_t cr = {
    .ric_id = _ric_id,
    .call_process_id = NULL,
    .hdr = hdr,
    .msg = msg,
    .ack_req = &ack
  };
  cb_ctrl_t ctrl_cb = {
    .ctrl_ack = [](const ric_control_acknowledge_t* ca, void* object) {
      (void) ca;
      (void) object;
      std::cout << "ue assoc succeeded\n";
    },
    .ctrl_fail = [](const ric_control_failure_t* cf, void* object) {
      (void) cf;
      (void) object;
      assert(0 && "ue assoc control failed");
    }
  };
  e2ap_control_req(&_ric, _assoc_id, &cr, ctrl_cb, this);
  free_byte_array(hdr);
  free_byte_array(msg);
}

byte_array_t rslicing_manager::create_indication(const std::vector<nvs>& slices, const std::vector<ue_assoc>& ues)
{
  flatcc_builder_t builder;
  flatcc_builder_t* B = &builder;
  flatcc_builder_init(B);

  mac_rslicing_IndicationMessage_start(B);
  mac_rslicing_IndicationMessage_sliceConfig_start(B);
  mac_rslicing_SliceConfig_start(B);
    mac_rslicing_SliceConfig_dl_start(B);
      add_sliceconfig_dl(B, slices);
    mac_rslicing_SliceConfig_dl_end(B);

    mac_rslicing_SliceConfig_ul_start(B);
      mac_rslicing_UlDlSliceConfig_algorithm_force_add(B, mac_rslicing_SliceAlgorithm_None);
      flatbuffers_string_ref_t sched = flatbuffers_string_create_str(B, "pf");
      mac_rslicing_UlDlSliceConfig_scheduler_add(B, sched);
    mac_rslicing_SliceConfig_ul_end(B);
  mac_rslicing_IndicationMessage_sliceConfig_end(B);

  mac_rslicing_IndicationMessage_ueSliceConfig_start(B);
  mac_rslicing_UeSliceConfig_ues_start(B);
  for (const auto& ue : ues) {
    mac_rslicing_UeSliceAssoc_start(B);
    mac_rslicing_UeSliceAssoc_dlId_force_add(B, ue.dl);
    mac_rslicing_UeSliceAssoc_rnti_force_add(B, ue.rnti);
    mac_rslicing_UeSliceConfig_ues_push(B, mac_rslicing_UeSliceAssoc_end(B));
  }
  mac_rslicing_UeSliceConfig_ues_end(B);
  mac_rslicing_IndicationMessage_ueSliceConfig_end(B);
  mac_rslicing_IndicationMessage_end_as_root(B);

  size_t size;
  uint8_t *buf = (uint8_t*) flatcc_builder_finalize_buffer(B, &size);

  int ret;
  if ((ret = mac_rslicing_IndicationMessage_verify_as_root(buf, size))) {
    std::cerr << "IndicationMessage is invalid: " << flatcc_verify_error_string(ret) << "\n";
    abort();
  }

  flatcc_json_printer_t printer_ctx;
  flatcc_json_printer_init_dynamic_buffer(&printer_ctx, 0);
  printer_ctx.indent = 2;
  mac_rslicing_IndicationMessage_print_json_as_root(&printer_ctx, buf, size, NULL);
  size_t buf_size;
  char *txt = (char*)flatcc_json_printer_get_buffer(&printer_ctx, &buf_size);
  std::cout << "*** " << __func__ << "():\n" << txt;
  flatcc_json_printer_clear(&printer_ctx);

  const byte_array_t msg = { .len = size, .buf = buf };
  return msg;
}

byte_array_t rslicing_manager::create_addmod_slice_command(const std::vector<nvs>& addmod_slices)
{
  assert(addmod_slices.size() > 0);

  flatcc_builder_t builder;
  flatcc_builder_t* B = &builder;
  flatcc_builder_init(B);

  mac_rslicing_SliceConfig_start(B);
    mac_rslicing_SliceConfig_dl_start(B);
      add_sliceconfig_dl(B, addmod_slices);
    mac_rslicing_SliceConfig_dl_end(B);
  mac_rslicing_SliceConfig_ref_t sc = mac_rslicing_SliceConfig_end(B);
  mac_rslicing_AddModSliceCommand_ref_t amsc = mac_rslicing_AddModSliceCommand_create(B, sc);
  mac_rslicing_ControlCommand_union_ref_t u = mac_rslicing_ControlCommand_as_addModSlice(amsc);
  mac_rslicing_ControlMessage_create_as_root(B, u);

  size_t size;
  uint8_t *buf = (uint8_t*) flatcc_builder_finalize_buffer(B, &size);

  flatcc_builder_clear(B);

  int ret;
  if ((ret = mac_rslicing_ControlMessage_verify_as_root(buf, size))) {
    std::cout << __func__ << "(): ControlMessage is invalid: "
              << flatcc_verify_error_string(ret) << "\n";
    abort();
  }

  flatcc_json_printer_t printer_ctx;
  flatcc_json_printer_init_dynamic_buffer(&printer_ctx, 0);
  printer_ctx.indent = 2;
  mac_rslicing_ControlMessage_print_json_as_root(&printer_ctx, buf, size, NULL);
  size_t buf_size;
  char *txt = (char*)flatcc_json_printer_get_buffer(&printer_ctx, &buf_size);
  std::cout << txt;
  flatcc_json_printer_clear(&printer_ctx);

  byte_array_t msg = { .len = size, .buf = buf };
  return msg;
}

byte_array_t rslicing_manager::create_remove_slice_command(const std::vector<uint32_t>& remove_slices)
{
  flatcc_builder_t builder;
  flatcc_builder_t* B = &builder;
  flatcc_builder_init(B);

  mac_rslicing_DelSliceCommand_start(B);
  mac_rslicing_DelSliceCommand_delSliceConfig_start(B);
  mac_rslicing_DelSliceConfig_dl_start(B);
  for (uint32_t i : remove_slices)
    mac_rslicing_DelSliceConfig_dl_push_create(B, i);
  mac_rslicing_DelSliceConfig_dl_end(B);
  mac_rslicing_DelSliceCommand_delSliceConfig_end(B);
  mac_rslicing_DelSliceCommand_ref_t dsc = mac_rslicing_DelSliceCommand_end(B);
  mac_rslicing_ControlCommand_union_ref_t u = mac_rslicing_ControlCommand_as_delSlice(dsc);
  mac_rslicing_ControlMessage_create_as_root(B, u);

  size_t size;
  uint8_t *buf = (uint8_t*) flatcc_builder_finalize_buffer(B, &size);

  flatcc_builder_clear(B);

  int ret;
  if ((ret = mac_rslicing_ControlMessage_verify_as_root(buf, size))) {
    printf("ControlMessage is invalid: %s\n", flatcc_verify_error_string(ret));
    abort();
  }

  flatcc_json_printer_t printer_ctx;
  flatcc_json_printer_init_dynamic_buffer(&printer_ctx, 0);
  printer_ctx.indent = 2;
  mac_rslicing_ControlMessage_print_json_as_root(&printer_ctx, buf, size, NULL);
  size_t buf_size;
  char *txt = (char*)flatcc_json_printer_get_buffer(&printer_ctx, &buf_size);
  std::cout << txt;
  flatcc_json_printer_clear(&printer_ctx);

  byte_array_t msg = { .len = size, .buf = buf };
  return msg;
}

byte_array_t rslicing_manager::create_ue_assoc_command(const std::vector<ue_assoc>& real_ue_assoc)
{
  flatcc_builder_t builder;
  flatcc_builder_t* B = &builder;
  flatcc_builder_init(B);

  mac_rslicing_UeSliceAssocCommand_start(B);
  mac_rslicing_UeSliceAssocCommand_ueSliceConfig_start(B);
  mac_rslicing_UeSliceConfig_ues_start(B);
  for (const auto& ue : real_ue_assoc) {
    mac_rslicing_UeSliceAssoc_start(B);
    mac_rslicing_UeSliceAssoc_dlId_force_add(B, ue.dl);
    mac_rslicing_UeSliceAssoc_rnti_force_add(B, ue.rnti);
    mac_rslicing_UeSliceConfig_ues_push(B, mac_rslicing_UeSliceAssoc_end(B));
  }
  mac_rslicing_UeSliceConfig_ues_end(B);
  mac_rslicing_UeSliceAssocCommand_ueSliceConfig_end(B);
  mac_rslicing_UeSliceAssocCommand_ref_t dsc = mac_rslicing_UeSliceAssocCommand_end(B);
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

  flatcc_json_printer_t printer_ctx;
  flatcc_json_printer_init_dynamic_buffer(&printer_ctx, 0);
  printer_ctx.indent = 2;
  mac_rslicing_ControlMessage_print_json_as_root(&printer_ctx, buf, size, NULL);
  size_t buf_size;
  char *txt = (char*)flatcc_json_printer_get_buffer(&printer_ctx, &buf_size);
  std::cout << txt;
  flatcc_json_printer_clear(&printer_ctx);

  byte_array_t msg = { .len = size, .buf = buf };
  return msg;
}

nvs rslicing_manager::nvs_from_fb(uint32_t id, mac_rslicing_NvsSliceConfig_union_t unvs)
{
  if (unvs.type == mac_rslicing_NvsSliceConfig_rate) {
    mac_rslicing_NvsRate_table_t rate = (mac_rslicing_NvsRate_table_t) unvs.value;
    const float req = mac_rslicing_NvsRate_mbpsRequired(rate);
    const float ref = mac_rslicing_NvsRate_mbpsReference(rate);
    return nvs(id, req, ref);
  } else {
    assert(unvs.type == mac_rslicing_NvsSliceConfig_capacity);
    mac_rslicing_NvsCapacity_table_t cap = (mac_rslicing_NvsCapacity_table_t) unvs.value;
    const float pct = mac_rslicing_NvsCapacity_pctReserved(cap);
    return nvs(id, pct);
  }
}

std::vector<nvs> rslicing_manager::nvs_slices_from_fb(mac_rslicing_UlDlSliceConfig_table_t t)
{
  std::vector<nvs> nvs_slices;
  if (mac_rslicing_UlDlSliceConfig_algorithm(t) != mac_rslicing_SliceAlgorithm_NVS) {
    std::cerr << "received UlDlSliceConfig without NVS\n";
    return nvs_slices;
  }
  mac_rslicing_Slice_vec_t slices = mac_rslicing_UlDlSliceConfig_slices(t);
  size_t slices_len = mac_rslicing_Slice_vec_len(slices);
  for (size_t i = 0; i < slices_len; ++i) {
    mac_rslicing_Slice_table_t slice = mac_rslicing_Slice_vec_at(slices, i);
    uint32_t id = mac_rslicing_Slice_id(slice);
    mac_rslicing_SliceParams_union_t p = mac_rslicing_Slice_params_union(slice);
    assert(p.type == mac_rslicing_SliceParams_nvs);
    mac_rslicing_NvsSlice_table_t nvst = (mac_rslicing_NvsSlice_table_t) p.value;
    mac_rslicing_NvsSliceConfig_union_t unvs = mac_rslicing_NvsSlice_config_union(nvst);
    nvs s = nvs_from_fb(id, unvs);
    if (mac_rslicing_Slice_label_is_present(slice))
      s.label = std::string(mac_rslicing_Slice_label(slice));
    if (mac_rslicing_Slice_scheduler_is_present(slice))
      s.scheduler = std::string(mac_rslicing_Slice_scheduler(slice));
    nvs_slices.push_back(s);
  }
  return nvs_slices;
}

std::vector<ue_assoc> rslicing_manager::ue_assoc_from_fb(mac_rslicing_UeSliceConfig_table_t t)
{
  mac_rslicing_UeSliceAssoc_vec_t uesliceassoc = mac_rslicing_UeSliceConfig_ues(t);
  std::vector<ue_assoc> ues;
  size_t uesliceassoc_len = mac_rslicing_UeSliceAssoc_vec_len(uesliceassoc);
  for (size_t i = 0; i < uesliceassoc_len; ++i) {
    mac_rslicing_UeSliceAssoc_table_t uea = mac_rslicing_UeSliceAssoc_vec_at(uesliceassoc, i);
    const uint32_t rnti = mac_rslicing_UeSliceAssoc_rnti(uea);
    if (!mac_rslicing_UeSliceAssoc_dlId_is_present(uea)) {
      std::cerr << "expected dlId for rnti " << rnti << ", skipping\n";
      continue;
    }
    const uint32_t dl = mac_rslicing_UeSliceAssoc_dlId(uea);
    ues.emplace_back(rnti, dl);
  }
  return ues;
}

void rslicing_manager::add_fb_slice(struct flatcc_builder* B, const nvs& s)
{
  mac_rslicing_NvsSliceConfig_union_ref_t unvs;
  switch (s.type) {
    case nvs_type::rate: {
        const float req = s.rate.mbpsRequired;
        const float ref = s.rate.mbpsReference;
        mac_rslicing_NvsRate_ref_t nvsrate = mac_rslicing_NvsRate_create(B, req, ref);
        unvs = mac_rslicing_NvsSliceConfig_as_rate(nvsrate);
      }
      break;
    case nvs_type::capacity: {
        const float pct = s.cap.pctReserved;
        mac_rslicing_NvsCapacity_ref_t nvscap = mac_rslicing_NvsCapacity_create(B, pct);
        unvs = mac_rslicing_NvsSliceConfig_as_capacity(nvscap);
      }
      break;
    default:
      assert(0 && "illegal slice type");
      break;
  }
  mac_rslicing_NvsSlice_ref_t nvs = mac_rslicing_NvsSlice_create(B, unvs);
  mac_rslicing_SliceParams_union_ref_t p = mac_rslicing_SliceParams_as_nvs(nvs);
  mac_rslicing_Slice_vec_push_start(B);
  mac_rslicing_Slice_id_force_add(B, s.id);
  if (!s.label.empty()) {
    flatbuffers_string_ref_t label = flatbuffers_string_create_str(B, s.label.c_str());
    mac_rslicing_Slice_label_add(B, label);
  }
  if (!s.scheduler.empty()) {
    flatbuffers_string_ref_t scheduler = flatbuffers_string_create_str(B, s.scheduler.c_str());
    mac_rslicing_Slice_scheduler_add(B, scheduler);
  }
  mac_rslicing_Slice_params_add(B, p);
  mac_rslicing_Slice_vec_push_end(B);
}

void rslicing_manager::add_sliceconfig_dl(struct flatcc_builder* B, const std::vector<nvs>& slices)
{
  if (!slices.empty()) {
    mac_rslicing_UlDlSliceConfig_algorithm_force_add(B, mac_rslicing_SliceAlgorithm_NVS);
    mac_rslicing_UlDlSliceConfig_slices_start(B);
    for (const auto& s : slices)
      add_fb_slice(B, s);
    mac_rslicing_UlDlSliceConfig_slices_end(B);
  } else {
    mac_rslicing_UlDlSliceConfig_algorithm_force_add(B, mac_rslicing_SliceAlgorithm_None);
    flatbuffers_string_ref_t sched = flatbuffers_string_create_str(B, "pf");
    mac_rslicing_UlDlSliceConfig_scheduler_add(B, sched);
  }
}

}
}
