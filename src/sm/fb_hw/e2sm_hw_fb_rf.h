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

#ifndef E2SM_HW_RF_H
#define E2SM_HW_RF_H

#include <stdint.h>

#include "byte_array.h"
#include "type_defs.h"

#include "e2sm_hw_reader.h"
#include "flexric_agent.h"

void e2sm_hw_fb_register_ran_function(e2ap_agent_t* ag);


/* encoding/decoding function exposed for testing purposes */
void e2sm_hw_fb_decode_action_definition(byte_array_t b);
hw_TriggerNature_enum_t e2sm_hw_fb_decode_event_trigger_definition(byte_array_t b);
byte_array_t e2sm_hw_fb_generate_indication_message(byte_array_t message);
byte_array_t e2sm_hw_fb_generate_indication_header(uint32_t header);
byte_array_t e2sm_hw_fb_decode_control_message(byte_array_t b);
uint32_t e2sm_hw_fb_decode_control_header(byte_array_t b);

#endif /* E2SM_HW_RF_H */
