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

#include "mac_stats_msg_enc.h"

#include "mac_stats_builder.h"

byte_array_t mac_stats_encode_event_trigger(uint16_t ms)
{
  flatcc_builder_t builder;
  flatcc_builder_t* B = &builder;
  flatcc_builder_init(B);

  mac_stats_EventTrigger_create_as_root(B, ms);

  size_t size;
  uint8_t *buf = flatcc_builder_finalize_buffer(B, &size);
  byte_array_t ba = { .buf = buf, .len = size };

  flatcc_builder_clear(B);
  return ba;
}

byte_array_t mac_stats_encode_action_definition(mac_stats_report_style_t style)
{
  flatcc_builder_t builder;
  flatcc_builder_t* B = &builder;
  flatcc_builder_init(B);

  mac_stats_ReportStyle_ref_t rs = mac_stats_ReportStyle_create(B, style.type);
  mac_stats_ActionDefinition_create_as_root(B, rs);

  size_t size;
  uint8_t *buf = flatcc_builder_finalize_buffer(B, &size);
  byte_array_t ba = { .buf = buf, .len = size };

  flatcc_builder_clear(B);
  return ba;
}

byte_array_t mac_stats_encode_indication_header(mac_stats_fill_ind_hdr_cb cb)
{
  flatcc_builder_t builder;
  flatcc_builder_t* B = &builder;
  flatcc_builder_init(B);

  cb(B);

  size_t size;
  uint8_t *buf = flatcc_builder_finalize_buffer(B, &size);
  byte_array_t ba = { .buf = buf, .len = size };

  flatcc_builder_clear(B);
  return ba;
}

byte_array_t mac_stats_encode_indication_message(mac_stats_fill_ind_msg_cb cb, const mac_stats_report_style_t* style)
{
  flatcc_builder_t builder;
  flatcc_builder_t* B = &builder;
  flatcc_builder_init(B);

  cb(B, style);

  size_t size;
  uint8_t *buf = flatcc_builder_finalize_buffer(B, &size);
  byte_array_t ba = { .buf = buf, .len = size };

  flatcc_builder_clear(B);
  return ba;
}

byte_array_t mac_stats_encode_ran_function(const mac_stats_report_style_t* styles, size_t n)
{
  flatcc_builder_t builder;
  flatcc_builder_t* B = &builder;
  flatcc_builder_init(B);

  mac_stats_RanFunctionDefinition_start(B);
  mac_stats_RanFunctionDefinition_supportedReportStyles_start(B);
  for (size_t i = 0; i < n; ++i)
    mac_stats_RanFunctionDefinition_supportedReportStyles_push_create(B, styles[i].type);
  mac_stats_RanFunctionDefinition_supportedReportStyles_end(B);
  mac_stats_RanFunctionDefinition_end_as_root(B);

  size_t size;
  uint8_t *buf = flatcc_builder_finalize_buffer(B, &size);
  byte_array_t ba = { .buf = buf, .len = size };

  flatcc_builder_clear(B);
  return ba;
}
