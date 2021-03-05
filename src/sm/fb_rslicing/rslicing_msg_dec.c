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

#include "rslicing_msg_dec.h"

#include "rslicing_reader.h"
#include "rslicing_verifier.h"

mac_rslicing_ReportOccasion_enum_t mac_rslicing_decode_event_trigger(byte_array_t b)
{
  assert(b.buf && b.len > 0);
  int ret;
  if ((ret = mac_rslicing_EventTrigger_verify_as_root(b.buf, b.len))) {
    printf("EventTrigger is invalid: %s\n", flatcc_verify_error_string(ret));
    abort();
  }
  mac_rslicing_EventTrigger_table_t et = mac_rslicing_EventTrigger_as_root(b.buf);
  return mac_rslicing_EventTrigger_reportOccasion(et);
}

char* mac_rslicing_decode_control_outcome(byte_array_t b)
{
  assert(b.buf && b.len > 0);
  int ret;
  if ((ret = mac_rslicing_ControlOutcome_verify_as_root(b.buf, b.len))) {
    printf("ControlOutcome is invalid: %s\n", flatcc_verify_error_string(ret));
    abort();
  }
  mac_rslicing_ControlOutcome_table_t co = mac_rslicing_ControlOutcome_as_root(b.buf);
  const char* s = mac_rslicing_ControlOutcome_diagnostic(co);
  return strdup(s);
}

uint32_t mac_rslicing_decode_call_process_id(byte_array_t b)
{
  assert(b.buf && b.len > 0);
  int ret;
  if ((ret = mac_rslicing_CallProcessId_verify_as_root(b.buf, b.len))) {
    printf("CallProcessId is invalid: %s\n", flatcc_verify_error_string(ret));
    abort();
  }
  mac_rslicing_CallProcessId_table_t cpi = mac_rslicing_CallProcessId_as_root(b.buf);
  return mac_rslicing_CallProcessId_id(cpi);
}

size_t mac_rslicing_decode_ran_function(byte_array_t b, mac_rslicing_SliceAlgorithm_enum_t** algos)
{
  assert(b.buf && b.len > 0);
  int ret;
  if ((ret = mac_rslicing_RanFunctionDefinition_verify_as_root(b.buf, b.len))) {
    printf("RanFunctionDefinition is invalid: %s\n", flatcc_verify_error_string(ret));
    abort();
  }
  mac_rslicing_RanFunctionDefinition_table_t rfd = mac_rslicing_RanFunctionDefinition_as_root(b.buf);
  mac_rslicing_SupportedAlgorithm_vec_t sa_vec = mac_rslicing_RanFunctionDefinition_supportedAlgorithms(rfd);
  const size_t sa_vec_len = mac_rslicing_SupportedAlgorithm_vec_len(sa_vec);
  mac_rslicing_SliceAlgorithm_enum_t* sa = malloc(sa_vec_len * sizeof(*sa));
  assert(sa);
  for (size_t i = 0; i < sa_vec_len; ++i) {
    mac_rslicing_SupportedAlgorithm_table_t sat = mac_rslicing_SupportedAlgorithm_vec_at(sa_vec, i);
    sa[i] = mac_rslicing_SupportedAlgorithm_algorithm(sat);
  }
  *algos = sa;
  return sa_vec_len;
}
