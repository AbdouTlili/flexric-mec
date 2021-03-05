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

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "flexric_agent.h"
#include "rrc_stats_rf.h"
#include "rrc_conf_rf.h"

e2ap_agent_t* ag;

void fill_rrc_stats_msg(struct flatcc_builder* B, const rrc_stats_report_style_t* style, void* ctxt)
{
  assert(B);
  assert(!ctxt);
  assert(style->type == RRC_STATS_REPORT_STYLE_TYPE_MINIMAL);

  rrc_stats_IndicationMessage_start(B);
  rrc_stats_IndicationMessage_measReport_start(B);
  // only a single measReport. The point of the list is to accumulate them in
  // the controller
  // Note: add in order of rrc_stats_MeasReport_create()

    rrc_stats_MeasReport_vec_push_start(B);
    rrc_stats_MeasReport_measId_force_add(B, 3);
    rrc_stats_MeasReport_pCellResult_create(B, rand() % 100, rand() % 100, 0);
    rrc_stats_MeasReport_neighMeas_start(B);
    /*
    for (size_t j = 0; j < 1; ++j) {
      rrc_stats_NeighMeas_vec_push_start(B);
      // Note: add in order of rrc_stats_NeighMeas_create()
      rrc_common_Plmn_ref_t plmn = create_short_plmn(B, 208, 95, 2);
      rrc_stats_NeighMeas_plmnInfo_create(B, plmn, 0, 0, 507);
      rrc_stats_NeighMeas_measResult_create(B, 16, 17, 0);
      rrc_stats_NeighMeas_phyCellId_add(B, 200);
      rrc_stats_NeighMeas_rat_force_add(B, rrc_common_RAT_LTE);
      rrc_stats_NeighMeas_vec_push_end(B);
    }
    */
    rrc_stats_MeasReport_neighMeas_end(B);
    rrc_stats_MeasReport_vec_push_end(B);

  rrc_stats_IndicationMessage_measReport_end(B);
  rrc_stats_IndicationMessage_end_as_root(B);
}

void* periodic_rrc_stats_sender(void* arg)
{
  (void) arg;
  int i = 0;
  while (true) {
    const uint16_t rnti = 0x1234 * (i + 1);
    sm_rrc_stats_trigger_message(ag, rnti, fill_rrc_stats_msg, NULL);
    i = !i;
    printf("triggered RRC stats for RNTI %04x\n", rnti);
    sleep(1);
  }
  return NULL;
}

rrc_common_Plmn_ref_t create_short_plmn(flatcc_builder_t* B, uint16_t mcc, uint16_t mnc, uint8_t mnc_length)
{
  // Note: in order of rrc_common_Plmn_create()
  rrc_common_Plmn_start(B);
  rrc_common_Plmn_mcc_force_add(B, mcc);
  rrc_common_Plmn_mnc_force_add(B, mnc);
  rrc_common_Plmn_mnc_length_force_add(B, mnc_length);
  return rrc_common_Plmn_end(B);
}

void fill_rrc_conf_msg(flatcc_builder_t* B, const rrc_conf_report_style_t* style)
{
  assert(B);
  assert(style->type == RRC_CONF_REPORT_STYLE_TYPE_MINIMAL || style->type == RRC_CONF_REPORT_STYLE_TYPE_COMPLETE);
  rrc_conf_IndicationMessage_start(B);
  rrc_conf_IndicationMessage_bsStats_start(B);
  // Note: add in order of rrc_conf_BSStats_create()
    rrc_conf_BSStats_baseStationName_create_str(B, "OAI NSA");
    rrc_conf_BSStats_plmnInfo_start(B);
    for (size_t i = 0; i < 1; ++i) {
      rrc_common_Plmn_ref_t plmn = create_short_plmn(B, 208, 95, 2);
      rrc_conf_BSStats_plmnInfo_push_create(B, plmn, 0, 0, 20000);
    }
    rrc_conf_BSStats_plmnInfo_end(B);
    rrc_common_Plmn_ref_t pPlmn = create_short_plmn(B, 208, 95, 2);
    rrc_conf_BSStats_targetPCell_create(B, pPlmn, 0, 0, 10000);
    rrc_conf_BSStats_carriers_start(B);
    for (size_t i = 0; i < 1; ++i) {
      // Note: add in order of rrc_conf_CarrierInfo_create()
      rrc_conf_CarrierInfo_vec_push_start(B);
      rrc_conf_CarrierInfo_dlFreqHz_add(B, 3654);
      rrc_conf_CarrierInfo_ulFreqHz_add(B, 3654);
      if (style->type == RRC_CONF_REPORT_STYLE_TYPE_COMPLETE) {
        //rrc_conf_CarrierInfo_lte_add();
        rrc_conf_CarrierInfo_nr_add(B, 1);
      }
      rrc_conf_CarrierInfo_phyCellId_force_add(B, 782);
      rrc_conf_CarrierInfo_band_add(B, 78);
      rrc_conf_CarrierInfo_dlBandwidthPrb_add(B, 106);
      rrc_conf_CarrierInfo_ulBandwidthPrb_add(B, 106);
      rrc_conf_CarrierInfo_txAntennaPorts_add(B, 1);
      rrc_conf_CarrierInfo_rxAnennaPorts_add(B, 1);
      rrc_conf_CarrierInfo_vec_push_end(B);
    }
    rrc_conf_BSStats_carriers_end(B);
    rrc_conf_BSStats_rat_force_add(B, rrc_common_RAT_NR);
  rrc_conf_IndicationMessage_bsStats_end(B);
  rrc_conf_IndicationMessage_end_as_root(B);
}

int main()
{
  const plmn_t plmn = {.mcc = 30, .mnc = 45, .mnc_digit_len = 2};
  const global_e2_node_id_t ge2ni = {.type = ngran_gNB, .plmn = plmn, .nb_id = 7777};
  ag = e2ap_init_agent("127.0.0.1", 36421, ge2ni);

  const rrc_stats_report_style_t rrc_stats_style = { .type = RRC_STATS_REPORT_STYLE_TYPE_MINIMAL };
  sm_rrc_stats_register_ran_function(ag, &rrc_stats_style, 1);
  pthread_t rrc_stats;
  pthread_create(&rrc_stats, NULL, periodic_rrc_stats_sender, NULL);

  const rrc_conf_report_style_t rrc_conf_style = { .type = RRC_CONF_REPORT_STYLE_TYPE_MINIMAL };
  sm_rrc_conf_register_ran_function(ag, fill_rrc_conf_msg, &rrc_conf_style, 1);

  e2ap_start_agent(ag);

  e2ap_free_agent(ag);
  return 0;
}
