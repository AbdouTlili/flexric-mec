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


#include "rlc_sm_agent.h"
#include "rlc_sm_id.h"
#include "enc/rlc_enc_generic.h"
#include "dec/rlc_dec_generic.h"
#include "../../util/alg_ds/alg/defer.h"


#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct{

  sm_agent_t base;

#ifdef ASN
  rlc_enc_asn_t enc;
#elif FLATBUFFERS 
  rlc_enc_fb_t enc;
#elif PLAIN
  rlc_enc_plain_t enc;
#else
  static_assert(false, "No encryption type selected");
#endif

} sm_rlc_agent_t;


// Function pointers provided by the RAN for the 
// 5 procedures, 
// subscription, indication, control, 
// E2 Setup and RIC Service Update. 
//
static
subscribe_timer_t  on_subscription_rlc_sm_ag(sm_agent_t* sm_agent, const sm_subs_data_t* data)
{
  assert(sm_agent != NULL);
  assert(data != NULL);

  sm_rlc_agent_t* sm = (sm_rlc_agent_t*)sm_agent;
 
  rlc_event_trigger_t ev = rlc_dec_event_trigger(&sm->enc, data->len_et, data->event_trigger);

  subscribe_timer_t timer = {.ms = ev.ms };
  return timer;
}

static
sm_ind_data_t on_indication_rlc_sm_ag(sm_agent_t* sm_agent)
{
//  printf("on_indication RLC called \n");

  assert(sm_agent != NULL);
  sm_rlc_agent_t* sm = (sm_rlc_agent_t*)sm_agent;

  sm_ind_data_t ret = {0};

  // Fill Indication Header
  rlc_ind_hdr_t hdr = {.dummy = 0 };
  byte_array_t ba_hdr = rlc_enc_ind_hdr(&sm->enc, &hdr );
  ret.ind_hdr = ba_hdr.buf;
  ret.len_hdr = ba_hdr.len;

  // Fill Indication Message 
  sm_ag_if_rd_t rd_if = {0};
  rd_if.type = RLC_STATS_V0;
  sm->base.io.read(&rd_if);

  // Liberate the memory if previously allocated by the RAN. It sucks
  rlc_ind_data_t* ind = &rd_if.rlc_stats;
  defer({ free_rlc_ind_hdr(&ind->hdr) ;});
  defer({ free_rlc_ind_msg(&ind->msg) ;});
  defer({ free_rlc_call_proc_id(ind->proc_id);});

  byte_array_t ba = rlc_enc_ind_msg(&sm->enc, &rd_if.rlc_stats.msg);
  ret.ind_msg = ba.buf;
  ret.len_msg = ba.len;

  // Fill Call Process ID
  ret.call_process_id = NULL;
  ret.len_cpid = 0;

  return ret;
}

static
 sm_ctrl_out_data_t on_control_rlc_sm_ag(sm_agent_t* sm_agent, sm_ctrl_req_data_t const* data)
{
  assert(sm_agent != NULL);
  assert(data != NULL);
  sm_rlc_agent_t* sm = (sm_rlc_agent_t*) sm_agent;

  rlc_ctrl_hdr_t hdr = rlc_dec_ctrl_hdr(&sm->enc, data->len_hdr, data->ctrl_hdr);
  assert(hdr.dummy == 0 && "Only dummy == 0 supported ");

  rlc_ctrl_msg_t msg = rlc_dec_ctrl_msg(&sm->enc, data->len_msg, data->ctrl_msg);
  assert(msg.action == 42 && "Only action number 42 supported");

  sm_ag_if_wr_t wr = {.type = RLC_CTRL_REQ_V0 };
  wr.rlc_ctrl.hdr.dummy = 0; 
  wr.rlc_ctrl.msg.action = msg.action;

  sm->base.io.write(&wr);

  // Answer from the E2 Node
  sm_ctrl_out_data_t ret = {0};
  ret.ctrl_out = NULL;
  ret.len_out = 0;

  printf("on_control called \n");
  return ret;
}

static
sm_e2_setup_t on_e2_setup_rlc_sm_ag(sm_agent_t* sm_agent)
{
  assert(sm_agent != NULL);
  //printf("on_e2_setup called \n");
  sm_rlc_agent_t* sm = (sm_rlc_agent_t*)sm_agent;

  sm_e2_setup_t setup = {.len_rfd =0, .ran_fun_def = NULL  }; 

  setup.len_rfd = strlen(sm->base.ran_func_name);
  setup.ran_fun_def = calloc(1, strlen(sm->base.ran_func_name));
  assert(setup.ran_fun_def != NULL);
  memcpy(setup.ran_fun_def, sm->base.ran_func_name, strlen(sm->base.ran_func_name));

  return setup;
}

static
void on_ric_service_update_rlc_sm_ag(sm_agent_t* sm_agent, sm_ric_service_update_t const* data)
{
  assert(sm_agent != NULL);
  assert(data != NULL);


  printf("on_ric_service_update called \n");
}

static
void free_rlc_sm_ag(sm_agent_t* sm_agent)
{
  assert(sm_agent != NULL);
  sm_rlc_agent_t* sm = (sm_rlc_agent_t*)sm_agent;
  free(sm);
}


sm_agent_t* make_rlc_sm_agent(sm_io_ag_t io)
{
  sm_rlc_agent_t* sm = calloc(1, sizeof(sm_rlc_agent_t));
  assert(sm != NULL && "Memory exhausted!!!");

  *(uint16_t*)(&sm->base.ran_func_id) = SM_RLC_ID; 

  sm->base.io = io;
  sm->base.free_sm = free_rlc_sm_ag;

  sm->base.proc.on_subscription = on_subscription_rlc_sm_ag;
  sm->base.proc.on_indication = on_indication_rlc_sm_ag;
  sm->base.proc.on_control = on_control_rlc_sm_ag;
  sm->base.proc.on_ric_service_update = on_ric_service_update_rlc_sm_ag;
  sm->base.proc.on_e2_setup = on_e2_setup_rlc_sm_ag;
  sm->base.handle = NULL;

  assert(strlen(SM_RLC_STR) < sizeof( sm->base.ran_func_name) );
  memcpy(sm->base.ran_func_name, SM_RLC_STR, strlen(SM_RLC_STR)); 

  return &sm->base;
}


uint16_t id_rlc_sm_agent(sm_agent_t const* sm_agent )
{
  assert(sm_agent != NULL);
  sm_rlc_agent_t* sm = (sm_rlc_agent_t*)sm_agent;
  return sm->base.ran_func_id;
}

