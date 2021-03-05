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
#include "mac_stats_rf.h"
#include "rslicing_rf.h"
#include "rslicing_verifier.h"
#include "pdcp_stats_rf.h"
#include "rlc_stats_rf.h"
#include "rrc_stats_rf.h"
#include "rrc_conf_rf.h"
#include "rrc_event_rf.h"

e2ap_agent_t* g_ag;

static size_t num_ues = 2;

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
  static uint32_t tmp = 1;
  mac_stats_IndicationMessage_start(B);
  mac_stats_IndicationMessage_ueStats_start(B);
  for (size_t i = 0; i < num_ues; ++i) {
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
    if (style->type == MAC_STATS_REPORT_STYLE_TYPE_COMPLETE) {
      mac_stats_UEStats_puschSnr_add(B, tmp++ % 31);
      mac_stats_UEStats_pucchSnr_add(B, tmp++ % 31);
      const size_t numBsr = tmp++ % 3;
      if (numBsr > 0) {
        mac_stats_UEStats_bsr_start(B);
        for (uint8_t j = 0; j < numBsr; ++j) {
          const uint8_t lcgid = tmp++ % 10;
          const uint32_t bufferSize = tmp++;
          mac_stats_UEStats_bsr_push_create(B, lcgid, bufferSize);
        }
        mac_stats_UEStats_bsr_end(B);
      }
      const size_t numDlHarq = tmp++ % 7;
      if (numDlHarq > 0) {
        mac_stats_UEStats_dlHarq_start(B);
        uint32_t* v = mac_stats_UEStats_dlHarq_extend(B, numDlHarq);
        for (uint8_t j = 0; j < numDlHarq; ++j)
          v[j] = tmp++;
        mac_stats_UEStats_dlHarq_end(B);
      }
      const size_t numUlHarq = tmp++ % 7;
      if (numUlHarq > 0) {
        mac_stats_UEStats_ulHarq_start(B);
        uint32_t* v = mac_stats_UEStats_ulHarq_extend(B, numUlHarq);
        for (uint8_t j = 0; j < numUlHarq; ++j)
          v[j] = tmp++;
        mac_stats_UEStats_ulHarq_end(B);
      }
    }
    mac_stats_UEStats_rnti_add(B, 37 * (i + 1));
    if (style->type == MAC_STATS_REPORT_STYLE_TYPE_COMPLETE) {
      mac_stats_UEStats_wbCqi_add(B, tmp++ % 15);
      mac_stats_UEStats_dlMcs1_add(B, tmp++ % 28);
      mac_stats_UEStats_ulMcs1_add(B, tmp++ % 28);
      //mac_stats_UEStats_dlMcs2_add(B, 0);
      //mac_stats_UEStats_ulMcs2_add(B, 0);
      mac_stats_UEStats_phr_add(B, tmp++ % 48);
    }
    mac_stats_UEStats_vec_push_end(B);
  }
  mac_stats_IndicationMessage_ueStats_end(B);
  mac_stats_IndicationMessage_end_as_root(B);
}

