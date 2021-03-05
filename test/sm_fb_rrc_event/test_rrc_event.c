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

#include "rrc_event_msg_dec.h"
#include "rrc_event_msg_enc.h"

#include "rrc_event_reader.h"
#include "rrc_event_verifier.h"
#include "rrc_event_builder.h"
#include "rrc_event_json_printer.h"

void test_event_trigger()
{
  const rrc_event_ReportOccasion_enum_t occ = rrc_event_ReportOccasion_onChange;
  byte_array_t ba = rrc_event_encode_event_trigger(occ);
  assert(ba.buf && ba.len > 0);
  const rrc_event_ReportOccasion_enum_t ret = rrc_event_decode_event_trigger(ba);
  assert(ret == occ);
  free(ba.buf);
}

void test_action_definition()
{
  const rrc_event_report_style_t rs = { .type = rrc_event_ReportStyleType_Complete };
  byte_array_t ba = rrc_event_encode_action_definition(rs);
  assert(ba.buf && ba.len > 0);
  const rrc_event_report_style_t ret = rrc_event_decode_action_definition(ba);
  assert(rs.type == ret.type);
  free(ba.buf);
}

void test_indication_header()
{
  const rrc_event_indication_header_t hdr = {
    .rnti = 0xdead,
    .event = rrc_event_Event_Complete
  };
  byte_array_t ba = rrc_event_encode_indication_header(hdr);
  assert(ba.buf && ba.len > 0);

  const rrc_event_indication_header_t ret = rrc_event_decode_indication_header(ba);
  assert(ret.rnti == hdr.rnti);
  assert(ret.event == hdr.event);
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

void generate_rrc_event_attach_message(flatcc_builder_t* B, const rrc_event_report_style_t* style, void* ctxt)
{
  assert(B);
  assert(!ctxt); // might be used be the UP to point to the UE to encode
  assert(style->type == rrc_event_ReportStyleType_Minimal || style->type == rrc_event_ReportStyleType_Complete);
  rrc_event_IndicationMessage_start(B);
  // in order rrc_event_IndicationMessage_create()
  rrc_event_NrExt_start(B);
    rrc_event_NrExt_sNssaiList_start(B);
      rrc_common_Snssai_vec_push_create(B, 1, 2);
      rrc_common_Snssai_vec_push_create(B, 1, 3);
    rrc_event_NrExt_sNssaiList_end(B);
  rrc_event_NrExt_ref_t nrext = rrc_event_NrExt_end(B);
  rrc_event_IndicationMessage_nrExt_add(B, nrext);
  rrc_event_IndicationMessage_rat_force_add(B, rrc_common_RAT_NR);
  rrc_event_IndicationMessage_selectedPlmnId_force_add(B, 0);
  rrc_event_IndicationMessage_end_as_root(B);
}

void generate_rrc_event_complete_message(flatcc_builder_t* B, const rrc_event_report_style_t* style, void* ctxt)
{
  assert(B);
  assert(!ctxt); // might be used be the UP to point to the UE to encode
  assert(style->type == rrc_event_ReportStyleType_Minimal || style->type == rrc_event_ReportStyleType_Complete);
  rrc_event_IndicationMessage_start(B);
  // in order rrc_event_IndicationMessage_create()
  rrc_event_IndicationMessage_imsi_create_str(B, "208950121");
  static const char* cap = "FANCY_CAPABILITY";
  rrc_event_IndicationMessage_ueCapabilityRrcContainer_create(B, (uint8_t*) cap, strlen(cap));
  rrc_event_LTECapability_ref_t ltecap = rrc_event_LTECapability_create(B, 14, 12, true);
  rrc_event_IndicationMessage_lteExt_create(B, ltecap);
  rrc_event_IndicationMessage_rat_force_add(B, rrc_common_RAT_LTE);
  rrc_event_IndicationMessage_end_as_root(B);
}

void test_indication_message(rrc_event_fill_ind_msg_cb f)
{
  rrc_event_report_style_t style = { .type = rrc_event_ReportStyleType_Complete };
  byte_array_t ba = rrc_event_encode_indication_message(f, &style, NULL);
  assert(ba.buf && ba.len > 0);

  int ret;
  if ((ret = rrc_event_IndicationMessage_verify_as_root(ba.buf, ba.len))) {
    printf("IndicationMessage is invalid: %s\n", flatcc_verify_error_string(ret));
    abort();
  }

  /* we are ok if JSON prints for the moment. TODO: make full test and verify
   * that generic generate_rrc_event_indication_message() encodes all fields
   * properly */
  flatcc_json_printer_t printer_ctx;
  flatcc_json_printer_init_dynamic_buffer(&printer_ctx, 0);
  printer_ctx.indent = 1;
  rrc_event_IndicationMessage_print_json_as_root(&printer_ctx, ba.buf, ba.len, NULL);
  size_t buf_size;
  char *buf = (char*)flatcc_json_printer_get_buffer(&printer_ctx, &buf_size);
  //printf("%s\n", buf);
  flatcc_json_printer_clear(&printer_ctx);

  free(ba.buf);
}

void test_ran_function()
{
  const size_t num = 2;
  rrc_event_report_style_t rs[num];
  rs[0].type = rrc_event_ReportStyleType_Complete;
  rs[1].type = rrc_event_ReportStyleType_Minimal;
  byte_array_t ba = rrc_event_encode_ran_function(rs, num);
  assert(ba.buf && ba.len > 0);
  rrc_event_report_style_t* ret;
  const size_t ret_num = rrc_event_decode_ran_function(ba, &ret);
  assert(ret_num == num);
  for (size_t i = 0; i < num; ++i)
    assert(rs[i].type == ret[i].type);
  free(ba.buf);
  free(ret);
}

int main()
{
  test_event_trigger();
  test_action_definition();
  test_indication_header();
  test_indication_message(generate_rrc_event_attach_message);
  test_indication_message(generate_rrc_event_complete_message);
  test_ran_function();
}
