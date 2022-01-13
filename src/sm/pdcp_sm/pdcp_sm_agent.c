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


#include "pdcp_sm_agent.h"
#include "pdcp_sm_id.h"
#include "enc/pdcp_enc_generic.h"
#include "dec/pdcp_dec_generic.h"
#include "../../util/alg_ds/alg/defer.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct{

  sm_agent_t base;

#ifdef ASN
  pdcp_enc_asn_t enc;
#elif FLATBUFFERS 
  pdcp_enc_fb_t enc;
#elif PLAIN
  pdcp_enc_plain_t enc;
#else
  static_assert(false, "No encryptioin type selected");
#endif

} sm_pdcp_agent_t;


// Function pointers provided by the RAN for the 
// 5 procedures, 
// subscription, indication, control, 
// E2 Setup and RIC Service Update. 
//
static
subscribe_timer_t  on_subscription_pdcp_sm_ag(sm_agent_t* sm_agent, const sm_subs_data_t* data)
{
  assert(sm_agent != NULL);
  assert(data != NULL);

  sm_pdcp_agent_t* sm = (sm_pdcp_agent_t*)sm_agent;
 
  pdcp_event_trigger_t ev = pdcp_dec_event_trigger(&sm->enc, data->len_et, data->event_trigger);

  subscribe_timer_t timer = {.ms = ev.ms };
  return timer;
}

static
sm_ind_data_t on_indication_pdcp_sm_ag(sm_agent_t* sm_agent)
{
  //printf("on_indication called \n");

  assert(sm_agent != NULL);
  sm_pdcp_agent_t* sm = (sm_pdcp_agent_t*)sm_agent;

  sm_ind_data_t ret = {0};

  // Fill Indication Header
  pdcp_ind_hdr_t hdr = {.dummy = 0 };
  byte_array_t ba_hdr = pdcp_enc_ind_hdr(&sm->enc, &hdr );
  ret.ind_hdr = ba_hdr.buf;
  ret.len_hdr = ba_hdr.len;

  // Fill Indication Message 
  sm_ag_if_rd_t rd_if = {0};
  rd_if.type = PDCP_STATS_V0;
  sm->base.io.read(&rd_if);

// Liberate the memory if previously allocated by the RAN. It sucks
  pdcp_ind_data_t* ind = &rd_if.pdcp_stats;
  defer({ free_pdcp_ind_hdr(&ind->hdr) ;});
  defer({ free_pdcp_ind_msg(&ind->msg) ;});
  defer({ free_pdcp_call_proc_id(ind->proc_id);});

  byte_array_t ba = pdcp_enc_ind_msg(&sm->enc, &rd_if.pdcp_stats.msg);
  ret.ind_msg = ba.buf;
  ret.len_msg = ba.len;

  // Fill Call Process ID
  ret.call_process_id = NULL;
  ret.len_cpid = 0;

  return ret;
}

static
sm_ctrl_out_data_t on_control_pdcp_sm_ag(sm_agent_t* sm_agent, sm_ctrl_req_data_t const* data)
{
  assert(sm_agent != NULL);
  assert(data != NULL);
  sm_pdcp_agent_t* sm = (sm_pdcp_agent_t*) sm_agent;

  pdcp_ctrl_hdr_t hdr = pdcp_dec_ctrl_hdr(&sm->enc, data->len_hdr, data->ctrl_hdr);
  defer({ free_pdcp_ctrl_hdr(&hdr); });
  assert(hdr.dummy == 0 && "Only dummy == 0 supported ");

  pdcp_ctrl_msg_t msg = pdcp_dec_ctrl_msg(&sm->enc, data->len_msg, data->ctrl_msg);
  defer({ free_pdcp_ctrl_msg(&msg); });
  assert(msg.action == 42 && "Only action number 42 supported");

  sm_ag_if_wr_t wr = {.type = PDCP_CTRL_REQ_V0 };
  wr.pdcp_req_ctrl.msg = cp_pdcp_ctrl_msg(&msg);

  // Call the RAN
 sm_ag_if_ans_t ans = sm->base.io.write(&wr);
 assert(ans.type == PDCP_AGENT_IF_CTRL_ANS_V0);
  defer({ free_pdcp_ctrl_out(&ans.pdcp); });

  // Encode the answer from the RAN
  byte_array_t ba = pdcp_enc_ctrl_out(&sm->enc, &ans.pdcp);

  sm_ctrl_out_data_t ret = {0}; 
  ret.len_out = ba.len;
  ret.ctrl_out = ba.buf;

//  printf("on_control called \n");
  return ret;
}

static
sm_e2_setup_t on_e2_setup_pdcp_sm_ag(sm_agent_t* sm_agent)
{
  assert(sm_agent != NULL);
  //printf("on_e2_setup called \n");
  sm_pdcp_agent_t* sm = (sm_pdcp_agent_t*)sm_agent;

  sm_e2_setup_t setup = {.len_rfd =0, .ran_fun_def = NULL  }; 

  setup.len_rfd = strlen(sm->base.ran_func_name);
  setup.ran_fun_def = calloc(1, strlen(sm->base.ran_func_name));
  assert(setup.ran_fun_def != NULL);
  memcpy(setup.ran_fun_def, sm->base.ran_func_name, strlen(sm->base.ran_func_name));

  return setup;
}

static
void on_ric_service_update_pdcp_sm_ag(sm_agent_t* sm_agent, sm_ric_service_update_t const* data)
{
  assert(sm_agent != NULL);
  assert(data != NULL);


  printf("on_ric_service_update called \n");
}

static
void free_pdcp_sm_ag(sm_agent_t* sm_agent)
{
  assert(sm_agent != NULL);
  sm_pdcp_agent_t* sm = (sm_pdcp_agent_t*)sm_agent;
  free(sm);
}


sm_agent_t* make_pdcp_sm_agent(sm_io_ag_t io)
{
  sm_pdcp_agent_t* sm = calloc(1, sizeof(*sm));
  assert(sm != NULL && "Memory exhausted!!!");

  *(uint16_t*)(&sm->base.ran_func_id) = SM_PDCP_ID; 

  sm->base.io = io;
  sm->base.free_sm = free_pdcp_sm_ag;

  // O-RAN E2SM 5 Procedures
  sm->base.proc.on_subscription = on_subscription_pdcp_sm_ag;
  sm->base.proc.on_indication = on_indication_pdcp_sm_ag;
  sm->base.proc.on_control = on_control_pdcp_sm_ag;
  sm->base.proc.on_ric_service_update = on_ric_service_update_pdcp_sm_ag;
  sm->base.proc.on_e2_setup = on_e2_setup_pdcp_sm_ag;

  assert(strlen(SM_PDCP_STR) < sizeof( sm->base.ran_func_name) );
  memcpy(sm->base.ran_func_name, SM_PDCP_STR, strlen(SM_PDCP_STR)); 

  return &sm->base;
}