static byte_array_t g_slice_config;
static byte_array_t g_ue_slice_config;
static
void rslicing_init_config()
{
  assert(g_slice_config.len == 0 && !g_slice_config.buf);
  assert(g_ue_slice_config.len == 0 && !g_ue_slice_config.buf);

  flatcc_builder_t builder;
  flatcc_builder_t* B = &builder;
  flatcc_builder_init(B);

  mac_rslicing_SliceConfig_start(B);
    mac_rslicing_SliceConfig_dl_start(B);
      mac_rslicing_UlDlSliceConfig_algorithm_force_add(B, mac_rslicing_SliceAlgorithm_None);
      flatbuffers_string_ref_t sched_dl = flatbuffers_string_create_str(B, "pf_dl");
      mac_rslicing_UlDlSliceConfig_scheduler_add(B, sched_dl);
    mac_rslicing_SliceConfig_dl_end(B);

    mac_rslicing_SliceConfig_ul_start(B);
      mac_rslicing_UlDlSliceConfig_algorithm_force_add(B, mac_rslicing_SliceAlgorithm_None);
      flatbuffers_string_ref_t sched_ul = flatbuffers_string_create_str(B, "pf_ul");
      mac_rslicing_UlDlSliceConfig_scheduler_add(B, sched_ul);
    mac_rslicing_SliceConfig_ul_end(B);
  mac_rslicing_SliceConfig_end_as_root(B);

  size_t size;
  uint8_t *buf = flatcc_builder_finalize_buffer(B, &size);
  g_slice_config = (byte_array_t) { .buf = buf, .len = size };

  int ret;
  if ((ret = mac_rslicing_SliceConfig_verify_as_root(buf, size))) {
    printf("SliceConfig is invalid: %s\n", flatcc_verify_error_string(ret));
    abort();
  }

  flatcc_builder_reset(B);

  mac_rslicing_UeSliceConfig_start(B);
  mac_rslicing_UeSliceConfig_ues_start(B);
  for (size_t i = 1; i < 3; ++i) {
    const uint32_t rnti = 37 * i;
    mac_rslicing_UeSliceAssoc_start(B);
    mac_rslicing_UeSliceAssoc_rnti_force_add(B, rnti);
    mac_rslicing_UeSliceConfig_ues_push(B, mac_rslicing_UeSliceAssoc_end(B));
  }
  mac_rslicing_UeSliceConfig_ues_end(B);
  mac_rslicing_UeSliceConfig_end_as_root(B);

  buf = flatcc_builder_finalize_buffer(B, &size);
  g_ue_slice_config = (byte_array_t) { .buf = buf, .len = size };

  if ((ret = mac_rslicing_UeSliceConfig_verify_as_root(buf, size))) {
    printf("UeSliceConfig is invalid: %s\n", flatcc_verify_error_string(ret));
    abort();
  }

  flatcc_builder_clear(B);
}

static
void rslicing_read_slice_conf(flatcc_builder_t* B)
{
  mac_rslicing_IndicationMessage_start(B);

  assert(g_slice_config.len > 0 && g_slice_config.buf);
  mac_rslicing_SliceConfig_table_t sc = mac_rslicing_SliceConfig_as_root(g_slice_config.buf);
  mac_rslicing_IndicationMessage_sliceConfig_clone(B, sc);
  /*
  mac_rslicing_IndicationMessage_sliceConfig_start(B);
    mac_rslicing_SliceConfig_dl_start(B);
      mac_rslicing_UlDlSliceConfig_algorithm_force_add(B, mac_rslicing_SliceAlgorithm_None);
      flatbuffers_string_ref_t sched_dl = flatbuffers_string_create_str(B, "pf_dl");
      mac_rslicing_UlDlSliceConfig_scheduler_add(B, sched_dl);
    mac_rslicing_SliceConfig_dl_end(B);

    mac_rslicing_SliceConfig_ul_start(B);
      mac_rslicing_UlDlSliceConfig_algorithm_force_add(B, mac_rslicing_SliceAlgorithm_None);
      flatbuffers_string_ref_t sched_ul = flatbuffers_string_create_str(B, "pf_ul");
      mac_rslicing_UlDlSliceConfig_scheduler_add(B, sched_ul);
    mac_rslicing_SliceConfig_ul_end(B);
  mac_rslicing_IndicationMessage_sliceConfig_end(B);
  */

  assert(g_ue_slice_config.len > 0 && g_ue_slice_config.buf);
  mac_rslicing_UeSliceConfig_table_t uesc = mac_rslicing_UeSliceConfig_as_root(g_ue_slice_config.buf);
  mac_rslicing_IndicationMessage_ueSliceConfig_clone(B, uesc);
  /*
  mac_rslicing_IndicationMessage_ueSliceConfig_start(B);
  mac_rslicing_UeSliceConfig_ues_start(B);
  for (size_t i = 1; i < 3; ++i) {
    const uint32_t rnti = 37 * i;
    mac_rslicing_UeSliceAssoc_start(B);
    mac_rslicing_UeSliceAssoc_rnti_force_add(B, rnti);
    mac_rslicing_UeSliceConfig_ues_push(B, mac_rslicing_UeSliceAssoc_end(B));
  }
  mac_rslicing_UeSliceConfig_ues_end(B);
  mac_rslicing_IndicationMessage_ueSliceConfig_end(B);
  */
  mac_rslicing_IndicationMessage_end_as_root(B);
}

