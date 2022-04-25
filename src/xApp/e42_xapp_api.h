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
  
#ifndef E42_XAPP_API_H
#define E42_XAPP_API_H 

#ifdef __cplusplus
extern "C" {
#endif


#include <stdbool.h>
#include <stdint.h>

//#include "../sm/slice_sm/ie/slice_data_ie.h"
#include "../lib/msg_hand/e2_node_arr.h"
#include "../sm/agent_if/write/sm_ag_if_wr.h"
#include "../sm/agent_if/read/sm_ag_if_rd.h"



void init_xapp_api(const char* addr);
  
bool try_stop_xapp_api(void);     

e2_node_arr_t e2_nodes_xapp_api(void);

typedef void (*sm_cb)(sm_ag_if_rd_t const*);

typedef union{
  char* reason;
  int handle;
} sm_ans_xapp_u;

typedef struct{
  sm_ans_xapp_u u;
  bool success;
} sm_ans_xapp_t;

typedef enum{
  ms_1,
  ms_2,
  ms_5,
  ms_10,


  ms_end,
} inter_xapp_e;

// returns a handle
sm_ans_xapp_t report_sm_xapp_api (global_e2_node_id_t* id, uint32_t sm_id, inter_xapp_e i, sm_cb handler);

// Remove the handle previously returned
void rm_report_sm_xapp_api(int const handle);

// Send control message
sm_ans_xapp_t control_sm_xapp_api(global_e2_node_id_t* id, uint32_t ran_func_id, sm_ag_if_wr_t const* wr);


#ifdef __cplusplus
}
#endif



#endif

