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

#include "../../src/lib/ap/e2ap_types/ric_subscription_request.h"
#include "../../src/lib/ap/e2ap_types/ric_subscription_delete_request.h"
#include "../../src/lib/ap/e2ap_types/ric_control_request.h"
#include "../../src/lib/ap/type_defs.h"
#include "../../src/ric/iApp/iapp_if_generic.h"

#include <stdint.h>

#ifndef NEAR_RIC_EMULATOR_H
#define NEAR_RIC_EMULATOR_H 

void start_near_ric_iapp_emulator( near_ric_if_emulator_t* ric); 

void stop_near_ric_iapp_emulator();

uint16_t fwd_ric_subscription_request_emulator(near_ric_if_emulator_t* ric, ric_subscription_request_t const* sr,  void (*f)(e2ap_msg_t const* msg) );

void fwd_ric_subscription_request_delete_emulator(near_ric_if_emulator_t* ric, ric_subscription_delete_request_t const* sr,  void (*f)(e2ap_msg_t const* msg) );

uint16_t fwd_ric_control_request_emulator(near_ric_if_emulator_t* ric, ric_control_request_t const* cr,  void (*f)(e2ap_msg_t const* msg) );

#endif

