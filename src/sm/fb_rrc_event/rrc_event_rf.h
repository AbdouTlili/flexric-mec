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

#ifndef RRC_EVENT_RF_H
#define RRC_EVENT_RF_H

#include "type_defs.h"
#include "rrc_event_defs.h"
#include "rrc_event_builder.h"

static const int RRC_EVENT_RF = 138;

struct e2ap_agent_s;

void sm_rrc_event_register_ran_function(struct e2ap_agent_s* ag, const rrc_event_report_style_t* styles, size_t n_styles);

void sm_rrc_event_trigger(struct e2ap_agent_s* ag, uint16_t rnti, rrc_event_Event_enum_t event, rrc_event_fill_ind_msg_cb cb, void* ctxt);

#endif /* RRC_EVENT_RF_H */
