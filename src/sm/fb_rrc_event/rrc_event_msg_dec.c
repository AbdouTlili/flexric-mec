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

#include "rrc_event_msg_dec.h"

#include "rrc_event_reader.h"
#include "rrc_event_verifier.h"

rrc_event_ReportOccasion_enum_t rrc_event_decode_event_trigger(byte_array_t b)
{
  assert(b.buf && b.len > 0);
  int ret;
  if ((ret = rrc_event_EventTrigger_verify_as_root(b.buf, b.len))) {
    printf("EventTrigger is invalid: %s\n", flatcc_verify_error_string(ret));
    abort();
  }
  rrc_event_EventTrigger_table_t et = rrc_event_EventTrigger_as_root(b.buf);
  return rrc_event_EventTrigger_reportOccasion(et);
}

rrc_event_report_style_t rrc_event_decode_action_definition(byte_array_t b)
{
  assert(b.buf && b.len > 0);
  int ret;
  if ((ret = rrc_event_ActionDefinition_verify_as_root(b.buf, b.len))) {
    printf("ActionDefinition is invalid: %s\n", flatcc_verify_error_string(ret));
    abort();
  }
  rrc_event_ActionDefinition_table_t ad = rrc_event_ActionDefinition_as_root(b.buf);
  rrc_event_ReportStyle_table_t rs = rrc_event_ActionDefinition_reportStyle(ad);
  return (rrc_event_report_style_t) { .type = rrc_event_ReportStyle_type(rs) };
}

rrc_event_indication_header_t rrc_event_decode_indication_header(byte_array_t b)
{
  assert(b.buf && b.len > 0);
  int ret;
  if ((ret = rrc_event_IndicationHeader_verify_as_root(b.buf, b.len))) {
    printf("IndicationHeader is invalid: %s\n", flatcc_verify_error_string(ret));
    abort();
  }
  rrc_event_IndicationHeader_table_t ih = rrc_event_IndicationHeader_as_root(b.buf);
  rrc_event_indication_header_t res = {
    .rnti = rrc_event_IndicationHeader_rnti(ih),
    .event = rrc_event_IndicationHeader_event(ih)
  };
  return res;
}

size_t rrc_event_decode_ran_function(byte_array_t b, rrc_event_report_style_t** styles)
{
  assert(b.buf && b.len > 0);
  int ret;
  if ((ret = rrc_event_RanFunctionDefinition_verify_as_root(b.buf, b.len))) {
    printf("RanFunctionDefinition is invalid: %s\n", flatcc_verify_error_string(ret));
    abort();
  }
  rrc_event_RanFunctionDefinition_table_t rfd = rrc_event_RanFunctionDefinition_as_root(b.buf);
  rrc_event_ReportStyle_vec_t rs_vec = rrc_event_RanFunctionDefinition_supportedReportStyles(rfd);
  const size_t rs_vec_len = rrc_event_ReportStyle_vec_len(rs_vec);
  rrc_event_report_style_t* s = malloc(rs_vec_len * sizeof(*s));
  assert(s);
  for (size_t i = 0; i < rs_vec_len; ++i) {
    rrc_event_ReportStyle_table_t rs = rrc_event_ReportStyle_vec_at(rs_vec, i);
    rrc_event_report_style_t* style = &s[i];
    style->type = rrc_event_ReportStyle_type(rs);
  }
  *styles = s;
  return rs_vec_len;
}
