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

#include "pdcp_stats_msg_dec.h"

#include "pdcp_stats_reader.h"
#include "pdcp_stats_verifier.h"

uint16_t pdcp_stats_decode_event_trigger(byte_array_t b)
{
  assert(b.buf && b.len > 0);
  int ret;
  if ((ret = pdcp_stats_EventTrigger_verify_as_root(b.buf, b.len))) {
    printf("EventTrigger is invalid: %s\n", flatcc_verify_error_string(ret));
    abort();
  }
  pdcp_stats_EventTrigger_table_t et = pdcp_stats_EventTrigger_as_root(b.buf);
  return pdcp_stats_EventTrigger_msReportPeriod(et);
}

pdcp_stats_report_style_t pdcp_stats_decode_action_definition(byte_array_t b)
{
  assert(b.buf && b.len > 0);
  int ret;
  if ((ret = pdcp_stats_ActionDefinition_verify_as_root(b.buf, b.len))) {
    printf("ActionDefinition is invalid: %s\n", flatcc_verify_error_string(ret));
    abort();
  }
  pdcp_stats_ActionDefinition_table_t ad = pdcp_stats_ActionDefinition_as_root(b.buf);
  pdcp_stats_ReportStyle_table_t rs = pdcp_stats_ActionDefinition_reportStyle(ad);
  return (pdcp_stats_report_style_t) { .type = pdcp_stats_ReportStyle_type(rs) };
}

size_t pdcp_stats_decode_ran_function(byte_array_t b, pdcp_stats_report_style_t** styles)
{
  assert(b.buf && b.len > 0);
  int ret;
  if ((ret = pdcp_stats_RanFunctionDefinition_verify_as_root(b.buf, b.len))) {
    printf("RanFunctionDefinition is invalid: %s\n", flatcc_verify_error_string(ret));
    abort();
  }
  pdcp_stats_RanFunctionDefinition_table_t rfd = pdcp_stats_RanFunctionDefinition_as_root(b.buf);
  pdcp_stats_ReportStyle_vec_t rs_vec = pdcp_stats_RanFunctionDefinition_supportedReportStyles(rfd);
  const size_t rs_vec_len = pdcp_stats_ReportStyle_vec_len(rs_vec);
  pdcp_stats_report_style_t* s = malloc(rs_vec_len * sizeof(*s));
  assert(s);
  for (size_t i = 0; i < rs_vec_len; ++i) {
    pdcp_stats_ReportStyle_table_t rs = pdcp_stats_ReportStyle_vec_at(rs_vec, i);
    pdcp_stats_report_style_t* style = &s[i];
    style->type = pdcp_stats_ReportStyle_type(rs);
  }
  *styles = s;
  return rs_vec_len;
}
