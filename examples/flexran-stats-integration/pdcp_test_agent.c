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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "flexric_agent.h"
#include "pdcp_stats_rf.h"

void pdcp_gen_hdr(flatcc_builder_t* B)
{
  const uint16_t frame = 1;
  const uint8_t slot = 80;
  pdcp_stats_IndicationHeader_create_as_root(B, frame, slot);
}

void pdcp_gen_msg(flatcc_builder_t* B, const pdcp_stats_report_style_t* style)
{
  assert(style->type == PDCP_STATS_REPORT_STYLE_TYPE_MINIMAL);
  pdcp_stats_IndicationMessage_start(B);
  pdcp_stats_IndicationMessage_ueStats_start(B);
  const size_t num_stats = 2;
  static uint32_t tmp = 1;
  for (size_t i = 0; i < num_stats; ++i) {
    // add in the order of pdcp_UEStats_create
    pdcp_stats_UEStats_vec_push_start(B);
    const size_t num_rbs = 1;
    pdcp_stats_UEStats_rb_start(B);
    for (size_t j = 0; j < num_rbs; ++j) {
      // add in the order of pdcp_RBStats_create
      pdcp_stats_RBStats_vec_push_start(B);
      pdcp_stats_RBStats_txPduPkts_add(B, tmp++);
      pdcp_stats_RBStats_rxPduPkts_add(B, tmp++);
      pdcp_stats_RBStats_txPduBytes_add(B, tmp++);
      pdcp_stats_RBStats_rxPduBytes_add(B, tmp++);
      pdcp_stats_RBStats_txPduSn_add(B, tmp++);
      pdcp_stats_RBStats_rxPduSn_add(B, tmp++);
      pdcp_stats_RBStats_rbid_add(B, 3);
      pdcp_stats_RBStats_vec_push_end(B);
    }
    pdcp_stats_UEStats_rb_end(B);
    pdcp_stats_UEStats_rnti_add(B, 0x1234 * (i + 1));
    pdcp_stats_UEStats_vec_push_end(B);
  }
  pdcp_stats_IndicationMessage_ueStats_end(B);
  pdcp_stats_IndicationMessage_end_as_root(B);

}

int main()
{
  const plmn_t plmn = {.mcc = 10, .mnc = 15, .mnc_digit_len = 2};
  const global_e2_node_id_t ge2ni = {.type = ngran_gNB, .plmn = plmn, .nb_id = 5555};
  e2ap_agent_t* ag = e2ap_init_agent("127.0.0.1", 36421, ge2ni);

  const pdcp_stats_report_style_t pdcp_style = { .type = PDCP_STATS_REPORT_STYLE_TYPE_MINIMAL };
  pdcp_stats_callbacks_t pdcp_cb = {
    .hdr = pdcp_gen_hdr,
    .msg = pdcp_gen_msg
  };
  sm_pdcp_stats_register_ran_function(ag, pdcp_cb, &pdcp_style, 1);

  e2ap_start_agent(ag);

  e2ap_free_agent(ag);
  return 0;
}
