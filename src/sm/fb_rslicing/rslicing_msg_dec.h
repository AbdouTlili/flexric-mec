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

#ifndef RSLICING_MSG_DEC_H
#define RSLICING_MSG_DEC_H

#include "rslicing_defs.h"
#include "type_defs.h"

mac_rslicing_ReportOccasion_enum_t mac_rslicing_decode_event_trigger(byte_array_t b);

// there is no action definition

// there is no (meaningful) indication header

// indication message: for now decode only through JSON

// there is no (meaningful) control header

// control command: for now decode only through JSON

char* mac_rslicing_decode_control_outcome(byte_array_t b);

uint32_t mac_rslicing_decode_call_process_id(byte_array_t b);

size_t mac_rslicing_decode_ran_function(byte_array_t b, mac_rslicing_SliceAlgorithm_enum_t** algos);

#endif /* RSLICING_MSG_DEC_H */
