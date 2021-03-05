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

#ifndef MAC_STATS_MSG_ENC_H
#define MAC_STATS_MSG_ENC_H

#include "mac_stats_defs.h"
#include "type_defs.h"

byte_array_t mac_stats_encode_event_trigger(uint16_t ms);

byte_array_t mac_stats_encode_action_definition(mac_stats_report_style_t stats);

byte_array_t mac_stats_encode_indication_header(mac_stats_fill_ind_hdr_cb cb);

byte_array_t mac_stats_encode_indication_message(mac_stats_fill_ind_msg_cb cb, const mac_stats_report_style_t* style);

byte_array_t mac_stats_encode_ran_function(const mac_stats_report_style_t* styles, size_t n);

#endif /* MAC_STATS_MSG_ENC_H */
