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

#include "mac_stats_msg_dec.h"
#include "mac_stats_msg_enc.h"

#include "mac_stats_reader.h"
#include "mac_stats_verifier.h"
#include "mac_stats_builder.h"
#include "mac_stats_json_printer.h"

void test_types()
{
  static_assert(MAC_STATS_REPORT_STYLE_TYPE_MINIMAL == mac_stats_ReportStyleType_Minimal, "mismatch of report style");
  static_assert(MAC_STATS_REPORT_STYLE_TYPE_COMPLETE == mac_stats_ReportStyleType_Complete, "mismatch of report style");
}

void test_event_trigger()
{
  const uint16_t ms = 1337;
  byte_array_t ba = mac_stats_encode_event_trigger(ms);
  assert(ba.buf && ba.len > 0);
  const uint16_t ret = mac_stats_decode_event_trigger(ba);
  assert(ret == ms);
  free(ba.buf);
}

void test_action_definition()
{
  const mac_stats_report_style_t rs = { .type = MAC_STATS_REPORT_STYLE_TYPE_MINIMAL };
  byte_array_t ba = mac_stats_encode_action_definition(rs);
  assert(ba.buf && ba.len > 0);
  const mac_stats_report_style_t ret = mac_stats_decode_action_definition(ba);
  assert(rs.type == ret.type);
  free(ba.buf);
}

void test_indication_header()
{
  const uint16_t frame = 12;
  const uint16_t slot = 13;
  void gen_hdr(flatcc_builder_t* B) {
    mac_stats_IndicationHeader_create_as_root(B, frame, slot);
  }
  byte_array_t ba = mac_stats_encode_indication_header(gen_hdr);
  assert(ba.buf && ba.len > 0);

  int ret;
  if ((ret = mac_stats_IndicationHeader_verify_as_root(ba.buf, ba.len))) {
    printf("IndicationHeader is invalid: %s\n", flatcc_verify_error_string(ret));
    abort();
  }
  mac_stats_IndicationHeader_table_t ih = mac_stats_IndicationHeader_as_root(ba.buf);
  assert(mac_stats_IndicationHeader_frame(ih) == frame);
  assert(mac_stats_IndicationHeader_slot(ih) == slot);
  free(ba.buf);
}

