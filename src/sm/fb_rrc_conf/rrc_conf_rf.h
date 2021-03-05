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

#ifndef RRC_CONF_RF_H
#define RRC_CONF_RF_H

#include "type_defs.h"
#include "rrc_conf_defs.h"
#include "rrc_conf_builder.h"

static const int RRC_CONF_RF = 133;

struct e2ap_agent_s;

void sm_rrc_conf_register_ran_function(struct e2ap_agent_s* ag, rrc_conf_fill_ind_msg_cb cb, const rrc_conf_report_style_t* styles, size_t n_styles);

#endif /* RRC_CONF_RF_H */
