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

#include "../../src/ric/iApp/e42_iapp_api.h"
#include "../../src/xApp/e42_xapp_api.h"
#include "../../src/util/alg_ds/alg/defer.h"
#include "../../src/sm/slice_sm/slice_sm_id.h"
#include "../../src/sm/rlc_sm/rlc_sm_id.h"
#include "../../src/sm/mac_sm/mac_sm_id.h"
#include "near_ric_emulator.h"

#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

static
void fill_mac_ran_func(ran_function_t* r)
{
  assert(r != NULL); 
  r->id = SM_MAC_ID;

  const char* str = SM_MAC_STR;
  r->def.buf = malloc(strlen(str));
  assert( r->def.buf != NULL && "memory exhausted");
  memcpy(r->def.buf, str, strlen(str) );
  r->def.len = strlen(str);
  r->oid = NULL;
  r->rev = 0;
}

static
void fill_rlc_ran_func(ran_function_t* r)
{
  assert(r != NULL); 
  r->id =  SM_RLC_ID; 
  const char* str = SM_RLC_STR; 
  r->def.buf = malloc(strlen(str));
  assert( r->def.buf != NULL && "memory exhausted");
  memcpy(r->def.buf, str, strlen(str) );
  r->def.len = strlen(str);
  r->oid = NULL;
  r->rev = 0;
}

static
void sm_cb_mac(sm_ag_if_rd_t const* rd)
{
  assert(rd != NULL);
  assert(rd->type == MAC_STATS_V0); 

  printf("MAC tstamp = %ld \n", rd->mac_stats.msg.tstamp );

}

static
void sm_cb_rlc(sm_ag_if_rd_t const* rd)
{
  assert(rd != NULL);
  assert(rd->type == RLC_STATS_V0); 

  printf("RLC tstamp = %ld \n", rd->rlc_stats.msg.tstamp );

}

int main()
{
  const char* addr = "127.0.0.1";

  near_ric_if_t ric_if = {.type = NULL};
  init_iapp_api(addr, ric_if);

  sleep(1);

  plmn_t p = { .mcc = 208,
    .mnc = 95,
    .mnc_digit_len = 2
  };

  global_e2_node_id_t node = {.type = ngran_gNB,
                              .plmn = p,
                              .nb_id = 42}; 

  size_t len = 2;
  ran_function_t ran_func[len];
  fill_mac_ran_func(&ran_func[0]);
  fill_rlc_ran_func(&ran_func[1]);

  add_e2_node_iapp_api(&node, len, ran_func);

  node.nb_id = 43;
  add_e2_node_iapp_api(&node, len, ran_func);

  init_xapp_api(addr);
  sleep(1);

  e2_node_arr_t nodes = e2_nodes_xapp_api();
  defer({  free_e2_node_arr(&nodes);  } );

  printf("Connected E2 nodes = %d\n", nodes.len );

  assert(nodes.len > 0);

  inter_xapp_e i = ms_5;

  sm_ans_xapp_t h = report_sm_xapp_api(&nodes.n[0].id, SM_MAC_ID, i, sm_cb_mac);

  assert(h.success == true);

  sleep(1);

  sm_ans_xapp_t h_rlc = report_sm_xapp_api(&nodes.n[0].id, SM_RLC_ID, i, sm_cb_rlc);

  assert(h.success == true);

  sleep(1);

  // Silence unused parameter
  const char* tmp = SM_SLICE_STR;
  (void)tmp;

  sm_ag_if_wr_t ctrl_msg = {.type = SLICE_CTRL_REQ_V0 };
  ctrl_msg.slice_req_ctrl.hdr.dummy = 2;
 
  slice_ctrl_msg_t* sl_ctrl_msg = &ctrl_msg.slice_req_ctrl.msg;
  sl_ctrl_msg->type = SLICE_CTRL_SM_V0_ADD;
  char sched_name[] = "My scheduler";
  sl_ctrl_msg->u.add_mod_slice.dl.len_sched_name = strlen(sched_name);
  sl_ctrl_msg->u.add_mod_slice.dl.sched_name = sched_name;
  sl_ctrl_msg->u.add_mod_slice.dl.len_slices = 1;
  sl_ctrl_msg->u.add_mod_slice.dl.slices = calloc(1, sizeof(fr_slice_t) );
  sl_ctrl_msg->u.add_mod_slice.dl.slices[0].id = 37;
  sl_ctrl_msg->u.add_mod_slice.dl.slices[0].params.type = SLICE_ALG_SM_V0_STATIC;
  sl_ctrl_msg->u.add_mod_slice.dl.slices[0].params.u.sta.pos_high = 10;
  sl_ctrl_msg->u.add_mod_slice.dl.slices[0].params.u.sta.pos_low = 0;
  defer({ free(sl_ctrl_msg->u.add_mod_slice.dl.slices); });

  control_sm_xapp_api(&nodes.n[0].id, SM_SLICE_ID, &ctrl_msg);

  sleep(3);

  rm_report_sm_xapp_api(h.u.handle);

  sleep(1);

  rm_report_sm_xapp_api(h_rlc.u.handle);

  sleep(1);

  stop_iapp_api();     

  while(try_stop_xapp_api() == 0)
    usleep(1000);

  for(size_t i = 0; i < len; ++i){
    free_ran_function(&ran_func[i]);
  }

  return EXIT_SUCCESS;
}

