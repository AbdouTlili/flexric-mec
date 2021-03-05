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

#include "pdcp_stats_msg_dec.h"
#include "pdcp_stats_msg_enc.h"

#include "pdcp_stats_reader.h"
#include "pdcp_stats_verifier.h"
#include "pdcp_stats_builder.h"
#include "pdcp_stats_json_printer.h"

void test_types()
{
  static_assert(PDCP_STATS_REPORT_STYLE_TYPE_MINIMAL == pdcp_stats_ReportStyleType_Minimal, "mismatch of report style");
  static_assert(PDCP_STATS_REPORT_STYLE_TYPE_COMPLETE == pdcp_stats_ReportStyleType_Complete, "mismatch of report style");
}

void test_event_trigger()
{
  const uint16_t ms = 1337;
  byte_array_t ba = pdcp_stats_encode_event_trigger(ms);
  assert(ba.buf && ba.len > 0);
  const uint16_t ret = pdcp_stats_decode_event_trigger(ba);
  assert(ret == ms);
  free(ba.buf);
}

void test_action_definition()
{
  const pdcp_stats_report_style_t rs = { .type = PDCP_STATS_REPORT_STYLE_TYPE_MINIMAL };
  byte_array_t ba = pdcp_stats_encode_action_definition(rs);
  assert(ba.buf && ba.len > 0);
  const pdcp_stats_report_style_t ret = pdcp_stats_decode_action_definition(ba);
  assert(rs.type == ret.type);
  free(ba.buf);
}

void test_indication_header()
{
  const uint16_t frame = 10;
  const uint16_t slot = 12;
  void gen_hdr(flatcc_builder_t* B) {
    pdcp_stats_IndicationHeader_create_as_root(B, frame, slot);
  }
  byte_array_t ba = pdcp_stats_encode_indication_header(gen_hdr);
  assert(ba.buf && ba.len > 0);

  int ret;
  if ((ret = pdcp_stats_IndicationHeader_verify_as_root(ba.buf, ba.len))) {
    printf("IndicationHeader is invalid: %s\n", flatcc_verify_error_string(ret));
    abort();
  }
  pdcp_stats_IndicationHeader_table_t ih = pdcp_stats_IndicationHeader_as_root(ba.buf);
  assert(pdcp_stats_IndicationHeader_frame(ih) == frame);
  assert(pdcp_stats_IndicationHeader_slot(ih) == slot);
  free(ba.buf);
}


void test_indication_message()
{
  size_t num = 10;
  size_t num_verif = num;
  size_t tmp = 10;
  size_t tmp_verif = num;
  void gen_msg(flatcc_builder_t* B, const pdcp_stats_report_style_t* style) {
    pdcp_stats_IndicationMessage_start(B);
    pdcp_stats_IndicationMessage_ueStats_start(B);
    const size_t num_stats = num++;
    for (size_t i = 0; i < num_stats; ++i) {
      // add in the order of pdcp_UEStats_create
      pdcp_stats_UEStats_vec_push_start(B);
      const size_t num_rbs = num++;
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
        pdcp_stats_RBStats_rbid_add(B, tmp++);
        pdcp_stats_RBStats_vec_push_end(B);
      }
      pdcp_stats_UEStats_rb_end(B);
      pdcp_stats_UEStats_rnti_add(B, tmp++);
      pdcp_stats_UEStats_vec_push_end(B);
    }
    pdcp_stats_IndicationMessage_ueStats_end(B);
    pdcp_stats_IndicationMessage_end_as_root(B);
  }

  pdcp_stats_report_style_t style = { .type = PDCP_STATS_REPORT_STYLE_TYPE_COMPLETE };
  byte_array_t ba = pdcp_stats_encode_indication_message(gen_msg, &style);
  assert(ba.buf && ba.len > 0);

  int ret;
  if ((ret = pdcp_stats_IndicationMessage_verify_as_root(ba.buf, ba.len))) {
    printf("IndicationMessage is invalid: %s\n", flatcc_verify_error_string(ret));
    abort();
  }

  /* JSON prints? */
  flatcc_json_printer_t printer_ctx;
  flatcc_json_printer_init_dynamic_buffer(&printer_ctx, 0);
  //printer_ctx.indent = 1;
  pdcp_stats_IndicationMessage_print_json_as_root(&printer_ctx, ba.buf, ba.len, NULL);
  size_t buf_size;
  char *buf = (char*)flatcc_json_printer_get_buffer(&printer_ctx, &buf_size);
  //printf("%s\n", buf);
  flatcc_json_printer_clear(&printer_ctx);

  pdcp_stats_IndicationMessage_table_t im = pdcp_stats_IndicationMessage_as_root(ba.buf);
  pdcp_stats_UEStats_vec_t uestats_vec = pdcp_stats_IndicationMessage_ueStats(im);
  const size_t uestats_vec_len = pdcp_stats_UEStats_vec_len(uestats_vec);
  assert(uestats_vec_len == num_verif);
  num_verif++;
  for (size_t i = 0; i < uestats_vec_len; ++i) {
    pdcp_stats_UEStats_table_t ue_stats = pdcp_stats_UEStats_vec_at(uestats_vec, i);
    /* check in order of how we stored it */
    pdcp_stats_RBStats_vec_t rbstats_vec = pdcp_stats_UEStats_rb(ue_stats);
    const size_t rbstats_vec_len = pdcp_stats_RBStats_vec_len(rbstats_vec);
    assert(rbstats_vec_len == num_verif);
    num_verif++;
    for (size_t j = 0; j < rbstats_vec_len; ++j) {
      pdcp_stats_RBStats_table_t rb_stats = pdcp_stats_RBStats_vec_at(rbstats_vec, j);
      assert(pdcp_stats_RBStats_txPduPkts(rb_stats) == tmp_verif++);
      assert(pdcp_stats_RBStats_rxPduPkts(rb_stats) == tmp_verif++);
      assert(pdcp_stats_RBStats_txPduBytes(rb_stats) == tmp_verif++);
      assert(pdcp_stats_RBStats_rxPduBytes(rb_stats) == tmp_verif++);
      assert(pdcp_stats_RBStats_txPduSn(rb_stats) == tmp_verif++);
      assert(pdcp_stats_RBStats_rxPduSn(rb_stats) == tmp_verif++);
      assert(pdcp_stats_RBStats_rxPduOoPkts(rb_stats) == tmp_verif++);
      assert(pdcp_stats_RBStats_rxPduOoBytes(rb_stats) == tmp_verif++);
      assert(pdcp_stats_RBStats_rxPduDdPkts(rb_stats) == tmp_verif++);
      assert(pdcp_stats_RBStats_rxPduDdBytes(rb_stats) == tmp_verif++);
      assert(pdcp_stats_RBStats_rxPduRoCount(rb_stats) == tmp_verif++);
      /* rbid is uint8_t, so truncate integer! */
      assert(pdcp_stats_RBStats_rbid(rb_stats) == (uint8_t)tmp_verif++);
    }
    assert(pdcp_stats_UEStats_rnti(ue_stats) == tmp_verif++);
  }
  free(ba.buf);
}

void test_ran_function()
{
  const size_t num = 1;
  pdcp_stats_report_style_t rs[num];
  rs[0].type = PDCP_STATS_REPORT_STYLE_TYPE_COMPLETE;
  byte_array_t ba = pdcp_stats_encode_ran_function(rs, num);
  assert(ba.buf && ba.len > 0);
  pdcp_stats_report_style_t* ret;
  const size_t ret_num = pdcp_stats_decode_ran_function(ba, &ret);
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
  test_indication_message();
  test_ran_function();
}