static
rslicing_rc_t rslicing_handle_add_mod_slice_command(mac_rslicing_AddModSliceCommand_table_t t)
{
  assert(g_slice_config.len > 0 && g_slice_config.buf);
  mac_rslicing_SliceConfig_table_t sc = mac_rslicing_AddModSliceCommand_sliceConfig(t);
  if (mac_rslicing_SliceConfig_dl_is_present(sc)) {

    mac_rslicing_UlDlSliceConfig_table_t new_dl = mac_rslicing_SliceConfig_dl(sc);
    mac_rslicing_SliceConfig_table_t old_sc = mac_rslicing_SliceConfig_as_root(g_slice_config.buf);
    mac_rslicing_UlDlSliceConfig_table_t old_ul = mac_rslicing_SliceConfig_ul(old_sc);

    flatcc_builder_t builder;
    flatcc_builder_t* B = &builder;
    flatcc_builder_init(B);

    mac_rslicing_SliceConfig_start(B);
    mac_rslicing_SliceConfig_dl_clone(B, new_dl);
    mac_rslicing_SliceConfig_ul_clone(B, old_ul);
    mac_rslicing_SliceConfig_end_as_root(B);

    size_t size;
    uint8_t *buf = flatcc_builder_finalize_buffer(B, &size);
    free(g_slice_config.buf);
    g_slice_config = (byte_array_t) { .buf = buf, .len = size };

    flatcc_builder_clear(B);
  } else {
    printf("no DL in SliceConfig\n");
  }
  if (mac_rslicing_SliceConfig_ul_is_present(sc)) {
    printf("ignoring UL in SliceConfig\n");
  }
  return (rslicing_rc_t) { .success = true, .error_msg = NULL };
}

static
rslicing_rc_t rslicing_handle_del_slice_command(mac_rslicing_DelSliceCommand_table_t t)
{
  mac_rslicing_SliceConfig_table_t sc = mac_rslicing_SliceConfig_as_root(g_slice_config.buf);
  mac_rslicing_UlDlSliceConfig_table_t dl = mac_rslicing_SliceConfig_dl(sc);
  mac_rslicing_Slice_vec_t dlslices = mac_rslicing_UlDlSliceConfig_slices(dl);
  size_t n_dlslices = mac_rslicing_Slice_vec_len(dlslices);
  if (n_dlslices < 1)
    return (rslicing_rc_t) { .success = false, .error_msg = strdup("no slices configured") };

  bool slice_delete[n_dlslices];
  for (size_t i = 0; i < n_dlslices; ++i)
    slice_delete[i] = false;

  mac_rslicing_DelSliceConfig_table_t dsc = mac_rslicing_DelSliceCommand_delSliceConfig(t);
  flatbuffers_uint32_vec_t dl_ids = mac_rslicing_DelSliceConfig_dl(dsc);
  size_t n_ids = flatbuffers_uint32_vec_len(dl_ids);
  if (n_ids == n_dlslices)
    return (rslicing_rc_t) { .success = false, .error_msg = strdup("cannot delete all slices") };

  for (size_t i = 0; i < n_ids; ++i) {
    bool found = false;
    for (size_t j = 0; j < n_dlslices; ++j) {
      mac_rslicing_Slice_table_t slice = mac_rslicing_Slice_vec_at(dlslices, j);
      if (mac_rslicing_Slice_id(slice) == dl_ids[i]) {
        found = true;
        slice_delete[j] = true;
        break;
      }
    }
    if (!found)
      return (rslicing_rc_t) { .success = false, .error_msg = strdup("slice not found") };
  }

  flatcc_builder_t builder;
  flatcc_builder_t* B = &builder;
  flatcc_builder_init(B);

  mac_rslicing_SliceConfig_start(B);
  mac_rslicing_SliceConfig_dl_start(B);
  mac_rslicing_SliceAlgorithm_enum_t old_algo = mac_rslicing_UlDlSliceConfig_algorithm(dl);
  mac_rslicing_UlDlSliceConfig_algorithm_force_add(B, old_algo);
  mac_rslicing_UlDlSliceConfig_slices_start(B);
  for (size_t i = 0; i < n_dlslices; ++i) {
    mac_rslicing_Slice_table_t slice = mac_rslicing_Slice_vec_at(dlslices, i);
    if (slice_delete[i]) {
      printf("delete slice ID %d\n", mac_rslicing_Slice_id(slice));
      continue;
    }
    mac_rslicing_Slice_ref_t clone = mac_rslicing_Slice_clone(B, slice);
    mac_rslicing_UlDlSliceConfig_slices_push(B, clone);
  }
  mac_rslicing_UlDlSliceConfig_slices_end(B);
  mac_rslicing_SliceConfig_dl_end(B);

  mac_rslicing_UlDlSliceConfig_table_t old_ul = mac_rslicing_SliceConfig_ul(sc);
  mac_rslicing_SliceConfig_ul_clone(B, old_ul);

  mac_rslicing_SliceConfig_end_as_root(B);

  size_t size;
  uint8_t *buf = flatcc_builder_finalize_buffer(B, &size);
  free(g_slice_config.buf);
  g_slice_config = (byte_array_t) { .buf = buf, .len = size };

  int ret;
  if ((ret = mac_rslicing_SliceConfig_verify_as_root(buf, size))) {
    printf("SliceConfig is invalid: %s\n", flatcc_verify_error_string(ret));
    abort();
  }

  flatcc_builder_clear(B);

  return (rslicing_rc_t) { .success = true };
}

