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


#include "../../rlc_sm/rlc_sm_agent.h"
#include "../../rlc_sm/rlc_sm_ric.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

static
rlc_ind_data_t cp;


void fill_rlc_ind_data(rlc_ind_data_t* ind_data)
{

  srand(time(0));

  int const mod_len = 24;
  int const mod = 1024;

  rlc_ind_msg_t* ind_msg = &ind_data->msg;

  ind_msg->len = abs(rand()%mod_len);
  if(ind_msg->len > 0 ){
    ind_msg->rb = calloc(ind_msg->len, sizeof(rlc_radio_bearer_stats_t) );
    assert(ind_msg->rb != NULL);
  }
  printf("Value of the len = %u \n", ind_msg->len);

  for(uint32_t i = 0; i < ind_msg->len; ++i){
    rlc_radio_bearer_stats_t* rb = &ind_msg->rb[i];

    rb->txpdu_pkts=abs(rand()%mod) ;         /* aggregated number of transmitted RLC PDUs */
    rb->txpdu_bytes=abs(rand()%mod);        /* aggregated amount of transmitted bytes in RLC PDUs */
    /* TODO? */
    rb->txpdu_wt_ms=abs(rand()%mod);      /* aggregated head-of-line tx packet waiting time to be transmitted (i.e. send to the MAC layer) */
    rb->txpdu_dd_pkts=abs(rand()%mod);      /* aggregated number of dropped or discarded tx packets by RLC */
    rb->txpdu_dd_bytes=abs(rand()%mod);     /* aggregated amount of bytes dropped or discarded tx packets by RLC */
    rb->txpdu_retx_pkts=abs(rand()%mod);    /* aggregated number of tx pdus/pkts to be re-transmitted (only applicable to RLC AM) */
    rb->txpdu_retx_bytes=abs(rand()%mod);   /* aggregated amount of bytes to be re-transmitted (only applicable to RLC AM) */
    rb->txpdu_segmented=abs(rand()%mod);    /* aggregated number of segmentations */
    rb->txpdu_status_pkts=abs(rand()%mod);  /* aggregated number of tx status pdus/pkts (only applicable to RLC AM) */
    rb->txpdu_status_bytes=abs(rand()%mod); /* aggregated amount of tx status bytes  (only applicable to RLC AM) */
    /* TODO? */
    rb->txbuf_occ_bytes=abs(rand()%mod);    /* current tx buffer occupancy in terms of amount of bytes (average: NOT IMPLEMENTED) */
    /* TODO? */
    rb->txbuf_occ_pkts=abs(rand()%mod);     /* current tx buffer occupancy in terms of number of packets (average: NOT IMPLEMENTED) */
    /* txbuf_wd_ms: the time window for which the txbuf  occupancy value is obtained - NOT IMPLEMENTED */

    /* RX */
    rb->rxpdu_pkts=abs(rand()%mod);         /* aggregated number of received RLC PDUs */
    rb->rxpdu_bytes=abs(rand()%mod);        /* amount of bytes received by the RLC */
    rb->rxpdu_dup_pkts=abs(rand()%mod);     /* aggregated number of duplicate packets */
    rb->rxpdu_dup_bytes=abs(rand()%mod);    /* aggregated amount of duplicated bytes */
    rb->rxpdu_dd_pkts=abs(rand()%mod);      /* aggregated number of rx packets dropped or discarded by RLC */
    rb->rxpdu_dd_bytes=abs(rand()%mod);     /* aggregated amount of rx bytes dropped or discarded by RLC */
    rb->rxpdu_ow_pkts=abs(rand()%mod);      /* aggregated number of out of window received RLC pdu */
    rb->rxpdu_ow_bytes=abs(rand()%mod);     /* aggregated number of out of window bytes received RLC pdu */
    rb->rxpdu_status_pkts=abs(rand()%mod);  /* aggregated number of rx status pdus/pkts (only applicable to RLC AM) */
    rb->rxpdu_status_bytes=abs(rand()%mod); /* aggregated amount of rx status bytes  (only applicable to RLC AM) */
    /* rxpdu_rotout_ms: flag indicating rx reordering  timeout in ms - NOT IMPLEMENTED */
    /* rxpdu_potout_ms: flag indicating the poll retransmit time out in ms - NOT IMPLEMENTED */
    /* rxpdu_sptout_ms: flag indicating status prohibit timeout in ms - NOT IMPLEMENTED */
    /* TODO? */
    rb->rxbuf_occ_bytes=abs(rand()%mod);    /* current rx buffer occupancy in terms of amount of bytes (average: NOT IMPLEMENTED) */
    /* TODO? */
    rb->rxbuf_occ_pkts=abs(rand()%mod);     /* current rx buffer occupancy in terms of number of packets (average: NOT IMPLEMENTED) */

    /* SDU stats */
    /* TX */
    rb->txsdu_pkts=abs(rand()%mod);         /* number of SDUs delivered */
    rb->txsdu_bytes=abs(rand()%mod);        /* number of bytes of SDUs delivered */

    /* RX */
    rb->rxsdu_pkts=abs(rand()%mod);         /* number of SDUs received */
    rb->rxsdu_bytes=abs(rand()%mod);        /* number of bytes of SDUs received */
    rb->rxsdu_dd_pkts=abs(rand()%mod);      /* number of dropped or discarded SDUs */
    rb->rxsdu_dd_bytes=abs(rand()%mod);     /* number of bytes of SDUs dropped or discarded */

    rb->rnti=abs(rand()%mod);
    rb->mode=abs(rand()%mod);               /* 0: RLC AM, 1: RLC UM, 2: RLC TM */
    rb->rbid=abs(rand()%mod);

  }

  cp.hdr = cp_rlc_ind_hdr(&ind_data->hdr);
  cp.msg =  cp_rlc_ind_msg(&ind_data->msg);
}



