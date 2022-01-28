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



#include "../../mac_sm/mac_sm_agent.h"
#include "../../mac_sm/mac_sm_ric.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>


static
mac_ind_data_t cp;

///
// Functions implemented and specific to the RAN
//

void fill_mac_ind_data(mac_ind_data_t* ind_data)
{
  srand(time(0));

  int const mod = 1024;

  mac_ind_msg_t* ind_msg = &ind_data->msg; 
  
  int const NUM_UES = abs(rand()%10);

  ind_msg->len_ue_stats = NUM_UES;

  ind_msg->tstamp = 123456789;

  if(NUM_UES > 0){
    ind_msg->ue_stats = calloc(NUM_UES, sizeof(mac_ue_stats_impl_t));
    assert(ind_msg->ue_stats != NULL && "memory exhausted");
  }

  for(uint32_t i = 0; i < ind_msg->len_ue_stats; ++i){
    ind_msg->ue_stats[i].dl_aggr_tbs = abs(rand()%mod);
    ind_msg->ue_stats[i].ul_aggr_tbs = abs(rand()%mod);
    ind_msg->ue_stats[i].dl_aggr_bytes_sdus = abs(rand()%mod);
    ind_msg->ue_stats[i].ul_aggr_bytes_sdus = abs(rand()%mod);
    ind_msg->ue_stats[i].pusch_snr = 64.0; //: float = -64;
    ind_msg->ue_stats[i].pucch_snr = 64.0; //: float = -64;
    ind_msg->ue_stats[i].rnti = abs(rand()%mod);
    ind_msg->ue_stats[i].dl_aggr_prb = abs(rand()%mod);
    ind_msg->ue_stats[i].ul_aggr_prb = abs(rand()%mod);
    ind_msg->ue_stats[i].dl_aggr_sdus = abs(rand()%mod);
    ind_msg->ue_stats[i].ul_aggr_sdus= abs(rand()%mod);
    ind_msg->ue_stats[i].dl_aggr_retx_prb= abs(rand()%mod);
    ind_msg->ue_stats[i].wb_cqi= abs(rand()%mod);
    ind_msg->ue_stats[i].dl_mcs1= abs(rand()%mod);
    ind_msg->ue_stats[i].ul_mcs1= abs(rand()%mod);
    ind_msg->ue_stats[i].dl_mcs2= abs(rand()%mod);
    ind_msg->ue_stats[i].ul_mcs2= abs(rand()%mod);
    ind_msg->ue_stats[i].phr= abs(rand()%mod);
  }

  cp.hdr = cp_mac_ind_hdr(&ind_data->hdr);
  cp.msg = cp_mac_ind_msg(&ind_data->msg);
}


/////
// AGENT
////

static

void read_RAN(sm_ag_if_rd_t* read)
{
  assert(read != NULL);
  assert(read->type == MAC_STATS_V0);

  fill_mac_ind_data(&read->mac_stats);
}

static 
sm_ag_if_ans_t write_RAN(const sm_ag_if_wr_t* data)
{
  assert(data != NULL);
  assert(0!=0 && "Not implemented");
  sm_ag_if_ans_t ans = {0};
  return ans;
}



/////////////////////////////
// Check Functions
// //////////////////////////

static
void check_eq_ran_function(sm_agent_t const* ag, sm_ric_t const* ric)
{
  assert(ag != NULL);
  assert(ric != NULL);
  assert(ag->ran_func_id == ric->ran_func_id);
}

// RIC -> E2
static
void check_subscription(sm_agent_t* ag, sm_ric_t* ric)
{
  assert(ag != NULL);
  assert(ric != NULL);
 
  sm_subs_data_t data = ric->proc.on_subscription(ric, "2_ms");
  ag->proc.on_subscription(ag, &data); 

  free_sm_subs_data(&data);
}

// E2 -> RIC
static
void check_indication(sm_agent_t* ag, sm_ric_t* ric)
{
  assert(ag != NULL);
  assert(ric != NULL);

  sm_ind_data_t sm_data = ag->proc.on_indication(ag);
  sm_ag_if_rd_t msg = ric->proc.on_indication(ric, &sm_data);

  assert(msg.type == MAC_STATS_V0);
  mac_ind_data_t* data = &msg.mac_stats;

  assert(eq_mac_ind_hdr(&data->hdr, &cp.hdr) == true);
  assert(eq_mac_ind_msg(&data->msg, &cp.msg) == true);
  assert(eq_mac_call_proc_id(data->proc_id, cp.proc_id) == true);

  free_mac_ind_hdr(&data->hdr);
  free_mac_ind_msg(&data->msg);

  free_sm_ind_data(&sm_data); 
}

int main()
{
  sm_io_ag_t io_ag = {.read = read_RAN, .write = write_RAN};  
  sm_agent_t* sm_ag = make_mac_sm_agent(io_ag);
  sm_ric_t* sm_ric = make_mac_sm_ric();

  check_eq_ran_function(sm_ag, sm_ric);
  check_subscription(sm_ag, sm_ric);
  check_indication(sm_ag, sm_ric);

  sm_ag->free_sm(sm_ag);
  sm_ric->free_sm(sm_ric);

  printf("Success\n");
  return EXIT_SUCCESS;
}

