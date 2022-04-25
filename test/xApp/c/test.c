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

#include "../../../src/xApp/e42_xapp_api.h"
#include "../../../src/sm/slice_sm/slice_sm_id.h"
#include "../../../src/util/alg_ds/alg/defer.h"
#include "../../../src/util/time_now_us.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

static
void sm_cb_mac(sm_ag_if_rd_t const* rd)
{
  assert(rd != NULL);
  assert(rd->type == MAC_STATS_V0); 

  int64_t now = time_now_us();
  printf("MAC ind_msg latency = %ld \n", now - rd->mac_stats.msg.tstamp);
}

static
void sm_cb_rlc(sm_ag_if_rd_t const* rd)
{
  assert(rd != NULL);
  assert(rd->type == RLC_STATS_V0); 

  int64_t now = time_now_us();

  printf("RLC ind_msg latency = %ld \n", now - rd->rlc_stats.msg.tstamp);
}



static
sm_ag_if_wr_t create_add_slice(void)
{
  sm_ag_if_wr_t ctrl_msg = {.type = SLICE_CTRL_REQ_V0 };
  ctrl_msg.slice_req_ctrl.hdr.dummy = 2;
 
  slice_ctrl_msg_t* sl_ctrl_msg = &ctrl_msg.slice_req_ctrl.msg;
  sl_ctrl_msg->type = SLICE_CTRL_SM_V0_ADD;
  char sched_name[] = "My scheduler";
  size_t const sz = strlen(sched_name);
  sl_ctrl_msg->u.add_mod_slice.dl.len_sched_name = sz;
  sl_ctrl_msg->u.add_mod_slice.dl.sched_name = calloc(1,sz+1);
  memcpy(sl_ctrl_msg->u.add_mod_slice.dl.sched_name, sched_name, sz);

  sl_ctrl_msg->u.add_mod_slice.dl.len_slices = 1;
  sl_ctrl_msg->u.add_mod_slice.dl.slices = calloc(1, sizeof(slice_t) );
  sl_ctrl_msg->u.add_mod_slice.dl.slices[0].id = 37;
  sl_ctrl_msg->u.add_mod_slice.dl.slices[0].params.type = SLICE_ALG_SM_V0_STATIC;
  sl_ctrl_msg->u.add_mod_slice.dl.slices[0].params.u.sta.pos_high = 10;
  sl_ctrl_msg->u.add_mod_slice.dl.slices[0].params.u.sta.pos_low = 0;
 
  return ctrl_msg; 

}

static
sm_ag_if_wr_t create_assoc_slice(void)
{
  sm_ag_if_wr_t ctrl_msg = { .type = SLICE_CTRL_REQ_V0 };
  ctrl_msg.slice_req_ctrl.hdr.dummy = 2;
 
  slice_ctrl_msg_t* sl_ctrl_msg = &ctrl_msg.slice_req_ctrl.msg;
  sl_ctrl_msg->type = SLICE_CTRL_SM_V0_UE_SLICE_ASSOC;

  ue_slice_conf_t* ue_slice = &sl_ctrl_msg->u.ue_slice;

  ue_slice->len_ue_slice = 2;
  ue_slice->ues = calloc(2, sizeof(ue_slice_assoc_t));
  for(size_t i = 0; i < 2; ++i){
    ue_slice_assoc_t* assoc = &ue_slice->ues[i];
    assoc->dl_id = 42;
    assoc->ul_id = 42;
    assoc->rnti = 121;
  }
  return ctrl_msg;
}

int main()
{
  //Init the xApp
  init_xapp_api("127.0.0.1");
  sleep(1);

  e2_node_arr_t nodes = e2_nodes_xapp_api();
  defer({ free_e2_node_arr(&nodes); });

  assert(nodes.len > 0);

  printf("Connected E2 nodes = %d\n", nodes.len );

  e2_node_connected_t* n = &nodes.n[0];
  for(size_t i = 0; i < n->len_rf; ++i)
    printf("Registered ran func id = %d \n ", n->ack_rf[i].id );

  inter_xapp_e i = ms_1;
  // returns a handle
  sm_ans_xapp_t h = report_sm_xapp_api(&nodes.n[0].id, n->ack_rf[0].id, i, sm_cb_mac);
  assert(h.success == true);
  sleep(2);

  inter_xapp_e i_2 = ms_1;
  // returns a handle
  sm_ans_xapp_t h_2 = report_sm_xapp_api(&nodes.n[0].id, n->ack_rf[1].id, i_2, sm_cb_rlc);
  assert(h_2.success == true);

  inter_xapp_e i_3 = ms_1;
  // returns a handle
  sm_ans_xapp_t h_3 = report_sm_xapp_api(&nodes.n[1].id, n->ack_rf[0].id, i_3, sm_cb_mac);
  assert(h_2.success == true);
  sleep(2);

  // Control ADD slice
  sm_ag_if_wr_t ctrl_msg_add = create_add_slice();
  control_sm_xapp_api(&nodes.n[0].id, SM_SLICE_ID, &ctrl_msg_add);
  free(ctrl_msg_add.slice_req_ctrl.msg.u.add_mod_slice.dl.slices); 
  free(ctrl_msg_add.slice_req_ctrl.msg.u.add_mod_slice.dl.sched_name);

  sleep(1);

  // Control ASSOC slice
  sm_ag_if_wr_t ctrl_msg_assoc = create_assoc_slice();
  control_sm_xapp_api(&nodes.n[0].id, SM_SLICE_ID, &ctrl_msg_assoc);
  free(ctrl_msg_assoc.slice_req_ctrl.msg.u.ue_slice.ues); 

  sleep(1);

  // Remove the handle previously returned
  rm_report_sm_xapp_api(h.u.handle);

  // Remove the handle previously returned
  rm_report_sm_xapp_api(h_2.u.handle);

  // Remove the handle previously returned
  rm_report_sm_xapp_api(h_3.u.handle);

  sleep(1);

  //Stop the xApp
  while(try_stop_xapp_api() == false)
    usleep(1000);     

  printf("Test xApp run SUCCESSFULLY\n");
}


