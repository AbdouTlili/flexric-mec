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

#include "rrc_event_msg_enc.h"

#include "rrc_event_builder.h"

byte_array_t rrc_event_encode_event_trigger(rrc_event_ReportOccasion_enum_t occ)
{
  flatcc_builder_t builder;
  flatcc_builder_t* B = &builder;
  flatcc_builder_init(B);

  rrc_event_EventTrigger_create_as_root(B, occ);

  size_t size;
  uint8_t *buf = flatcc_builder_finalize_buffer(B, &size);
  byte_array_t ba = { .buf = buf, .len = size };

  flatcc_builder_clear(B);
  return ba;
}

byte_array_t rrc_event_encode_action_definition(rrc_event_report_style_t style)
{
  flatcc_builder_t builder;
  flatcc_builder_t* B = &builder;
  flatcc_builder_init(B);

  rrc_event_ReportStyle_ref_t rs = rrc_event_ReportStyle_create(B, style.type);
  rrc_event_ActionDefinition_create_as_root(B, rs);

  size_t size;
  uint8_t *buf = flatcc_builder_finalize_buffer(B, &size);
  byte_array_t ba = { .buf = buf, .len = size };

  flatcc_builder_clear(B);
  return ba;
}

byte_array_t rrc_event_encode_indication_header(rrc_event_indication_header_t hdr)
{
  flatcc_builder_t builder;
  flatcc_builder_t* B = &builder;
  flatcc_builder_init(B);

  rrc_event_IndicationHeader_create_as_root(B, hdr.rnti, hdr.event);

  size_t size;
  uint8_t *buf = flatcc_builder_finalize_buffer(B, &size);
  byte_array_t ba = { .buf = buf, .len = size };

  flatcc_builder_clear(B);
  return ba;
}

byte_array_t rrc_event_encode_indication_message(rrc_event_fill_ind_msg_cb cb, const rrc_event_report_style_t* style, void* ctxt)
{
  flatcc_builder_t builder;
  flatcc_builder_t* B = &builder;
  flatcc_builder_init(B);

  if (cb) {
    cb(B, style, ctxt);
  } else {
    // no content: create empty message
    rrc_event_IndicationMessage_start(B);
    rrc_event_IndicationMessage_end_as_root(B);
  }

  size_t size;
  uint8_t *buf = flatcc_builder_finalize_buffer(B, &size);
  byte_array_t ba = { .buf = buf, .len = size };

  flatcc_builder_clear(B);
  return ba;
}

byte_array_t rrc_event_encode_ran_function(const rrc_event_report_style_t* styles, size_t n)
{
  flatcc_builder_t builder;
  flatcc_builder_t* B = &builder;
  flatcc_builder_init(B);

  rrc_event_RanFunctionDefinition_start(B);
  rrc_event_RanFunctionDefinition_supportedReportStyles_start(B);
  for (size_t i = 0; i < n; ++i)
    rrc_event_RanFunctionDefinition_supportedReportStyles_push_create(B, styles[i].type);
  rrc_event_RanFunctionDefinition_supportedReportStyles_end(B);
  rrc_event_RanFunctionDefinition_end_as_root(B);

  size_t size;
  uint8_t *buf = flatcc_builder_finalize_buffer(B, &size);
  byte_array_t ba = { .buf = buf, .len = size };

  flatcc_builder_clear(B);
  return ba;
}
