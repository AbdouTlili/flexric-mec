/*
 * Implementation agent side of SM KPM

 * Note that the agent side functionalities at E2AP level is already implemented with generic code
 * given that the framework is the same for all SM, it changes only the content of the IE at applicative layer SM, i.e. SM-KPM, SM-PDCP
 * 
 * I.e. ON SUBSCRIPTION, implemented in src/agent/msg_handler_agent.c:e2ap_handle_subscription_request_agent()) 
 * and following O-RAN.WG3.E2AP-v01.01, $8.2.1.2, the functionality is expressed below:
 * "
 * At reception of the RIC SUBSCRIPTION REQUEST message the target E2 Node shall:
 * - Determine the target function using the information in the RAN Function ID IE and configure the requested
 *   event trigger using information in the RIC Subscription Details IE (part of RIC SUBSCRIPTION REQUEST sent by server RIC) 
 * - If one or more Report, Insert and/or Policy RIC service actions are included in the RIC Subscription Details IE
 *   then the target function shall validate the event trigger and requested action sequence and, if accepted, store the
 *   required RIC Request ID, RIC Event Trigger Definition IE and sequence of RIC Action ID IE, RIC Action Type
 *   IE, RIC Action Definition IE, if included, and RIC Subsequent Action IE, if included.
 * If the requested trigger and at least one required action are accepted by the target E2 Node, the target E2 Node shall
 * reserve necessary resources and send the RIC SUBSCRIPTION RESPONSE message back to the Near-RT RIC. The
 * target E2 Node shall include in the response message the actions for which resources have been prepared at the target
 * E2 Node in the RIC Actions Admitted List IE. The target E2 Node shall include the actions that have not been admitted
 * in the RIC Actions Not Admitted List IE with an appropriate cause value.
 * "
 * In this case, you would need just to implement in this source code the IE above mentioned (RAN Function ID IE, RIC Event Trigger 
 * Definition etc..)
 */
#include "kpm_sm_agent.h"
#include "kpm_sm_id.h"
#include "enc/kpm_enc_generic.h"
#include "dec/kpm_dec_generic.h"
#include "../../util/alg_ds/alg/defer.h"
#include "../../util/conversions.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct{

  sm_agent_t base;

#ifdef ASN
  kpm_enc_asn_t enc;
#elif FLATBUFFERS 
  //pdcp_enc_fb_t enc;
  static_assert(false, "Encryption FLATBUFFERS not implemented yet");
#elif PLAIN
  kpm_enc_plain_t enc;
#else
  static_assert(false, "No encryption type selected");
#endif
} sm_kpm_agent_t;

// O-RAN.WG3.E2SM-KPM-v02.02, $8.2.1.1.1
static
subscribe_timer_t on_subscription_kpm_sm_ag(sm_agent_t* sm_agent, const sm_subs_data_t* data)
{ 
  assert(sm_agent != NULL);
  assert(data != NULL);

  sm_kpm_agent_t* sm = (sm_kpm_agent_t*)sm_agent;
 
  kpm_event_trigger_t ev = kpm_dec_event_trigger(&sm->enc, data->len_et, data->event_trigger);

  subscribe_timer_t timer = {.ms = ev.ms};

// XXX: Leaving 'acd' doing nothing for the moment. We need to fix the logic upper layer and change 
// the signature of this function
  if (data->len_ad != 0){
    kpm_action_def_t acd = kpm_dec_action_def(&sm->enc, data->len_ad, data->action_def);
    free_kpm_action_def(&acd);
  }
  
  return timer;
}

// O-RAN.WG3.E2SM-KPM-v02.02, $8.2.1.3
static sm_ind_data_t on_indication_kpm_sm_ag(sm_agent_t* sm_agent)
{
  assert(sm_agent != NULL);
  sm_kpm_agent_t* sm = (sm_kpm_agent_t*)sm_agent;

  sm_ind_data_t ret = {0};

  // Fill Indication Message  and Header
  sm_ag_if_rd_t rd_if = {0};
  rd_if.type = KPM_STATS_V0;
  sm->base.io.read(&rd_if); 

  kpm_ind_data_t* ind = &rd_if.kpm_stats;
  defer({ free_kpm_ind_hdr(&ind->hdr) ;});
  defer({ free_kpm_ind_msg(&ind->msg) ;});

  byte_array_t ba_hdr = kpm_enc_ind_hdr(&sm->enc, &rd_if.kpm_stats.hdr);
  ret.ind_hdr = ba_hdr.buf;
  ret.len_hdr = ba_hdr.len;

  byte_array_t ba = kpm_enc_ind_msg(&sm->enc, &rd_if.kpm_stats.msg);
  ret.ind_msg = ba.buf;
  ret.len_msg = ba.len;

  // we do not have Call Process ID
  ret.call_process_id = NULL;
  ret.len_cpid = 0;

  return ret;
}

