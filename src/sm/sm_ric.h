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


#ifndef MIR_SM_RIC_H
#define MIR_SM_RIC_H

#include <stddef.h>
#include <stdint.h>

#include "sm_alloc.h"
#include "sm_io.h"
#include "sm_proc_data.h"

typedef struct sm_ric_s sm_ric_t;

typedef struct {

  sm_subs_data_t (*on_subscription)(sm_ric_t const* ,const char* cmd);

  sm_ag_if_rd_t (*on_indication)(sm_ric_t const*, sm_ind_data_t* data);

  sm_ctrl_req_data_t (*on_control_req)(sm_ric_t const*, sm_ag_if_wr_t const*);

  sm_ag_if_ans_t (*on_control_out)(sm_ric_t const*, sm_ctrl_out_data_t const*);
 
  void (*on_e2_setup)(sm_ric_t const*, const sm_e2_setup_t*);

  sm_ric_service_update_t (*on_ric_service_update)(sm_ric_t const*, const char*);

} sm_e2ap_procedures_ric_t;

typedef struct sm_ric_s {

  // 5 Procedures stored at the SO
  sm_e2ap_procedures_ric_t proc; 

    // Free function
  void (*free_sm)(sm_ric_t* sm_ric);

  // (De)Allocation memory functions
  sm_alloc_t alloc;

  // Shared Object handle
  void* handle;

// RAN Function ID
  uint16_t const ran_func_id;

  char ran_func_name[32];

} sm_ric_t;

#endif

