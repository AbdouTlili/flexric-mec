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

#ifndef RSLICING_RF_H
#define RSLICING_RF_H

#include "type_defs.h"
#include "rslicing_defs.h"
#include "rslicing_builder.h"

static const int RSLICING_RF = 128;

typedef struct rslicing_cb_s {
  rslicing_fill_ind_msg_cb read;
  rslicing_add_mod_slice_ctrl_cb add_mod;
  rslicing_del_slice_ctrl_cb del;
  rslicing_ue_assoc_ctrl_cb ue_assoc;
} rslicing_cb_t;

struct e2ap_agent_s;
void sm_mac_rslicing_register_ran_function(struct e2ap_agent_s* ag, const mac_rslicing_SliceAlgorithm_enum_t* algos, size_t n, rslicing_cb_t cb);

#endif /* RSLICING_RF_H */
