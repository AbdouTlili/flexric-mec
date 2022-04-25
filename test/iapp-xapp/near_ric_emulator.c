#define START_VALUE 42

#include "near_ric_emulator.h"

#include "../../src/lib/ap/free/e2ap_msg_free.h"
#include "../../src/lib/ap/ie/asn/E2AP-PDU.h"  
#include "../../src/lib/ap/enc/e2ap_msg_enc_asn.h"
#include "../../src/sm/mac_sm/ie/mac_data_ie.h"
#include "../../src/sm/mac_sm/enc/mac_enc_plain.h"
#include "../../src/sm/mac_sm/enc/mac_enc_generic.h"
#include "../../src/sm/mac_sm/mac_sm_id.h" 
#include "../../src/sm/rlc_sm/ie/rlc_data_ie.h"
#include "../../src/sm/rlc_sm/enc/rlc_enc_plain.h"
#include "../../src/sm/rlc_sm/enc/rlc_enc_generic.h"
#include "../../src/sm/rlc_sm/rlc_sm_id.h" 
#include "../../src/util/alg_ds/alg/defer.h"
#include "../common/fill_ind_data.h"

#include <assert.h>
#include <stdatomic.h>
#include <pthread.h>
#include <unistd.h>

static
void (* e2_msg_ans)(e2ap_msg_t const* msg);

static pthread_t t_mac;
static pthread_t t_rlc;

static pthread_t t_sr_mac;
static pthread_t t_sr_rlc;

static pthread_t t_cr;

static
atomic_bool new_value_mac = false;

static
atomic_bool new_value_rlc = false;

static
atomic_bool stop_flag = false; 

static
ric_gen_id_t ric_id_mac = {}; 

static
ric_gen_id_t ric_id_rlc = {}; 

static
atomic_int ric_req_id = START_VALUE ;

static
int const ric_subscription_req_id_mac = START_VALUE - 2;

static
int const ric_subscription_req_id_rlc = START_VALUE - 1;

/*
static
int64_t time_now_us(void)
{
  struct timespec tms;


  if (clock_gettime(CLOCK_REALTIME,&tms)) {
    return -1;
  }
  int64_t micros = tms.tv_sec * 1000000;
  micros += tms.tv_nsec/1000;
  if (tms.tv_nsec % 1000 >= 500) {
    ++micros;
  }
  return micros;
}
*/

/*
static
void fill_mac_ind_data(mac_ind_data_t* ind_msg)
{
    mac_ind_msg_t* ind = &ind_msg->msg; // data->mac_stats.msg;
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
      ind->ue_stats[i].rnti = 1021;
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
      ind->ue_stats[i].phr = rand()%5000;
    }
}
*/

static
void* mac_subscription_thread(void* arg)
{
  (void)arg;
  while(new_value_mac == false ) 
    sleep(1);


  // Simulate latency between E2 Node and RIC...
  sleep(3);

  ric_id_mac.ric_req_id = ric_subscription_req_id_mac;
  ric_id_mac.ric_inst_id = 0;
  ric_id_mac.ran_func_id = 142;

  const size_t len_admitted = 1;;
  ric_action_admitted_t* admitted = calloc(len_admitted, sizeof(ric_action_admitted_t));
  admitted[0].ric_act_id = ric_id_mac.ric_req_id;
  ric_action_not_admitted_t* not_admitted = NULL;
  const size_t len_na = 0;

  ric_subscription_response_t sr_begin = {
    .ric_id = ric_id_mac,
    .admitted = admitted,
    .len_admitted = len_admitted,
    .not_admitted = not_admitted,
    .len_na = len_na
  };

  e2ap_msg_t msg = {.type = RIC_SUBSCRIPTION_RESPONSE,
    .u_msgs.ric_sub_resp = sr_begin};

  e2_msg_ans(&msg);

  new_value_mac = false;


  while(stop_flag == false){
    sleep(2);
   // if(stop_flag  == true) break;

    e2ap_msg_t msg = {.type = RIC_INDICATION }; 

    const uint8_t action_id = 19;
    ric_indication_type_e type = RIC_IND_REPORT ;

    ric_indication_t* dst = &msg.u_msgs.ric_ind;
    dst->ric_id = ric_id_mac;
    dst->action_id = action_id;
    dst->type = type;

    // Same steps as mac_sm_agent on_indication_mac_sm_ag 
    mac_enc_plain_t enc;
    // Fill Indication Header in plain mode
    mac_ind_hdr_t hdr = {.dummy = 0 };
    byte_array_t ba_hdr = mac_enc_ind_hdr(&enc, &hdr );

    // Fill Indication Message 
    mac_ind_data_t ind = {};
    fill_mac_ind_data(&ind);
    // Liberate the memory if previously allocated by the RAN. It sucks
    defer({ free_mac_ind_hdr(&ind.hdr) ;});
    defer({ free_mac_ind_msg(&ind.msg) ;});
    defer({ free_mac_call_proc_id(ind.proc_id);});
    byte_array_t ba_msg = mac_enc_ind_msg(&enc, &ind.msg);

    dst->hdr = ba_hdr;
    dst->msg = ba_msg;

    printf("From RIC Indication req_id = %d \n", dst->ric_id.ric_req_id);

    e2_msg_ans(&msg);

    e2ap_free_indication_msg(&msg);
  }
  return NULL;
}