static
rslicing_rc_t rslicing_handle_ue_assoc_command(mac_rslicing_UeSliceAssocCommand_table_t t)
{
  //mac_rslicing_UeSliceConfig_table_t uesc = mac_rslicing_UeSliceConfig_as_root(g_ue_slice_config.buf);
  //mac_rslicing_UeSliceAssoc_vec_t ues = mac_rslicing_UeSliceConfig_ues(uesc);
  //size_t n_ues = mac_rslicing_UeSliceAssoc_vec_len(ues);

  mac_rslicing_UeSliceConfig_table_t new_uesc = mac_rslicing_UeSliceAssocCommand_ueSliceConfig(t);
  mac_rslicing_UeSliceAssoc_vec_t new_ues = mac_rslicing_UeSliceConfig_ues(new_uesc);
  size_t n_new_ues = mac_rslicing_UeSliceAssoc_vec_len(new_ues);
  for (size_t i = 0; i < n_new_ues; ++i) {
    mac_rslicing_UeSliceAssoc_table_t uesa = mac_rslicing_UeSliceAssoc_vec_at(new_ues, i);
    printf("UE RNTI %04x DL Slice ID %d\n",
            mac_rslicing_UeSliceAssoc_rnti(uesa),
            mac_rslicing_UeSliceAssoc_dlId(uesa));
  }

  return (rslicing_rc_t) { .success = true };
}

void rlc_gen_hdr(flatcc_builder_t* B)
{
  const uint16_t frame = 12;
  const uint16_t slot = 213;
  rlc_stats_IndicationHeader_create_as_root(B, frame, slot);
}

void rlc_gen_msg(flatcc_builder_t* B, const rlc_stats_report_style_t* style)
{
  rlc_stats_IndicationMessage_start(B);
  rlc_stats_IndicationMessage_ueStats_start(B);
  static uint32_t tmp = 1;
  for (size_t i = 0; i < num_ues; ++i) {
    // add in the order of rlc_stats_UEStats_create
    rlc_stats_UEStats_vec_push_start(B);
    const size_t num_rbs = 1;
    rlc_stats_UEStats_rb_start(B);
    for (size_t j = 0; j < num_rbs; ++j) {
      // add in the order of rlc_stats_RBStats_create
      rlc_stats_RBStats_vec_push_start(B);
      rlc_stats_RBStats_txPduBytes_add(B, tmp++);
      rlc_stats_RBStats_rxPduBytes_add(B, tmp++);
      if (style->type == RLC_STATS_REPORT_STYLE_TYPE_COMPLETE) {
        rlc_stats_RBStats_txPduDdBytes_add(B, tmp++);
        rlc_stats_RBStats_rxPduDdBytes_add(B, tmp++);
        rlc_stats_RBStats_txBufOccBytes_add(B, tmp++);
        rlc_stats_RBStats_rxBufOccBytes_add(B, tmp++);
      }
      rlc_stats_RBStats_txPduRetxBytes_add(B, tmp++);
      if (style->type == RLC_STATS_REPORT_STYLE_TYPE_COMPLETE) {
        rlc_stats_RBStats_rxPduOwBytes_add(B, tmp++);
        rlc_stats_RBStats_rxPduDupBytes_add(B, tmp++);
      }
      rlc_stats_RBStats_txPduPkts_add(B, tmp++);
      rlc_stats_RBStats_rxPduPkts_add(B, tmp++);
      if (style->type == RLC_STATS_REPORT_STYLE_TYPE_COMPLETE) {
        rlc_stats_RBStats_txPduDdPkts_add(B, tmp++);
        rlc_stats_RBStats_rxPduDdPkts_add(B, tmp++);
      }
      rlc_stats_RBStats_txBufOccPkts_add(B, tmp++);
      rlc_stats_RBStats_rxBufOccPkts_add(B, tmp++);
      rlc_stats_RBStats_txPduRetxPkts_add(B, tmp++);
      if (style->type == RLC_STATS_REPORT_STYLE_TYPE_COMPLETE) {
        rlc_stats_RBStats_txBufWdMs_add(B, tmp++);
        rlc_stats_RBStats_rxPduOwPkts_add(B, tmp++);
        rlc_stats_RBStats_rxPduDupPkts_add(B, tmp++);
        rlc_stats_RBStats_txPduWtMs_add(B, tmp++);
        rlc_stats_RBStats_rxPduRotoutMs_add(B, tmp++);
        rlc_stats_RBStats_rxPduPotoutMs_add(B, tmp++);
        rlc_stats_RBStats_rxPduSptoutMs_add(B, tmp++);
      }
      rlc_stats_RBStats_rbid_add(B, tmp++);
      rlc_stats_RBStats_vec_push_end(B);
    }
    rlc_stats_UEStats_rb_end(B);
    rlc_stats_UEStats_rnti_add(B, 37 * (i + 1));
    rlc_stats_UEStats_vec_push_end(B);
  }
  rlc_stats_IndicationMessage_ueStats_end(B);
  rlc_stats_IndicationMessage_end_as_root(B);
}

