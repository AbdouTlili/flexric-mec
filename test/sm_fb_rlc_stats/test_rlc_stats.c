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

#include "rlc_stats_msg_dec.h"
#include "rlc_stats_msg_enc.h"

#include "rlc_stats_reader.h"
#include "rlc_stats_verifier.h"
#include "rlc_stats_builder.h"
#include "rlc_stats_json_printer.h"

void test_types()
{
  static_assert(RLC_STATS_REPORT_STYLE_TYPE_MINIMAL == rlc_stats_ReportStyleType_Minimal, "mismatch of ReportStyle");
  static_assert(RLC_STATS_REPORT_STYLE_TYPE_COMPLETE == rlc_stats_ReportStyleType_Complete, "mismatch of ReportStyle");
}

void test_event_trigger()
{
  const uint16_t ms = 1337;
  byte_array_t ba = rlc_stats_encode_event_trigger(ms);
  assert(ba.buf && ba.len > 0);
  const uint16_t ret = rlc_stats_decode_event_trigger(ba);
  assert(ret == ms);
  free(ba.buf);
}

void test_action_definition()
{
  const rlc_stats_report_style_t rs = { .type = RLC_STATS_REPORT_STYLE_TYPE_MINIMAL };
  byte_array_t ba = rlc_stats_encode_action_definition(rs);
  assert(ba.buf && ba.len > 0);
  const rlc_stats_report_style_t ret = rlc_stats_decode_action_definition(ba);
  assert(rs.type == ret.type);
  free(ba.buf);
}

void test_indication_header()
{
  const uint16_t frame = 10;
  const uint16_t slot = 12;
  void gen_hdr(flatcc_builder_t* B) {
    rlc_stats_IndicationHeader_create_as_root(B, frame, slot);
  }
  byte_array_t ba = rlc_stats_encode_indication_header(gen_hdr);
  assert(ba.buf && ba.len > 0);

  int ret;
  if ((ret = rlc_stats_IndicationHeader_verify_as_root(ba.buf, ba.len))) {
    printf("IndicationHeader is invalid: %s\n", flatcc_verify_error_string(ret));
    abort();
  }
  rlc_stats_IndicationHeader_table_t ih = rlc_stats_IndicationHeader_as_root(ba.buf);
  assert(rlc_stats_IndicationHeader_frame(ih) == frame);
  assert(rlc_stats_IndicationHeader_slot(ih) == slot);
  free(ba.buf);
}


