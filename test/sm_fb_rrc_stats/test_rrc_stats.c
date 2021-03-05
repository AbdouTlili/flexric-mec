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

#include "rrc_stats_msg_dec.h"
#include "rrc_stats_msg_enc.h"

#include "rrc_stats_reader.h"
#include "rrc_stats_verifier.h"
#include "rrc_stats_builder.h"
#include "rrc_stats_json_printer.h"

void test_types()
{
  static_assert(RRC_STATS_REPORT_STYLE_TYPE_MINIMAL == rrc_stats_ReportStyleType_Minimal, "mismatch of report style");
  static_assert(RRC_STATS_REPORT_STYLE_TYPE_COMPLETE == rrc_stats_ReportStyleType_Complete, "mismatch of report style");
  static_assert(RRC_STATS_REPORT_OCCASION_ONCHANGE == rrc_stats_ReportOccasion_onChange, "mismatch of report style");
}

void test_event_trigger()
{
  const rrc_stats_report_occasion_e occ = RRC_STATS_REPORT_OCCASION_ONCHANGE;
  byte_array_t ba = rrc_stats_encode_event_trigger(occ);
  assert(ba.buf && ba.len > 0);
  const rrc_stats_report_occasion_e ret = rrc_stats_decode_event_trigger(ba);
  assert(ret == occ);
  free(ba.buf);
}

void test_action_definition()
{
  const rrc_stats_report_style_t rs = { .type = RRC_STATS_REPORT_STYLE_TYPE_COMPLETE };
  byte_array_t ba = rrc_stats_encode_action_definition(rs);
  assert(ba.buf && ba.len > 0);
  const rrc_stats_report_style_t ret = rrc_stats_decode_action_definition(ba);
  assert(rs.type == ret.type);
  free(ba.buf);
}

void test_indication_header()
{
  const uint16_t rnti = 0x1234;
  byte_array_t ba = rrc_stats_encode_indication_header(rnti);
  assert(ba.buf && ba.len > 0);

  const uint16_t ret = rrc_stats_decode_indication_header(ba);
  assert(ret == rnti);
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

void generate_rrc_stats_indication_message(flatcc_builder_t* B, const rrc_stats_report_style_t* style, void* ctxt)
{
  assert(B);
  assert(!ctxt); // might be used be the UP to point to the UE to encode
  assert(style->type == RRC_STATS_REPORT_STYLE_TYPE_MINIMAL || style->type == RRC_STATS_REPORT_STYLE_TYPE_COMPLETE);
  rrc_stats_IndicationMessage_start(B);
  rrc_stats_IndicationMessage_measReport_start(B);
  // only a single measReport. The point of the list is to accumulate them in
  // the controller
  // Note: add in order of rrc_stats_MeasReport_create()
  for (size_t i = 0; i < 1; ++i) {
    rrc_stats_MeasReport_vec_push_start(B);
    rrc_stats_MeasReport_measId_force_add(B, 3);
    rrc_stats_MeasReport_pCellResult_create(B, 12, 13, 0);
    rrc_stats_MeasReport_neighMeas_start(B);
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
    rrc_stats_MeasReport_neighMeas_end(B);
    if (style->type == RRC_STATS_REPORT_STYLE_TYPE_COMPLETE) {
      const char* c = "RRC_Container";
      rrc_stats_MeasReport_rrcContainer_create(B, (uint8_t*) c, strlen(c));
    }
    rrc_stats_MeasReport_vec_push_end(B);
  }
  rrc_stats_IndicationMessage_measReport_end(B);
  rrc_stats_IndicationMessage_end_as_root(B);
}

void test_complete_indication_message()
{
  rrc_stats_report_style_t style = { .type = RRC_STATS_REPORT_STYLE_TYPE_COMPLETE };
  byte_array_t ba = rrc_stats_encode_indication_message(generate_rrc_stats_indication_message, &style, NULL);
  assert(ba.buf && ba.len > 0);

  int ret;
  if ((ret = rrc_stats_IndicationMessage_verify_as_root(ba.buf, ba.len))) {
    printf("IndicationMessage is invalid: %s\n", flatcc_verify_error_string(ret));
    abort();
  }

  /* we are ok if JSON prints for the moment. TODO: make full test and verify
   * that generic generate_rrc_stats_indication_message() encodes all fields
   * properly */
  flatcc_json_printer_t printer_ctx;
  flatcc_json_printer_init_dynamic_buffer(&printer_ctx, 0);
  printer_ctx.indent = 1;
  rrc_stats_IndicationMessage_print_json_as_root(&printer_ctx, ba.buf, ba.len, NULL);
  size_t buf_size;
  char *buf = (char*)flatcc_json_printer_get_buffer(&printer_ctx, &buf_size);
  //printf("%s\n", buf);
  flatcc_json_printer_clear(&printer_ctx);

  free(ba.buf);
}

void test_ran_function()
{
  const size_t num = 2;
  rrc_stats_report_style_t rs[num];
  rs[0].type = RRC_STATS_REPORT_STYLE_TYPE_COMPLETE;
  rs[1].type = RRC_STATS_REPORT_STYLE_TYPE_MINIMAL;
  byte_array_t ba = rrc_stats_encode_ran_function(rs, num);
  assert(ba.buf && ba.len > 0);
  rrc_stats_report_style_t* ret;
  const size_t ret_num = rrc_stats_decode_ran_function(ba, &ret);
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
