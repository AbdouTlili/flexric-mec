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



#include "../src/ric/near_ric_api.h"
#include "../src/agent/e2_agent_api.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

static
int64_t time_now_us(void)
{
  struct timespec tms;

  /* The C11 way */
  /* if (! timespec_get(&tms, TIME_UTC))  */

  /* POSIX.1-2008 way */
  if (clock_gettime(CLOCK_REALTIME,&tms)) {
    return -1;
  }
  /* seconds, multiplied with 1 million */
  int64_t micros = tms.tv_sec * 1000000;
  /* Add full microseconds */
  micros += tms.tv_nsec/1000;
  /* round up if necessary */
  if (tms.tv_nsec % 1000 >= 500) {
    ++micros;
  }
  return micros;
}

void read_RAN(sm_ag_if_rd_t* data)
{
  assert(data->type == MAC_STATS_V0 || data->type == RLC_STATS_V0 ||  data->type == PDCP_STATS_V0);

  srand(time(0));

  if(data->type == MAC_STATS_V0 ){
    mac_ind_msg_t* ind = &data->mac_stats.msg;
    ind->ue_stats = calloc(5, sizeof(mac_ue_stats_impl_t));
    assert(ind->ue_stats != NULL);

    ind->tstamp = time_now_us();
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

  } else if(data->type == RLC_STATS_V0) {

    data->rlc_stats.msg.tstamp = time_now_us();
    data->rlc_stats.msg.len = 5;
    data->rlc_stats.msg.rb = calloc(5, sizeof(rlc_radio_bearer_stats_t) );

    rlc_ind_msg_t* rlc = &data->rlc_stats.msg;
    for(int i = 0 ; i < 5; ++i){
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
  } else if (data->type == PDCP_STATS_V0 ){

    pdcp_ind_msg_t* ind_msg = &data->pdcp_stats.msg; 
    ind_msg->len = 5; //abs(rand()%10);
    ind_msg->tstamp =  time_now_us();

    if(ind_msg->len > 0){
      ind_msg->rb = calloc(ind_msg->len, sizeof(pdcp_radio_bearer_stats_t));
      assert(ind_msg->rb != NULL && "Memory exhausted!");
    }

    int const mod = 1024;
    for(uint32_t i = 0; i < ind_msg->len; ++i){
      pdcp_radio_bearer_stats_t* rb = &ind_msg->rb[i];

      rb->txpdu_bytes = abs(rand()%mod);    /* aggregated bytes of tx packets */
      rb->txpdu_pkts = rb->txpdu_bytes != 0 ? rb->txpdu_bytes - rand()%rb->txpdu_bytes : 0;     /* aggregated number of tx packets */
      rb->txpdu_sn= abs(rand()%mod);       /* current sequence number of last tx packet (or TX_NEXT) */

      rb->rxpdu_bytes = abs(rand()%mod);    /* aggregated bytes of rx packets */
      rb->rxpdu_pkts = rb->rxpdu_bytes != 0 ? rb->rxpdu_bytes - rand()%rb->rxpdu_bytes : 0;     /* aggregated number of rx packets */
      rb->rxpdu_sn= abs(rand()%mod);       /* current sequence number of last rx packet (or  RX_NEXT) */
      rb->rxpdu_oo_pkts= abs(rand()%mod);       /* aggregated number of out-of-order rx pkts  (or RX_REORD) */
      rb->rxpdu_oo_bytes= abs(rand()%mod); /* aggregated amount of out-of-order rx bytes */
      rb->rxpdu_dd_pkts= abs(rand()%mod);  /* aggregated number of duplicated discarded packets (or dropped packets because of other reasons such as integrity failure) (or RX_DELIV) */
      rb->rxpdu_dd_bytes= abs(rand()%mod); /* aggregated amount of discarded packets' bytes */
      rb->rxpdu_ro_count= abs(rand()%mod); /* this state variable indicates the COUNT value following the COUNT value associated with the PDCP Data PDU which triggered t-Reordering. (RX_REORD) */
      rb->txsdu_bytes = abs(rand()%mod);    /* number of bytes of SDUs delivered */
      rb->txsdu_pkts = rb->txsdu_bytes != 0 ? rb->txsdu_bytes - rand()% rb->txsdu_bytes : 0;     /* number of SDUs delivered */

      rb->rxsdu_bytes = abs(rand()%mod);    /* number of bytes of SDUs received */
      rb->rxsdu_pkts =  rb->rxsdu_bytes != 0 ? rb->rxsdu_bytes - rand()%rb->rxsdu_bytes : 0;     /* number of SDUs received */

      rb->rnti= abs(rand()%mod);
      rb->mode= abs(rand()%3);               /* 0: PDCP AM, 1: PDCP UM, 2: PDCP TM */
      rb->rbid= abs(rand()%11);

    }

  } else {
    assert("Invalid data type");
  }
}

sm_ag_if_ans_t write_RAN(sm_ag_if_wr_t const* data)
{
  assert(data != NULL);
  if(data->type == MAC_CTRL_REQ_V0){
    //printf("Control message called in the RAN \n");
    sm_ag_if_ans_t ans = {.type = MAC_AGENT_IF_CTRL_ANS_V0};
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
  const char server_ip_str[] = "127.0.0.1";
  const int mcc = 208; 
  const int mnc = 92; 
  const int mnc_digit_len = 2;
  const int nb_id = 42;
  sm_io_ag_t io = {.read = read_RAN, .write = write_RAN};
 
  init_agent_api(server_ip_str, mcc, mnc, mnc_digit_len, nb_id, io );

  sleep(2);

  // Init the RIC
  const char addr[] = "127.0.0.1"; 
  init_near_ric_api(addr);

  sleep(2);

  const uint16_t MAC_ran_func_id = 142;
  const char* cmd = "5_ms";
  report_service_near_ric_api(MAC_ran_func_id, cmd );

  sleep(3);

  const char* cmd2 = "Hello";
  control_service_near_ric_api(MAC_ran_func_id, cmd2 );  

  sleep(1);

//  load_sm_near_ric_api("../test/so/librlc_sm.so");

  const uint16_t RLC_ran_func_id = 143;
  report_service_near_ric_api(RLC_ran_func_id, cmd);
  sleep(2);

  const uint16_t PDCP_ran_func_id = 144;
  report_service_near_ric_api(PDCP_ran_func_id, cmd);

  sleep(5);

  rm_report_service_near_ric_api(MAC_ran_func_id, cmd);
  rm_report_service_near_ric_api(RLC_ran_func_id, cmd);
  rm_report_service_near_ric_api(PDCP_ran_func_id, cmd);

  sleep(1);

  // Stop the Agent
  stop_agent_api();

  // Stop the RIC
  stop_near_ric_api();
}
