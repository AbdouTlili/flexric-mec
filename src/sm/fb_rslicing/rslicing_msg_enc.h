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

#ifndef RSLICING_MSG_ENC_H
#define RSLICING_MSG_ENC_H

#include "rslicing_defs.h"
#include "type_defs.h"

byte_array_t mac_rslicing_encode_event_trigger(mac_rslicing_ReportOccasion_enum_t occ);

byte_array_t mac_rslicing_encode_empty_indication_header();

byte_array_t mac_rslicing_encode_indication_message(rslicing_fill_ind_msg_cb cb);

byte_array_t mac_rslicing_encode_empty_control_header();

byte_array_t mac_rslicing_encode_control_outcome(const char* msg);

byte_array_t mac_rslicing_encode_call_process_id(uint32_t id);

byte_array_t mac_rslicing_encode_ran_function(const mac_rslicing_SliceAlgorithm_enum_t* algos, size_t n);

#endif /* RSLICING_MSG_ENC_H */