void test_indication_message()
{
  size_t num = 10;
  size_t num_verif = num;
  size_t tmp = 10;
  size_t tmp_verif = num;
  void gen_msg(flatcc_builder_t* B, const rlc_stats_report_style_t* style) {
    rlc_stats_IndicationMessage_start(B);
    rlc_stats_IndicationMessage_ueStats_start(B);
    const size_t num_stats = num++;
    for (size_t i = 0; i < num_stats; ++i) {
      // add in the order of rlc_stats_UEStats_create
      rlc_stats_UEStats_vec_push_start(B);
      const size_t num_rbs = num++;
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
      rlc_stats_UEStats_rnti_add(B, tmp++);
      rlc_stats_UEStats_vec_push_end(B);
    }
    rlc_stats_IndicationMessage_ueStats_end(B);
    rlc_stats_IndicationMessage_end_as_root(B);
  }

  rlc_stats_report_style_t style = { .type = RLC_STATS_REPORT_STYLE_TYPE_COMPLETE };
  byte_array_t ba = rlc_stats_encode_indication_message(gen_msg, &style);
  assert(ba.buf && ba.len > 0);

  int ret;
  if ((ret = rlc_stats_IndicationMessage_verify_as_root(ba.buf, ba.len))) {
    printf("IndicationMessage is invalid: %s\n", flatcc_verify_error_string(ret));
    abort();
  }

  /* JSON prints? */
  flatcc_json_printer_t printer_ctx;
  flatcc_json_printer_init_dynamic_buffer(&printer_ctx, 0);
  //printer_ctx.indent = 1;
  rlc_stats_IndicationMessage_print_json_as_root(&printer_ctx, ba.buf, ba.len, NULL);
  size_t buf_size;
  char *buf = (char*)flatcc_json_printer_get_buffer(&printer_ctx, &buf_size);
  //printf("%s\n", buf);
  flatcc_json_printer_clear(&printer_ctx);

  rlc_stats_IndicationMessage_table_t im = rlc_stats_IndicationMessage_as_root(ba.buf);
  rlc_stats_UEStats_vec_t uestats_vec = rlc_stats_IndicationMessage_ueStats(im);
  const size_t uestats_vec_len = rlc_stats_UEStats_vec_len(uestats_vec);
  assert(uestats_vec_len == num_verif);
  num_verif++;
  for (size_t i = 0; i < uestats_vec_len; ++i) {
    rlc_stats_UEStats_table_t ue_stats = rlc_stats_UEStats_vec_at(uestats_vec, i);
    /* check in order of how we stored it */
    rlc_stats_RBStats_vec_t rbstats_vec = rlc_stats_UEStats_rb(ue_stats);
    const size_t rbstats_vec_len = rlc_stats_RBStats_vec_len(rbstats_vec);
    assert(rbstats_vec_len == num_verif);
    num_verif++;
    for (size_t j = 0; j < rbstats_vec_len; ++j) {
      rlc_stats_RBStats_table_t rb_stats = rlc_stats_RBStats_vec_at(rbstats_vec, j);
      assert(rlc_stats_RBStats_txPduBytes(rb_stats) == tmp_verif++);
      assert(rlc_stats_RBStats_rxPduBytes(rb_stats) == tmp_verif++);
      assert(rlc_stats_RBStats_txPduDdBytes(rb_stats) == tmp_verif++);
      assert(rlc_stats_RBStats_rxPduDdBytes(rb_stats) == tmp_verif++);
      assert(rlc_stats_RBStats_txBufOccBytes(rb_stats) == tmp_verif++);
      assert(rlc_stats_RBStats_rxBufOccBytes(rb_stats) == tmp_verif++);
      assert(rlc_stats_RBStats_txPduRetxBytes(rb_stats) == tmp_verif++);
      assert(rlc_stats_RBStats_rxPduOwBytes(rb_stats) == tmp_verif++);
      assert(rlc_stats_RBStats_rxPduDupBytes(rb_stats) == tmp_verif++);
      assert(rlc_stats_RBStats_txPduPkts(rb_stats) == tmp_verif++);
      assert(rlc_stats_RBStats_rxPduPkts(rb_stats) == tmp_verif++);
      assert(rlc_stats_RBStats_txPduDdPkts(rb_stats) == tmp_verif++);
      assert(rlc_stats_RBStats_rxPduDdPkts(rb_stats) == tmp_verif++);
      assert(rlc_stats_RBStats_txBufOccPkts(rb_stats) == tmp_verif++);
      assert(rlc_stats_RBStats_rxBufOccPkts(rb_stats) == tmp_verif++);
      assert(rlc_stats_RBStats_txPduRetxPkts(rb_stats) == tmp_verif++);
      assert(rlc_stats_RBStats_txBufWdMs(rb_stats) == tmp_verif++);
      assert(rlc_stats_RBStats_rxPduOwPkts(rb_stats) == tmp_verif++);
      assert(rlc_stats_RBStats_rxPduDupPkts(rb_stats) == tmp_verif++);
      assert(rlc_stats_RBStats_txPduWtMs(rb_stats) == (uint16_t) tmp_verif++);
      assert(rlc_stats_RBStats_rxPduRotoutMs(rb_stats) == (uint16_t) tmp_verif++);
      assert(rlc_stats_RBStats_rxPduPotoutMs(rb_stats) == (uint16_t) tmp_verif++);
      assert(rlc_stats_RBStats_rxPduSptoutMs(rb_stats) == (uint16_t) tmp_verif++);
      assert(rlc_stats_RBStats_rbid(rb_stats) == (uint8_t)tmp_verif++);
    }
    assert(rlc_stats_UEStats_rnti(ue_stats) == tmp_verif++);
  }
  free(ba.buf);
}

void test_ran_function()
{
  const size_t num = 1;
  rlc_stats_report_style_t rs[num];
  rs[0].type = RLC_STATS_REPORT_STYLE_TYPE_COMPLETE;
  byte_array_t ba = rlc_stats_encode_ran_function(rs, num);
  assert(ba.buf && ba.len > 0);
  rlc_stats_report_style_t* ret;
  const size_t ret_num = rlc_stats_decode_ran_function(ba, &ret);
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
