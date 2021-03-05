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

#include "rrc_conf_msg_dec.h"
#include "rrc_conf_msg_enc.h"

#include "rrc_conf_reader.h"
#include "rrc_conf_verifier.h"
#include "rrc_conf_builder.h"
#include "rrc_conf_json_printer.h"

void test_types()
{
  static_assert(RRC_CONF_REPORT_STYLE_TYPE_MINIMAL == rrc_conf_ReportStyleType_Minimal, "mismatch of report style");
  static_assert(RRC_CONF_REPORT_STYLE_TYPE_COMPLETE == rrc_conf_ReportStyleType_Complete, "mismatch of report style");
  static_assert(RRC_CONF_REPORT_OCCASION_ONCHANGE == rrc_conf_ReportOccasion_onChange, "mismatch of report style");
}

void test_event_trigger()
{
  const rrc_conf_report_occasion_e occ = RRC_CONF_REPORT_OCCASION_ONCHANGE;
  byte_array_t ba = rrc_conf_encode_event_trigger(occ);
  assert(ba.buf && ba.len > 0);
  const rrc_conf_report_occasion_e ret = rrc_conf_decode_event_trigger(ba);
  assert(ret == occ);
  free(ba.buf);
}

void test_action_definition()
{
  const rrc_conf_report_style_t rs = { .type = RRC_CONF_REPORT_STYLE_TYPE_COMPLETE };
  byte_array_t ba = rrc_conf_encode_action_definition(rs);
  assert(ba.buf && ba.len > 0);
  const rrc_conf_report_style_t ret = rrc_conf_decode_action_definition(ba);
  assert(rs.type == ret.type);
  free(ba.buf);
}

void test_indication_header()
{
  /* nothing is encoded, just test that it does not crash */
  byte_array_t ba = rrc_conf_encode_indication_header();
  assert(ba.buf && ba.len > 0);
  free(ba.buf);
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

void generate_rrc_conf_indication_message(flatcc_builder_t* B, const rrc_conf_report_style_t* style)
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
        rrc_conf_CarrierInfo_nr_create(B, 1);
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

void test_complete_indication_message()
{
  rrc_conf_report_style_t style = { .type = RRC_CONF_REPORT_STYLE_TYPE_COMPLETE };
  byte_array_t ba = rrc_conf_encode_indication_message(generate_rrc_conf_indication_message, &style);
  assert(ba.buf && ba.len > 0);

  int ret;
  if ((ret = rrc_conf_IndicationMessage_verify_as_root(ba.buf, ba.len))) {
    printf("IndicationMessage is invalid: %s\n", flatcc_verify_error_string(ret));
    abort();
  }

  /* we are ok if JSON prints for the moment. TODO: make full test and verify
   * that generic generate_rrc_conf_indication_message() encodes all fields
   * properly */
  flatcc_json_printer_t printer_ctx;
  flatcc_json_printer_init_dynamic_buffer(&printer_ctx, 0);
  printer_ctx.indent = 2;
  rrc_conf_IndicationMessage_print_json_as_root(&printer_ctx, ba.buf, ba.len, NULL);
  size_t buf_size;
  char *buf = (char*)flatcc_json_printer_get_buffer(&printer_ctx, &buf_size);
  //printf("%s\n", buf);
  flatcc_json_printer_clear(&printer_ctx);

  free(ba.buf);
}

void test_ran_function()
{
  const size_t num = 2;
  rrc_conf_report_style_t rs[num];
  rs[0].type = RRC_CONF_REPORT_STYLE_TYPE_COMPLETE;
  rs[1].type = RRC_CONF_REPORT_STYLE_TYPE_MINIMAL;
  byte_array_t ba = rrc_conf_encode_ran_function(rs, num);
  assert(ba.buf && ba.len > 0);
  rrc_conf_report_style_t* ret;
  const size_t ret_num = rrc_conf_decode_ran_function(ba, &ret);
  assert(ret_num == num);
  for (size_t i = 0; i < num; ++i)
    assert(rs[i].type == ret[i].type);
  free(ba.buf);
  free(ret);
}

int main()
{
  test_types();
  test_event_trigger();
  test_action_definition();
  test_indication_header();
  test_complete_indication_message();
  test_ran_function();
}
