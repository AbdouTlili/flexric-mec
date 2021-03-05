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

#include "mac_stats_manager.h"
#include "mac_stats_reader.h"
#include "mac_stats_builder.h"
//#include "mac_stats_verifier.h"
extern "C" {
#include "e2ap_multi_control.h"
}

namespace flexric {
namespace recursive_slicing {

mac_stats_manager::mac_stats_manager(e2ap_ric_t& ric, int assoc_id, e2ap_agent_t* ag, ran_function_t* mac_rf)
  : _ric(ric), _assoc_id(assoc_id), _ag(ag)
{
  service_model_cb_t cbs = {
    .handle_subscription_request = +[](e2ap_agent_t* ag, subscription_t* s, void *object) {
        return static_cast<mac_stats_manager*>(object)->handle_sub_req(ag, s);
    },
    .handle_subscription_delete_request = +[](e2ap_agent_t* ag, subscription_t* s, void *object) {
      (void) ag;
      (void) s;
      (void) object;
      assert(0);
      return false;
    },
    .handle_control_request = +[](e2ap_agent_t* ag, ep_id_t ep_id, const ric_control_request_t* cr, void *object) {
      (void) ag;
      (void) ep_id;
      (void) cr;
      (void) object;
      assert(0);
    }
  };
  e2ap_register_ran_fun(_ag, mac_rf, cbs, this);
}

bool mac_stats_manager::handle_sub_req(e2ap_agent_t* ag, subscription_t* sub)
{
  /* We will likely get multiple requests from the above controllers. To do
   * simple, we "merge" both by only forwarding the first request (and
   * asserting on successful outcome), and every other request will simply be
   * checked for equality with the first */
  assert(ag == _ag);

  const ric_subscription_request_t* req = sub->req;
  assert(req->len_action == 1);

  if (_subs.size() > 0) {
    subscription_t* init_sub = _subs.at(0);
    assert(init_sub->ep_id != sub->ep_id);
    assert(eq_ric_subscritption_request(init_sub->req, sub->req));
    _subs.push_back(sub);
    ric_action_admitted_t admitted = { .ric_act_id = req->action[0].id };
    ric_subscription_response_t resp = {
      .ric_id = req->ric_id,
      .admitted = &admitted,
      .len_admitted = 1,
      .not_admitted = NULL,
      .len_na = 0
    };
    e2ap_send_subscription_response(_ag, sub->ep_id, &resp);
    return true;
  }

  _subs.push_back(sub);
  cb_sub_t cb = {
    .sub_resp = +[](const ric_subscription_response_t* r, void *object) {
      static_cast<mac_stats_manager*>(object)->handle_sub_resp(r);
    },
    .sub_fail = +[](const ric_subscription_failure_t *f, void *object) {
      (void) f;
      (void) object;
      assert(0);
    },
    .ind = +[](const ric_indication_t* i, void *object) {
      static_cast<mac_stats_manager*>(object)->handle_ind(i);
    }
  };
  e2ap_subscribe(&_ric, _assoc_id, sub->req, cb, this);
  return true;
}

void mac_stats_manager::handle_sub_resp(const ric_subscription_response_t* r)
{
  /* since we only send the subscription for the first request, the response
   * must belong to the first request */
  assert(_subs.size() > 0);
  e2ap_send_subscription_response(_ag, _subs.at(0)->ep_id, r);
}

void mac_stats_manager::handle_ind(const ric_indication_t* i)
{
   for (subscription_t* sub : _subs) {
      ric_indication_t ind = {
        .ric_id = i->ric_id,
        .action_id = i->action_id,
        .sn = i->sn,
        .type = i->type,
        .hdr = i->hdr,
        .msg = slice_indication_msg(sub, i->msg),
        .call_process_id = i->call_process_id
      };
      e2ap_send_indication_agent(_ag, sub->ep_id, &ind);
      free_byte_array(ind.msg);
   }
}

byte_array_t mac_stats_manager::slice_indication_msg(subscription_t* sub, byte_array_t msg)
{
  flatcc_builder_t builder;
  flatcc_builder_t* B = &builder;
  flatcc_builder_init(B);

  mac_stats_IndicationMessage_start(B);
  mac_stats_IndicationMessage_ueStats_start(B);

  mac_stats_IndicationMessage_table_t im = mac_stats_IndicationMessage_as_root(msg.buf);
  mac_stats_UEStats_vec_t uestats_vec = mac_stats_IndicationMessage_ueStats(im);
  const size_t uestats_vec_len = mac_stats_UEStats_vec_len(uestats_vec);
  for (size_t i = 0; i < uestats_vec_len; ++i) {
    mac_stats_UEStats_table_t ue_stats = mac_stats_UEStats_vec_at(uestats_vec, i);
    uint16_t rnti = mac_stats_UEStats_rnti(ue_stats);

    if (endpoint_has_rnti(_ag, sub->ep_id, rnti)) {
      mac_stats_UEStats_ref_t clone = mac_stats_UEStats_clone(B, ue_stats);
      mac_stats_IndicationMessage_ueStats_push(B, clone);
    }
  }

  mac_stats_IndicationMessage_ueStats_end(B);
  mac_stats_IndicationMessage_end_as_root(B);

  size_t size;
  uint8_t *buf = (uint8_t*) flatcc_builder_finalize_buffer(B, &size);
  byte_array_t ba = { .len = size, .buf = buf };

  /*
  int ret;
  if ((ret = mac_stats_IndicationMessage_verify_as_root(ba.buf, ba.len))) {
    std::cout << "IndicationMessage is invalid: " << flatcc_verify_error_string(ret) << "\n";
    abort();
  }
  */

  flatcc_builder_clear(B);
  return ba;
}

}
}