void pdcp_gen_hdr(flatcc_builder_t* B)
{
  const uint16_t frame = 1;
  const uint8_t slot = 80;
  pdcp_stats_IndicationHeader_create_as_root(B, frame, slot);
}

void pdcp_gen_msg(flatcc_builder_t* B, const pdcp_stats_report_style_t* style)
{
  pdcp_stats_IndicationMessage_start(B);
  pdcp_stats_IndicationMessage_ueStats_start(B);
  static uint32_t tmp = 1;
  for (size_t i = 0; i < num_ues; ++i) {
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
      if (style->type == PDCP_STATS_REPORT_STYLE_TYPE_COMPLETE) {
        pdcp_stats_RBStats_rxPduOoPkts_add(B, tmp++);
        pdcp_stats_RBStats_rxPduOoBytes_add(B, tmp++);
        pdcp_stats_RBStats_rxPduDdPkts_add(B, tmp++);
        pdcp_stats_RBStats_rxPduDdBytes_add(B, tmp++);
        pdcp_stats_RBStats_rxPduRoCount_add(B, tmp++);
      }
      pdcp_stats_RBStats_rbid_add(B, 3);
      pdcp_stats_RBStats_vec_push_end(B);
    }
    pdcp_stats_UEStats_rb_end(B);
    pdcp_stats_UEStats_rnti_add(B, 37 * (i + 1));
    pdcp_stats_UEStats_vec_push_end(B);
  }
  pdcp_stats_IndicationMessage_ueStats_end(B);
  pdcp_stats_IndicationMessage_end_as_root(B);

}

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
  while (true) {
    for (size_t i = 0; i < num_ues; ++i) {
      const uint16_t rnti = 37 * (i + 1);
      sm_rrc_stats_trigger_message(g_ag, rnti, fill_rrc_stats_msg, NULL);
      //printf("triggered RRC stats for RNTI %04x\n", rnti);
      usleep(26321);
    }
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

static
void generate_rrc_event_attach_message(flatcc_builder_t* B, const rrc_event_report_style_t* style, void* ctxt)
{
  assert(B);
  int selPlmnId = *(int*) ctxt;
  assert(style->type == rrc_event_ReportStyleType_Minimal);
  rrc_event_IndicationMessage_start(B);
  // in order rrc_event_IndicationMessage_create()
  rrc_event_IndicationMessage_selectedPlmnId_force_add(B, selPlmnId);
  rrc_event_IndicationMessage_rat_force_add(B, rrc_common_RAT_NR);
  rrc_event_IndicationMessage_end_as_root(B);
}

void generate_rrc_event_complete_message(flatcc_builder_t* B, const rrc_event_report_style_t* style, void* ctxt)
{
  assert(B);
  assert(!ctxt); // might be used be the UP to point to the UE to encode
  assert(style->type == rrc_event_ReportStyleType_Minimal);
  rrc_event_IndicationMessage_start(B);
  // in order rrc_event_IndicationMessage_create()
  rrc_event_IndicationMessage_rat_force_add(B, rrc_common_RAT_LTE);
  rrc_event_IndicationMessage_end_as_root(B);
}

static
void* rrc_event_trigger_two_ues(void* arg)
{
  e2ap_agent_t* ag = (e2ap_agent_t*) arg;
  const int ev_at = rrc_event_Event_Attach;
  const int ev_cp = rrc_event_Event_Complete;
  for (size_t i = 0; i < num_ues; ++i) {
    const uint16_t rnti = 37 * i;
    const int selPlmnId = i % 6;
    sm_rrc_event_trigger(ag, rnti, ev_at, generate_rrc_event_attach_message, (void*) &selPlmnId);
    usleep(25000);
    sm_rrc_event_trigger(ag, rnti, ev_cp, generate_rrc_event_complete_message, NULL);
    usleep(25000);
  }
  return NULL;
}

int main(int argc, char *argv[])
{
  if (argc > 3) {
    fprintf(stderr, "usage: %s [<nb_id> [<num_ues>]]\n", argv[0]);
    exit(1);
  }
  int nb_id = 5555;
  if (argc >= 2) {
    nb_id = atoi(argv[1]);
    if (argc == 3)
      num_ues = atoi(argv[2]);
  }
  printf("starting agent with nb_id %d num_ues %ld\n", nb_id, num_ues);

  const plmn_t plmn = {.mcc = 10, .mnc = 15, .mnc_digit_len = 2};
  const global_e2_node_id_t ge2ni = {.type = ngran_gNB, .plmn = plmn, .nb_id = 5555};
  g_ag = e2ap_init_agent("127.0.0.1", 36421, ge2ni);

  const mac_stats_report_style_t mac_style = { .type = MAC_STATS_REPORT_STYLE_TYPE_MINIMAL };
  mac_stats_callbacks_t mac_cb = {
    .hdr = mac_gen_hdr,
    .msg = mac_gen_msg
  };
  sm_mac_stats_register_ran_function(g_ag, mac_cb, &mac_style, 1);

  // we maintain local state instead of a user plane
  rslicing_init_config();
  const mac_rslicing_SliceAlgorithm_enum_t algos[3] = {
    mac_rslicing_SliceAlgorithm_None,
    mac_rslicing_SliceAlgorithm_Static,
    mac_rslicing_SliceAlgorithm_NVS
  };
  rslicing_cb_t rslicing_cb = {
    .read = rslicing_read_slice_conf,
    .add_mod = rslicing_handle_add_mod_slice_command,
    .del = rslicing_handle_del_slice_command,
    .ue_assoc = rslicing_handle_ue_assoc_command
  };
  sm_mac_rslicing_register_ran_function(g_ag, algos, 3, rslicing_cb);

  const rlc_stats_report_style_t rlc_style = { .type = RLC_STATS_REPORT_STYLE_TYPE_MINIMAL };
  rlc_stats_callbacks_t rlc_cb = {
    .hdr = rlc_gen_hdr,
    .msg = rlc_gen_msg
  };
  sm_rlc_stats_register_ran_function(g_ag, rlc_cb, &rlc_style, 1);

  const pdcp_stats_report_style_t pdcp_style = { .type = PDCP_STATS_REPORT_STYLE_TYPE_MINIMAL };
  pdcp_stats_callbacks_t pdcp_cb = {
    .hdr = pdcp_gen_hdr,
    .msg = pdcp_gen_msg
  };
  sm_pdcp_stats_register_ran_function(g_ag, pdcp_cb, &pdcp_style, 1);

  const rrc_stats_report_style_t rrc_stats_style = { .type = RRC_STATS_REPORT_STYLE_TYPE_MINIMAL };
  sm_rrc_stats_register_ran_function(g_ag, &rrc_stats_style, 1);
  pthread_t rrc_stats;
  pthread_create(&rrc_stats, NULL, periodic_rrc_stats_sender, NULL);

  const rrc_conf_report_style_t rrc_conf_style = { .type = RRC_CONF_REPORT_STYLE_TYPE_MINIMAL };
  sm_rrc_conf_register_ran_function(g_ag, fill_rrc_conf_msg, &rrc_conf_style, 1);

  rrc_event_report_style_t style = { .type = rrc_event_ReportStyleType_Minimal };
  sm_rrc_event_register_ran_function(g_ag, &style, 1);
  pthread_t rrc_event;
  pthread_create(&rrc_event, NULL, rrc_event_trigger_two_ues, g_ag);

  e2ap_start_agent(g_ag);

  e2ap_free_agent(g_ag);
  return 0;
}