static
sm_e2_setup_t on_e2_setup_kpm_sm_ag(sm_agent_t* sm_agent)
{
  assert(sm_agent != NULL);

  sm_kpm_agent_t* sm = (sm_kpm_agent_t*)sm_agent;

  sm_e2_setup_t setup = {.len_rfd =0, .ran_fun_def = NULL  }; 

  kpm_func_def_t func_def = {0};
  
  func_def.ranFunction_Name.Description.buf = (uint8_t *)SM_KPM_DESCRIPTION;
  func_def.ranFunction_Name.Description.len = strlen(SM_KPM_DESCRIPTION);
  func_def.ranFunction_Name.ShortName.buf = (uint8_t *)SM_KPM_STR;
  func_def.ranFunction_Name.ShortName.len = strlen(SM_KPM_STR);
  
  func_def.ranFunction_Name.E2SM_OID.buf = (uint8_t *)SM_KPM_OID;
  func_def.ranFunction_Name.E2SM_OID.len = strlen(SM_KPM_OID);
  
  byte_array_t ba = kpm_enc_func_def(&sm->enc, &func_def);
  setup.ran_fun_def = ba.buf;
  setup.len_rfd = ba.len;
  
  return setup;
}

static 
void free_e2_setup_kpm_sm_ag (void *msg)
{
  assert(msg != NULL);

  sm_e2_setup_t * func_def  = (sm_e2_setup_t *)msg;

  free(func_def->ran_fun_def);
}

static
void on_ric_service_update_kpm_sm_ag(sm_agent_t* sm_agent, sm_ric_service_update_t const* data)
{
  assert(sm_agent != NULL);
  assert(data != NULL);

  printf("on_ric_service_update called \n");
}


static void free_kpm_sm_ag(sm_agent_t *sm_agent)
{
  assert(sm_agent != NULL);
  sm_kpm_agent_t *sm = (sm_kpm_agent_t*)sm_agent;
  free(sm);
}

sm_agent_t *make_kpm_sm_agent(sm_io_ag_t io)
{
  sm_kpm_agent_t *sm = calloc(1, sizeof(*sm));
  assert(sm != NULL && "Memory exhausted!!!");

  *(uint16_t*)(&sm->base.ran_func_id) = SM_KPM_ID; 

  sm->base.io = io;
  sm->base.free_sm = free_kpm_sm_ag;

  // Memory DeAllocation 
  sm->base.alloc.free_subs_data_msg = NULL;
  sm->base.alloc.free_ind_data = NULL;
  sm->base.alloc.free_ctrl_req_data = NULL;
  sm->base.alloc.free_ctrl_out_data = NULL;
  sm->base.alloc.free_e2_setup = free_e2_setup_kpm_sm_ag; 
  sm->base.alloc.free_ric_service_update = NULL;//free_ric_service_update_kpm_sm_ric; 

  // O-RAN E2SM 5 Procedures
  sm->base.proc.on_subscription       = on_subscription_kpm_sm_ag;
  sm->base.proc.on_indication         = on_indication_kpm_sm_ag;
  sm->base.proc.on_control            = NULL;
  sm->base.proc.on_ric_service_update = on_ric_service_update_kpm_sm_ag;
  sm->base.proc.on_e2_setup           = on_e2_setup_kpm_sm_ag;

  // SM name
  assert(strlen(SM_KPM_STR) < sizeof( sm->base.ran_func_name) );
  memcpy(sm->base.ran_func_name, SM_KPM_STR, strlen(SM_KPM_STR)); 

  return &sm->base;
}