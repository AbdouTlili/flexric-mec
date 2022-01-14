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



#include "../src/agent/e2_agent_api.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>



void read_RAN(sm_ag_if_rd_t* rd_if)
{

  assert(rd_if->type == MAC_STATS_V0 || rd_if->type == RLC_STATS_V0);

  srand(time(0));


  if(rd_if->type == MAC_STATS_V0 ){
    mac_ind_msg_t* ind = &rd_if->mac_stats.msg;
    ind->ue_stats = calloc(5, sizeof(mac_ue_stats_impl_t));
    ind->len_ue_stats = 5;
    assert(ind->ue_stats != NULL);
    for(int i =0 ; i < 5; ++i){
      ind->ue_stats[i].dl_aggr_tbs = rand()%5000;
      ind->ue_stats[i].ul_aggr_tbs  = rand()%5000;
      ind->ue_stats[i].dl_aggr_bytes_sdus  = rand()%5000;
      ind->ue_stats[i].ul_aggr_bytes_sdus   = rand()%5000;
      ind->ue_stats[i].pusch_snr  = rand()%5000;
      ind->ue_stats[i].pucch_snr  = rand()%5000;
      ind->ue_stats[i].rnti  = rand()%5000;
      ind->ue_stats[i].dl_aggr_prb   = rand()%5000;
      ind->ue_stats[i].ul_aggr_prb  = rand()%5000;
      ind->ue_stats[i].dl_aggr_sdus  = rand()%5000;
      ind->ue_stats[i].ul_aggr_sdus  = rand()%5000;
      ind->ue_stats[i].dl_aggr_retx_prb   = rand()%5000;
      ind->ue_stats[i].wb_cqi = rand()%5000;
      ind->ue_stats[i].dl_mcs1 = rand()%5000;
      ind->ue_stats[i].ul_mcs1 = rand()%5000;
      ind->ue_stats[i].dl_mcs2 = rand()%5000;
      ind->ue_stats[i].ul_mcs2 = rand()%5000;
      ind->ue_stats[i].phr  = rand()%5000;
    }

  } else if(rd_if->type == RLC_STATS_V0) {
      rd_if->rlc_stats.msg.tstamp = 1123456789 ;
      rd_if->rlc_stats.msg.len = 5;
      rd_if->rlc_stats.msg.rb = calloc(5, sizeof(rlc_radio_bearer_stats_t));

     rlc_ind_msg_t* rlc = &rd_if->rlc_stats.msg;
     for(int i =0 ; i < 5; ++i){
       rlc->rb[i].txpdu_pkts= rand()%5000;
       rlc->rb[i].txpdu_bytes= rand()%5000;
       rlc->rb[i].txpdu_wt_ms= rand()%5000;
       rlc->rb[i].txpdu_dd_pkts= rand()%5000;
       rlc->rb[i].txpdu_dd_bytes= rand()%5000;
       rlc->rb[i].txpdu_retx_pkts= rand()%5000;
       rlc->rb[i].txpdu_retx_bytes= rand()%5000;
       rlc->rb[i].txpdu_segmented= rand()%5000;
       rlc->rb[i].txpdu_status_pkts= rand()%5000;
       rlc->rb[i].txpdu_status_bytes= rand()%5000;
       rlc->rb[i].txbuf_occ_bytes= rand()%5000;
       rlc->rb[i].txbuf_occ_pkts= rand()%5000;
       rlc->rb[i].rxpdu_pkts= rand()%5000;
       rlc->rb[i].rxpdu_bytes= rand()%5000;
       rlc->rb[i].rxpdu_dup_pkts= rand()%5000;
       rlc->rb[i].rxpdu_dup_bytes= rand()%5000;
       rlc->rb[i].rxpdu_dd_pkts= rand()%5000;
       rlc->rb[i].rxpdu_dd_bytes= rand()%5000;
       rlc->rb[i].rxpdu_ow_pkts= rand()%5000;
       rlc->rb[i].rxpdu_ow_bytes= rand()%5000;
       rlc->rb[i].rxpdu_status_pkts= rand()%5000;
       rlc->rb[i].rxpdu_status_bytes= rand()%5000;
       rlc->rb[i].rxbuf_occ_bytes= rand()%5000;
       rlc->rb[i].rxbuf_occ_pkts= rand()%5000;
       rlc->rb[i].txsdu_pkts= rand()%5000;
       rlc->rb[i].txsdu_bytes= rand()%5000;
       rlc->rb[i].rxsdu_pkts= rand()%5000;
       rlc->rb[i].rxsdu_dd_pkts= rand()%5000;
       rlc->rb[i].rxsdu_dd_bytes= rand()%5000;
       rlc->rb[i].rnti= rand()%5000;
       rlc->rb[i].mode= rand()%5000;
       rlc->rb[i].rbid= rand()%5000;
     }
  }else {
    assert("Invalid data type");
  }

}

sm_ag_if_ans_t write_RAN(sm_ag_if_wr_t const* data)
{
  assert(data != NULL);
  if(data->type == MAC_CTRL_REQ_V0){
    printf("Control message called in the RAN \n");
    sm_ag_if_ans_t ans = {.type = MAC_AGENT_IF_CTRL_ANS_V0 }; 
    ans.mac.ans = MAC_CTRL_OUT_OK;
    return ans;
  } else {
    assert(0 != 0 && "Not supported function ");
  }
  sm_ag_if_ans_t ans = {0};
  return ans;
}

int main()
{
  // Init the Agent
  const int mcc = 10; 
  const int mnc = 15; 
  const int mnc_digit_len = 2;
  const int nb_id = 42;
  sm_io_ag_t io = {.read = read_RAN, .write = write_RAN};
 
  init_agent_api(mcc, mnc, mnc_digit_len, nb_id, io );

  sleep(200);

  // Stop the Agent
  stop_agent_api();
}
