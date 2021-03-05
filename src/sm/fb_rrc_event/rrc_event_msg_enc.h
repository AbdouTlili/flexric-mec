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

#ifndef RRC_EVENT_MSG_ENC_H
#define RRC_EVENT_MSG_ENC_H

#include "rrc_event_defs.h"
#include "type_defs.h"

byte_array_t rrc_event_encode_event_trigger(rrc_event_ReportOccasion_enum_t occ);

byte_array_t rrc_event_encode_action_definition(rrc_event_report_style_t event);

byte_array_t rrc_event_encode_indication_header(rrc_event_indication_header_t hdr);

byte_array_t rrc_event_encode_indication_message(rrc_event_fill_ind_msg_cb cb, const rrc_event_report_style_t* style, void* ctxt);

byte_array_t rrc_event_encode_ran_function(const rrc_event_report_style_t* styles, size_t n);

#endif /* RRC_EVENT_MSG_ENC_H */
