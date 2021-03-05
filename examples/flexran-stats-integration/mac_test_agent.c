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

#include "flexric_agent.h"
#include "mac_stats_rf.h"

static
void mac_gen_hdr(flatcc_builder_t* B)
{
  static uint8_t slot = 0;
  slot = (slot + 1) % 20;
  static uint16_t frame = 0;
  frame = slot == 0 ? frame + 1 : frame;
  mac_stats_IndicationHeader_create_as_root(B, frame, slot);
}

static
void mac_gen_msg(flatcc_builder_t* B, const mac_stats_report_style_t* style)
{
  assert(style->type == MAC_STATS_REPORT_STYLE_TYPE_MINIMAL);
  static uint32_t tmp = 1;
  size_t max_stats = 2;
  mac_stats_IndicationMessage_start(B);
  mac_stats_IndicationMessage_ueStats_start(B);
  for (size_t i = 0; i < max_stats; ++i) {
    mac_stats_UEStats_vec_push_start(B);
    mac_stats_UEStats_dlAggrPrb_add(B, tmp++);
    mac_stats_UEStats_ulAggrPrb_add(B, tmp++);
    mac_stats_UEStats_dlAggrTbs_add(B, tmp++);
    mac_stats_UEStats_ulAggrTbs_add(B, tmp++);
    mac_stats_UEStats_dlAggrSdus_add(B, tmp++);
    mac_stats_UEStats_ulAggrSdus_add(B, tmp++);
    mac_stats_UEStats_dlAggrBytesSdus_add(B, tmp++);
    mac_stats_UEStats_ulAggrBytesSdus_add(B, tmp++);
    mac_stats_UEStats_dlAggrRetxPrb_add(B, tmp++);
    mac_stats_UEStats_ulAggrRetxPrb_add(B, tmp++);
    mac_stats_UEStats_rnti_add(B, 0x1234 * (i + 1));
    mac_stats_UEStats_vec_push_end(B);
  }
  mac_stats_IndicationMessage_ueStats_end(B);
  mac_stats_IndicationMessage_end_as_root(B);
}

int main()
{
  const plmn_t plmn = {.mcc = 20, .mnc = 30, .mnc_digit_len = 2};
  const global_e2_node_id_t ge2ni = {.type = ngran_gNB, .plmn = plmn, .nb_id = 6666};
  e2ap_agent_t* ag = e2ap_init_agent("127.0.0.1", 36421, ge2ni);

  const mac_stats_report_style_t mac_style = { .type = MAC_STATS_REPORT_STYLE_TYPE_MINIMAL };
  mac_stats_callbacks_t mac_cb = {
    .hdr = mac_gen_hdr,
    .msg = mac_gen_msg
  };
  sm_mac_stats_register_ran_function(ag, mac_cb, &mac_style, 1);

  e2ap_start_agent(ag);

  e2ap_free_agent(ag);
  return 0;
}
