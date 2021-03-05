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

#ifndef RSLICING_DEFS_H
#define RSLICING_DEFS_H

#include <stdint.h>
#include <stdbool.h>
#include "rslicing_reader.h"

// Fake OID so the controller recognizes the event
// iso(1) identified-organization(3) dod(6) internet(1) private(4) enterprise(1) 53148 e2(1) version1 (1) e2sm_fb(99) rslicing(128)  // last two would be: e2sm(2) e2sm-KPM-IEs (2)
__attribute__((unused)) static const char* rslicing_oid = "1.3.6.1.4.1.1.1.99.128";

struct flatcc_builder;
typedef void (*rslicing_fill_ind_msg_cb)(struct flatcc_builder*);

typedef struct rslicing_rc_s {
  bool success;
  char* error_msg;
} rslicing_rc_t;

typedef rslicing_rc_t (*rslicing_add_mod_slice_ctrl_cb)(mac_rslicing_AddModSliceCommand_table_t);
typedef rslicing_rc_t (*rslicing_del_slice_ctrl_cb)(mac_rslicing_DelSliceCommand_table_t);
typedef rslicing_rc_t (*rslicing_ue_assoc_ctrl_cb)(mac_rslicing_UeSliceAssocCommand_table_t);

#endif /* RSLICING_DEFS_H */
