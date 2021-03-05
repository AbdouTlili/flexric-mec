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

#ifndef PDCP_STATS_RF_H
#define PDCP_STATS_RF_H

#include "type_defs.h"
#include "pdcp_stats_defs.h"
#include "pdcp_stats_builder.h"

static const int PDCP_STATS_RF = 123;

struct e2ap_agent_s;

typedef struct pdcp_stats_callbacks_s {
  pdcp_stats_fill_ind_hdr_cb hdr;
  pdcp_stats_fill_ind_msg_cb msg;
} pdcp_stats_callbacks_t;

void sm_pdcp_stats_register_ran_function(struct e2ap_agent_s* ag, pdcp_stats_callbacks_t cb, const pdcp_stats_report_style_t* styles, size_t n_styles);

#endif /* PDCP_STATS_RF_H */
