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

#ifndef RRC_CONF_MSG_ENC_H
#define RRC_CONF_MSG_ENC_H

#include "rrc_conf_defs.h"
#include "type_defs.h"

byte_array_t rrc_conf_encode_event_trigger(rrc_conf_report_occasion_e occ);

byte_array_t rrc_conf_encode_action_definition(rrc_conf_report_style_t conf);

// encodes an empty indication header (E2AP enforces an indication header)
byte_array_t rrc_conf_encode_indication_header();

byte_array_t rrc_conf_encode_indication_message(rrc_conf_fill_ind_msg_cb cb, const rrc_conf_report_style_t* style);

byte_array_t rrc_conf_encode_ran_function(const rrc_conf_report_style_t* styles, size_t n);

#endif /* RRC_CONF_MSG_ENC_H */
