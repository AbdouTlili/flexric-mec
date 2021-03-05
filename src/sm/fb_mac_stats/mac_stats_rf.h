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

#ifndef MAC_STATS_RF_H
#define MAC_STATS_RF_H

#include "type_defs.h"
#include "mac_stats_defs.h"
#include "mac_stats_builder.h"

static const int MAC_STATS_RF = 121;

struct e2ap_agent_s;

typedef struct mac_stats_callbacks_s {
  mac_stats_fill_ind_hdr_cb hdr;
  mac_stats_fill_ind_msg_cb msg;
} mac_stats_callbacks_t;

void sm_mac_stats_register_ran_function(struct e2ap_agent_s* ag, mac_stats_callbacks_t cb, const mac_stats_report_style_t* styles, size_t n_styles);

#endif /* MAC_STATS_RF_H */