/////
// AGENT
////

static
void read_RAN(sm_ag_if_rd_t* read)
{
  assert(read != NULL);
  assert(read->type == RLC_STATS_V0);

  fill_rlc_ind_data(&read->rlc_stats);
}


static 
sm_ag_if_ans_t write_RAN(sm_ag_if_wr_t const* data)
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
  if(sm_data.call_process_id != NULL){
    assert(sm_data.len_cpid != 0);
  }
  if(sm_data.ind_hdr != NULL){
    assert(sm_data.len_hdr != 0);
  }
  if(sm_data.ind_msg != NULL){
    assert(sm_data.len_msg != 0);
  }

 sm_ag_if_rd_t msg = ric->proc.on_indication(ric, &sm_data);

  assert(msg.type == RLC_STATS_V0);


  rlc_ind_data_t* data = &msg.rlc_stats;

 if(msg.rlc_stats.msg.rb != NULL){
      assert(msg.rlc_stats.msg.len != 0);
 } 


  assert(eq_rlc_ind_hdr(&data->hdr, &cp.hdr) == true);
  assert(eq_rlc_ind_msg(&data->msg, &cp.msg) == true);
  assert(eq_rlc_call_proc_id(data->proc_id, cp.proc_id) == true);

  free_rlc_ind_hdr(&data->hdr);
  free_rlc_ind_msg(&data->msg);

  free_sm_ind_data(&sm_data); 
}

int main()
{
  sm_io_ag_t io_ag = {.read = read_RAN, .write = write_RAN};  
  sm_agent_t* sm_ag = make_rlc_sm_agent(io_ag);

  sm_ric_t* sm_ric = make_rlc_sm_ric();

  check_eq_ran_function(sm_ag, sm_ric);
  check_subscription(sm_ag, sm_ric);
  check_indication(sm_ag, sm_ric);

  sm_ag->free_sm(sm_ag);
  sm_ric->free_sm(sm_ric);

  printf("Success\n");
  return EXIT_SUCCESS;
}