static
void* rlc_subscription_thread(void* arg)
{
  (void)arg;
  while(new_value_rlc == false ) 
    sleep(1);


  // Simulate latency between E2 Node and RIC...
  sleep(3);

  ric_id_rlc.ric_req_id = ric_subscription_req_id_rlc;
  ric_id_rlc.ric_inst_id = 0;
  ric_id_rlc.ran_func_id = SM_RLC_ID;

  const size_t len_admitted = 1;;
  ric_action_admitted_t* admitted = calloc(len_admitted, sizeof(ric_action_admitted_t));
  admitted[0].ric_act_id = ric_id_rlc.ric_req_id;
  ric_action_not_admitted_t* not_admitted = NULL;
  const size_t len_na = 0;

  ric_subscription_response_t sr_begin = {
    .ric_id = ric_id_rlc,
    .admitted = admitted,
    .len_admitted = len_admitted,
    .not_admitted = not_admitted,
    .len_na = len_na
  };

  e2ap_msg_t msg = {.type = RIC_SUBSCRIPTION_RESPONSE,
    .u_msgs.ric_sub_resp = sr_begin};

  e2_msg_ans(&msg);

  new_value_rlc = false;


  while(stop_flag == false){
    sleep(2);

    e2ap_msg_t msg = {.type = RIC_INDICATION }; 

    const uint8_t action_id = 19;
    ric_indication_type_e type = RIC_IND_REPORT ;

    ric_indication_t* dst = &msg.u_msgs.ric_ind;
    dst->ric_id = ric_id_rlc;
    dst->action_id = action_id;
    dst->type = type;

    // Same steps as mac_sm_agent on_indication_mac_sm_ag 
    rlc_enc_plain_t enc;
    // Fill Indication Header in plain mode
    rlc_ind_hdr_t hdr = {.dummy = 0 };
    byte_array_t ba_hdr = rlc_enc_ind_hdr(&enc, &hdr );

    // Fill Indication Message 
    rlc_ind_data_t ind = {};
    fill_rlc_ind_data(&ind);
    // Liberate the memory if previously allocated by the RAN. It sucks
    defer({ free_rlc_ind_hdr(&ind.hdr) ;});
    defer({ free_rlc_ind_msg(&ind.msg) ;});
    defer({ free_rlc_call_proc_id(ind.proc_id);});
    byte_array_t ba_msg = rlc_enc_ind_msg(&enc, &ind.msg);

    dst->hdr = ba_hdr;
    dst->msg = ba_msg;

    printf("From RIC Indication req_id = %d \n", dst->ric_id.ric_req_id);

    e2_msg_ans(&msg);

    e2ap_free_indication_msg(&msg);

  }
  return NULL;
}

uint16_t fwd_ric_subscription_request_emulator(near_ric_if_emulator_t* ric,  ric_subscription_request_t const* sr,     void (*f_)(e2ap_msg_t const* msg) )
{
  assert(ric == NULL && "ric == NULL as it is not used");
  assert(sr != NULL);


  if(sr->ric_id.ran_func_id == SM_MAC_ID){
    e2_msg_ans = f_;
    new_value_mac = true; 
    return ric_subscription_req_id_mac;
  } 

  if (sr->ric_id.ran_func_id == SM_RLC_ID){
    assert(e2_msg_ans != NULL);
    new_value_rlc = true; 
    return ric_subscription_req_id_rlc;
  }
  
    assert(0!=0 && "Unknown subscription request received");
  
  return 0;
}

