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
#include "rslicing_msg_enc.h"

#include "rslicing_builder.h"
#include "rslicing_verifier.h"

byte_array_t mac_rslicing_encode_event_trigger(mac_rslicing_ReportOccasion_enum_t occ)
{
  flatcc_builder_t builder;
  flatcc_builder_t* B = &builder;
  flatcc_builder_init(B);

  mac_rslicing_EventTrigger_create_as_root(B, occ);

  size_t size;
  uint8_t *buf = flatcc_builder_finalize_buffer(B, &size);
  byte_array_t ba = { .buf = buf, .len = size };

  flatcc_builder_clear(B);
  return ba;
}

byte_array_t mac_rslicing_encode_empty_indication_header()
{
  flatcc_builder_t builder;
  flatcc_builder_t* B = &builder;
  flatcc_builder_init(B);

  mac_rslicing_IndicationHeader_create_as_root(B);

  size_t size;
  uint8_t *buf = flatcc_builder_finalize_buffer(B, &size);
  byte_array_t ba = { .buf = buf, .len = size };

  flatcc_builder_clear(B);
  return ba;
}

byte_array_t mac_rslicing_encode_indication_message(rslicing_fill_ind_msg_cb cb)
{
  flatcc_builder_t builder;
  flatcc_builder_t* B = &builder;
  flatcc_builder_init(B);

  cb(B);

  size_t size;
  uint8_t *buf = flatcc_builder_finalize_buffer(B, &size);
  byte_array_t ba = { .buf = buf, .len = size };

  int ret;
  if ((ret = mac_rslicing_IndicationMessage_verify_as_root(buf, size))) {
    printf("IndicationMessage is invalid: %s\n", flatcc_verify_error_string(ret));
    abort();
  }

  flatcc_builder_clear(B);
  return ba;
}

byte_array_t mac_rslicing_encode_empty_control_header()
{
  flatcc_builder_t builder;
  flatcc_builder_t* B = &builder;
  flatcc_builder_init(B);

  mac_rslicing_ControlHeader_create_as_root(B);

  size_t size;
  uint8_t *buf = flatcc_builder_finalize_buffer(B, &size);
  byte_array_t ba = { .buf = buf, .len = size };

  flatcc_builder_clear(B);
  return ba;
}

byte_array_t mac_rslicing_encode_control_outcome(const char* msg)
{
  flatcc_builder_t builder;
  flatcc_builder_t* B = &builder;
  flatcc_builder_init(B);

  flatbuffers_string_ref_t str = flatbuffers_string_create_str(B, msg);
  mac_rslicing_ControlOutcome_create_as_root(B, str);

  size_t size;
  uint8_t *buf = flatcc_builder_finalize_buffer(B, &size);
  byte_array_t ba = { .buf = buf, .len = size };

  flatcc_builder_clear(B);
  return ba;
}

byte_array_t mac_rslicing_encode_call_process_id(uint32_t id)
{
  flatcc_builder_t builder;
  flatcc_builder_t* B = &builder;
  flatcc_builder_init(B);

  mac_rslicing_CallProcessId_create_as_root(B, id);

  size_t size;
  uint8_t *buf = flatcc_builder_finalize_buffer(B, &size);
  byte_array_t ba = { .buf = buf, .len = size };

  flatcc_builder_clear(B);
  return ba;
}

byte_array_t mac_rslicing_encode_ran_function(const mac_rslicing_SliceAlgorithm_enum_t* algos, size_t n)
{
  flatcc_builder_t builder;
  flatcc_builder_t* B = &builder;
  flatcc_builder_init(B);

  mac_rslicing_RanFunctionDefinition_start(B);
  mac_rslicing_RanFunctionDefinition_supportedAlgorithms_start(B);
  for (size_t i = 0; i < n; ++i)
    mac_rslicing_RanFunctionDefinition_supportedAlgorithms_push_create(B, algos[i]);
  mac_rslicing_RanFunctionDefinition_supportedAlgorithms_end(B);
  mac_rslicing_RanFunctionDefinition_end_as_root(B);

  size_t size;
  uint8_t *buf = flatcc_builder_finalize_buffer(B, &size);
  byte_array_t ba = { .buf = buf, .len = size };

  flatcc_builder_clear(B);
  return ba;
}
