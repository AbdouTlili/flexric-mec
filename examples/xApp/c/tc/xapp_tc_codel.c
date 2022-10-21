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

#include "../../../../src/xApp/e42_xapp_api.h"
#include "../../../../src/sm/slice_sm/slice_sm_id.h"
#include "../../../../src/util/alg_ds/alg/defer.h"
#include "../../../../src/util/time_now_us.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <poll.h>
#include <unistd.h>

#include <arpa/inet.h>


static
tc_ctrl_msg_t gen_mod_bdp_pcr(void)
{
     	tc_ctrl_msg_t ans = {.type = TC_CTRL_SM_V0_PCR,  
		.pcr.act = TC_CTRL_ACTION_SM_V0_MOD};  

       	ans.pcr.mod.type = TC_PCR_5G_BDP; 
      	ans.pcr.mod.bdp.tstamp = time_now_us(); 
      	ans.pcr.mod.bdp.drb_sz = 25000; 
      	return ans;
}

static
tc_ctrl_msg_t gen_add_codel_queue(void)
{

  tc_ctrl_msg_t ans = {.type = TC_CTRL_SM_V0_QUEUE,
                       .q.act = TC_CTRL_ACTION_SM_V0_ADD}; 

  tc_add_ctrl_queue_t* q = &ans.q.add;

  q->type = TC_QUEUE_CODEL;

  q->codel.interval_ms = 100;
  q->codel.target_ms = 5;

  assert(ans.type ==  TC_CTRL_SM_V0_QUEUE );

  return ans;
}

static
tc_ctrl_msg_t gen_add_osi_cls(void) 
{
  tc_ctrl_msg_t ans = {.type = TC_CTRL_SM_V0_CLS,
		.cls.act = TC_CTRL_ACTION_SM_V0_ADD}; 

	tc_add_ctrl_cls_t* add = &ans.cls.add; 
	add->type = TC_CLS_OSI;  
	add->osi.dst_queue = 1;
	add->osi.l3.src_addr = -1;  
	add->osi.l3.dst_addr = -1;
	add->osi.l4.src_port = -1; 
	add->osi.l4.dst_port = -1;
	add->osi.l4.protocol = -1;
	return ans;
}

int main(int argc, char* argv[])
{
  fr_args_t args = init_fr_args(argc, argv);
  //Init the xApp
  init_xapp_api(&args);
  sleep(1);

  e2_node_arr_t nodes = e2_nodes_xapp_api();
  defer({ free_e2_node_arr(&nodes); });

  assert(nodes.len > 0);

  printf("Connected E2 nodes = %d\n", nodes.len );

  e2_node_connected_t* n = &nodes.n[0];
  for(size_t i = 0; i < n->len_rf; ++i)
    printf("Registered ran func id = %d \n ", n->ack_rf[i].id );

  uint32_t TC_SM_ID_TEST = 146;
  // Pacer
  sm_ag_if_wr_t wr = {.type = TC_CTRL_REQ_V0 };
  wr.tc_req_ctrl.msg = gen_mod_bdp_pcr();
  control_sm_xapp_api(&nodes.n[0].id, TC_SM_ID_TEST, &wr);
  free_tc_ctrl_msg(&wr.tc_req_ctrl.msg);
  poll(NULL, 0, 10);

  // Queues
  sm_ag_if_wr_t wr_q = {.type = TC_CTRL_REQ_V0 };
  wr_q.tc_req_ctrl.msg = gen_add_codel_queue();
  control_sm_xapp_api(&nodes.n[0].id, TC_SM_ID_TEST, &wr_q);
  free_tc_ctrl_msg(&wr_q.tc_req_ctrl.msg);
  poll(NULL, 0, 10);

 // Rule to segregate on the classifier
 sm_ag_if_wr_t wr_cls = {.type = TC_CTRL_REQ_V0 };
 wr_cls.tc_req_ctrl.msg = gen_add_osi_cls();
 control_sm_xapp_api(&nodes.n[0].id, TC_SM_ID_TEST, &wr_cls);
 free_tc_ctrl_msg(&wr_cls.tc_req_ctrl.msg);
 poll(NULL, 0, 10);

  //Stop the xApp
  while(try_stop_xapp_api() == false)
    poll(NULL, 0, 1000);

  printf("Test xApp run SUCCESSFULLY\n");
}