static
void* foo_sr_mac(void* arg)
{
  (void)arg;
  sleep(2);

  e2ap_msg_t msg = {.type = RIC_SUBSCRIPTION_DELETE_RESPONSE };

  ric_subscription_delete_response_t sdr = {
   .ric_id = ric_id_mac 
  }; 
  sdr.ric_id.ric_req_id = ric_subscription_req_id_mac; 

  msg.u_msgs.ric_sub_del_resp = sdr;

  e2_msg_ans(&msg);
  printf("[NEAR-RIC-SIMULATOR]: RIC_SUBSCRIPTION_DELETE_RESPONSE sent \n");

  stop_flag = true;

  return NULL;
}

static
void* foo_sr_rlc(void* arg)
{
  (void)arg;
  sleep(2);

  e2ap_msg_t msg = {.type = RIC_SUBSCRIPTION_DELETE_RESPONSE };

  ric_subscription_delete_response_t sdr = {
   .ric_id = ric_id_rlc 
  }; 
  sdr.ric_id.ric_req_id = ric_subscription_req_id_rlc; 

  msg.u_msgs.ric_sub_del_resp = sdr;

  e2_msg_ans(&msg);
  printf("[NEAR-RIC-SIMULATOR]: RIC_SUBSCRIPTION_DELETE_RESPONSE sent \n");

  stop_flag = true;

  return NULL;
}

void fwd_ric_subscription_request_delete_emulator(near_ric_if_emulator_t* ric,ric_subscription_delete_request_t const* sr, void (*f)(e2ap_msg_t const* msg) )
{
  assert(ric == NULL && "ric == NULL as it is not used");
  // It is the same subscription
  (void)sr;
  (void)f; 
  if(sr->ric_id.ran_func_id == SM_MAC_ID){
  // Simulate latency between E2 Node and RIC...
  int rc = pthread_create(&t_sr_mac, NULL, foo_sr_mac, NULL);
  assert(rc == 0);
  } else if(sr->ric_id.ran_func_id == SM_RLC_ID ){
  int rc = pthread_create(&t_sr_rlc, NULL, foo_sr_rlc, NULL);
  assert(rc == 0);

  } else {
    assert(0!=0 && "Unknwon ric_req_id" );
  }
  
}

static
atomic_int ctrl_tasks = 0;

static
pthread_mutex_t foo_mutex = PTHREAD_MUTEX_INITIALIZER;

static
void* foo_cr(void* arg)
{
  (void)arg;

  while(stop_flag  == false){

    sleep(1);

    if(ctrl_tasks == 0)
      continue;

    pthread_mutex_lock(&foo_mutex); 

    uint32_t tmp_ric_req_id = ctrl_tasks; 
    ctrl_tasks = 0; 

    pthread_mutex_unlock(&foo_mutex); 

    sleep(2);

    e2ap_msg_t msg = {.type = RIC_CONTROL_ACKNOWLEDGE };

    ric_control_acknowledge_t* dst = &msg.u_msgs.ric_ctrl_ack;

    dst->ric_id.ric_req_id = tmp_ric_req_id;
    dst->ric_id.ran_func_id = 145; // SLICE_SM_ID
    dst->ric_id.ric_inst_id = 0; 

    dst->status = RIC_CONTROL_STATUS_SUCCESS;

    e2_msg_ans(&msg);

  }
  return NULL;
}

uint16_t fwd_ric_control_request_emulator(near_ric_if_emulator_t* ric,ric_control_request_t const* cr,   void (*f)(e2ap_msg_t const* msg) )
{
  assert(ric == NULL && "ric == NULL as it is not used");
  (void)cr;
  (void)f;

  pthread_mutex_lock(&foo_mutex); 
  ctrl_tasks = ric_req_id++; 
  uint32_t tmp_ric_req_id = ctrl_tasks;
  pthread_mutex_unlock(&foo_mutex); 

  while(ctrl_tasks != 0)
    usleep(1000);

  return  tmp_ric_req_id;
}

void start_near_ric_iapp_emulator(near_ric_if_emulator_t* ric)
{
  (void)ric; // Needed when the real interface is needed
  stop_flag = false;
  int rc = pthread_create(&t_mac, NULL, mac_subscription_thread, NULL );
  assert(rc == 0);
  rc = pthread_create(&t_rlc, NULL, rlc_subscription_thread, NULL );
  assert(rc == 0);

  // Simulate latency between E2 Node and RIC...
  rc = pthread_create(&t_cr, NULL, foo_cr, NULL);
  assert(rc == 0);
}

void stop_near_ric_iapp_emulator()
{
  stop_flag = true;
  pthread_join(t_mac, NULL);
  pthread_join(t_rlc, NULL);
  pthread_join(t_sr_mac, NULL);
  pthread_join(t_sr_rlc, NULL);
  pthread_join(t_cr, NULL);
}