void test_indication_message()
{
  size_t num = 10;
  size_t num_verif = num;
  size_t tmp = 10;
  size_t tmp_verif = num;
  void gen_msg(flatcc_builder_t* B, const mac_stats_report_style_t* style) {
    mac_stats_IndicationMessage_start(B);
    mac_stats_IndicationMessage_ueStats_start(B);
    const size_t num_stats = num++;
    for (size_t i = 0; i < num_stats; ++i) {
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
        mac_stats_UEStats_puschSnr_add(B, tmp++);
        mac_stats_UEStats_pucchSnr_add(B, tmp++);
        const size_t numBsr = tmp++ % 7;
        if (numBsr > 0) {
          mac_stats_UEStats_bsr_start(B);
          for (uint8_t j = 0; j < numBsr; ++j) {
            const uint8_t lcgid = tmp++;
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
      mac_stats_UEStats_rnti_add(B, tmp++);
      if (style->type == MAC_STATS_REPORT_STYLE_TYPE_COMPLETE) {
        mac_stats_UEStats_wbCqi_add(B, tmp++);
        mac_stats_UEStats_dlMcs1_add(B, tmp++);
        mac_stats_UEStats_ulMcs1_add(B, tmp++);
        mac_stats_UEStats_dlMcs2_add(B, tmp++);
        mac_stats_UEStats_ulMcs2_add(B, tmp++);
        mac_stats_UEStats_phr_add(B, tmp++);
      }
      mac_stats_UEStats_vec_push_end(B);
    }
    mac_stats_IndicationMessage_ueStats_end(B);
    mac_stats_IndicationMessage_end_as_root(B);
  }

  mac_stats_report_style_t style = { .type = MAC_STATS_REPORT_STYLE_TYPE_COMPLETE };
  byte_array_t ba = mac_stats_encode_indication_message(gen_msg, &style);
  assert(ba.buf && ba.len > 0);
  assert(ba.buf && ba.len > 0);
  int ret;
  if ((ret = mac_stats_IndicationMessage_verify_as_root(ba.buf, ba.len))) {
    printf("IndicationMessage is invalid: %s\n", flatcc_verify_error_string(ret));
    abort();
  }

  /* JSON prints? */
  flatcc_json_printer_t printer_ctx;
  flatcc_json_printer_init_dynamic_buffer(&printer_ctx, 0);
  //printer_ctx.indent = 1;
  mac_stats_IndicationMessage_print_json_as_root(&printer_ctx, ba.buf, ba.len, NULL);
  size_t buf_size;
  char *buf = (char*)flatcc_json_printer_get_buffer(&printer_ctx, &buf_size);
  //printf("%s\n", buf);
  flatcc_json_printer_clear(&printer_ctx);

  mac_stats_IndicationMessage_table_t im = mac_stats_IndicationMessage_as_root(ba.buf);
  mac_stats_UEStats_vec_t uestats_vec = mac_stats_IndicationMessage_ueStats(im);
  const size_t uestats_vec_len = mac_stats_UEStats_vec_len(uestats_vec);
  assert(uestats_vec_len == num_verif);
  num_verif++;
  for (size_t i = 0; i < uestats_vec_len; ++i) {
    mac_stats_UEStats_table_t ue_stats = mac_stats_UEStats_vec_at(uestats_vec, i);
    /* check in order of how we stored it */
    assert(mac_stats_UEStats_dlAggrPrb(ue_stats) == tmp_verif++);
    assert(mac_stats_UEStats_ulAggrPrb(ue_stats) == tmp_verif++);
    assert(mac_stats_UEStats_dlAggrTbs(ue_stats) == tmp_verif++);
    assert(mac_stats_UEStats_ulAggrTbs(ue_stats) == tmp_verif++);
    assert(mac_stats_UEStats_dlAggrSdus(ue_stats) == tmp_verif++);
    assert(mac_stats_UEStats_ulAggrSdus(ue_stats) == tmp_verif++);
    assert(mac_stats_UEStats_dlAggrBytesSdus(ue_stats) == tmp_verif++);
    assert(mac_stats_UEStats_ulAggrBytesSdus(ue_stats) == tmp_verif++);
    assert(mac_stats_UEStats_dlAggrRetxPrb(ue_stats) == tmp_verif++);
    assert(mac_stats_UEStats_ulAggrRetxPrb(ue_stats) == tmp_verif++);
    assert(mac_stats_UEStats_puschSnr(ue_stats) == (float) tmp_verif++);
    assert(mac_stats_UEStats_pucchSnr(ue_stats) == (float) tmp_verif++);
    mac_stats_bsrStat_vec_t bsrstat_vec = mac_stats_UEStats_bsr(ue_stats);
    const size_t bsrstat_vec_len = mac_stats_bsrStat_vec_len(bsrstat_vec);
    const size_t numBsr = tmp_verif++ % 7;
    assert(numBsr == bsrstat_vec_len);
    for (size_t j = 0; j < numBsr; ++j) {
      mac_stats_bsrStat_struct_t bsrstat = mac_stats_bsrStat_vec_at(bsrstat_vec, j);
      assert(bsrstat->lcgid == (uint8_t) tmp_verif++);
      assert(bsrstat->bufferSize == tmp_verif++);
    }
    flatbuffers_uint32_vec_t dlHarq = mac_stats_UEStats_dlHarq(ue_stats);
    const size_t dlHarq_len = flatbuffers_uint32_vec_len(dlHarq);
    const size_t numDlHarq = tmp_verif++ % 7;
    assert(numDlHarq == dlHarq_len);
    for (size_t j = 0; j < numDlHarq; ++j)
      assert(dlHarq[j] == tmp_verif++);
    flatbuffers_uint32_vec_t ulHarq = mac_stats_UEStats_ulHarq(ue_stats);
    const size_t ulHarq_len = flatbuffers_uint32_vec_len(ulHarq);
    const size_t numUlHarq = tmp_verif++ % 7;
    assert(numUlHarq == ulHarq_len);
    for (size_t j = 0; j < numUlHarq; ++j)
      assert(ulHarq[j] == tmp_verif++);
    assert(mac_stats_UEStats_rnti(ue_stats) == (uint16_t) tmp_verif++);
    assert(mac_stats_UEStats_wbCqi(ue_stats) == (uint8_t) tmp_verif++);
    assert(mac_stats_UEStats_dlMcs1(ue_stats) == (uint8_t) tmp_verif++);
    assert(mac_stats_UEStats_ulMcs1(ue_stats) == (uint8_t) tmp_verif++);
    assert(mac_stats_UEStats_dlMcs2(ue_stats) == (uint8_t) tmp_verif++);
    assert(mac_stats_UEStats_ulMcs2(ue_stats) == (uint8_t) tmp_verif++);
    assert(mac_stats_UEStats_phr(ue_stats) == (int8_t) tmp_verif++);
  }
  free(ba.buf);
}

void test_ran_function()
{
  const size_t num = 2;
  mac_stats_report_style_t rs[num];
  rs[0].type = MAC_STATS_REPORT_STYLE_TYPE_MINIMAL;
  rs[1].type = MAC_STATS_REPORT_STYLE_TYPE_COMPLETE;
  byte_array_t ba = mac_stats_encode_ran_function(rs, num);
  assert(ba.buf && ba.len > 0);
  mac_stats_report_style_t* ret;
  const size_t ret_num = mac_stats_decode_ran_function(ba, &ret);
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
