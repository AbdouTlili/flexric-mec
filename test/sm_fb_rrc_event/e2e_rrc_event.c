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

#include <assert.h>
#include <pthread.h>
#include <stdio.h>

#include "e2ap_server.h"
#include "rrc_event_app.h"

#include "flexric_agent.h"
#include "rrc_event_rf.h"

static
void* start_server(void* arg)
{
  e2ap_ric_t* ric = (e2ap_ric_t*) arg;
  e2ap_start_server(ric);
  return NULL;
}

typedef struct app_ag_s {
  e2sm_rrc_event_app_t* app;
  e2ap_agent_t* ag;
  bool run_thread;
  pthread_t thread;
} app_ag_t;
static
void* stop_all(void* arg)
{
  app_ag_t* app_ag = (app_ag_t*) arg;
  assert(app_ag->app);
  assert(app_ag->ag);

  sleep(3);
  e2sm_rrc_event_app_free(app_ag->app); /* should be freed before freeing the agent, blocking */
  app_ag->run_thread = false;
  pthread_join(app_ag->thread, NULL);
  e2ap_free_agent(app_ag->ag);
  return NULL;
}

static
void generate_rrc_event_attach_message(flatcc_builder_t* B, const rrc_event_report_style_t* style, void* ctxt)
{
  assert(B);
  assert(!ctxt); // might be used be the UP to point to the UE to encode
  assert(style->type == rrc_event_ReportStyleType_Minimal || style->type == rrc_event_ReportStyleType_Complete);
  rrc_event_IndicationMessage_start(B);
  // in order rrc_event_IndicationMessage_create()
  rrc_event_NrExt_start(B);
    rrc_event_NrExt_sNssaiList_start(B);
      rrc_common_Snssai_vec_push_create(B, 1, 2);
      rrc_common_Snssai_vec_push_create(B, 1, 3);
    rrc_event_NrExt_sNssaiList_end(B);
  rrc_event_NrExt_ref_t nrext = rrc_event_NrExt_end(B);
  rrc_event_IndicationMessage_nrExt_add(B, nrext);
  rrc_event_IndicationMessage_rat_force_add(B, rrc_common_RAT_NR);
  rrc_event_IndicationMessage_selectedPlmnId_force_add(B, 0);
  rrc_event_IndicationMessage_end_as_root(B);
}

void generate_rrc_event_complete_message(flatcc_builder_t* B, const rrc_event_report_style_t* style, void* ctxt)
{
  assert(B);
  assert(!ctxt); // might be used be the UP to point to the UE to encode
  assert(style->type == rrc_event_ReportStyleType_Minimal || style->type == rrc_event_ReportStyleType_Complete);
  rrc_event_IndicationMessage_start(B);
  // in order rrc_event_IndicationMessage_create()
  rrc_event_IndicationMessage_imsi_create_str(B, "208950121");
  static const char* cap = "FANCY_CAPABILITY";
  rrc_event_IndicationMessage_ueCapabilityRrcContainer_create(B, (uint8_t*) cap, strlen(cap));
  rrc_event_LTECapability_ref_t ltecap = rrc_event_LTECapability_create(B, 14, 12, true);
  rrc_event_IndicationMessage_lteExt_create(B, ltecap);
  rrc_event_IndicationMessage_rat_force_add(B, rrc_common_RAT_LTE);
  rrc_event_IndicationMessage_end_as_root(B);
}

static
void* periodic_rrc_event_trigger(void* arg)
{
  app_ag_t* stuff = (app_ag_t*) arg;
  e2ap_agent_t* ag = stuff->ag;
  int ev = 0;
  while (stuff->run_thread) {
    usleep(250000);
    const uint16_t rnti = 0x1234;
    //printf("triggered RRC event %8s for RNTI %04x\n", rrc_event_Event_name(ev), rnti);
    if (ev == rrc_event_Event_Attach)
      sm_rrc_event_trigger(ag, rnti, ev, generate_rrc_event_attach_message, NULL);
    else if (ev == rrc_event_Event_Complete)
      sm_rrc_event_trigger(ag, rnti, ev, generate_rrc_event_complete_message, NULL);
    else if (ev == rrc_event_Event_Release)
      sm_rrc_event_trigger(ag, rnti, ev, NULL, NULL);
    ev = (ev + 1) % 3;
  }
  return NULL;
}

typedef struct app_ed_s {
  e2sm_rrc_event_app_t* app;
  ue_ed_t ue_ed;
} app_ed_t;
static
void ue_attach(const ran_t* ran, uint16_t rnti, byte_array_t msg, void* data)
{
  printf("UE attach of RNTI %04x at RAN %lx\n", rnti, ran->generated_ran_key);
  assert(msg.buf && msg.len > 0);
  app_ed_t* app_ed = (app_ed_t*) data;
  assert(ran == app_ed->app->ran);
  unsubscribe_ue(app_ed->app, app_ed->ue_ed);
}

static
void ue_conn_complete(const ran_t* ran, uint16_t rnti, byte_array_t msg, void* data)
{
  printf("UE complete of RNTI %04x at RAN %lx\n", rnti, ran->generated_ran_key);
  assert(msg.buf && msg.len > 0);
  app_ed_t* app_ed = (app_ed_t*) data;
  assert(ran == app_ed->app->ran);
}

static
void ue_release(const ran_t* ran, uint16_t rnti, byte_array_t msg, void* data)
{
  printf("UE release of RNTI %04x at RAN %lx\n", rnti, ran->generated_ran_key);
  assert(msg.buf && msg.len > 0);
  app_ed_t* app_ed = (app_ed_t*) data;
  assert(ran == app_ed->app->ran);
}

int main()
{
  e2ap_ric_t ric;
  memset(&ric, 0, sizeof(ric));
  e2ap_init_server(&ric, "127.0.0.1", 36421);

  e2sm_rrc_event_app_t app;
  memset(&app, 0, sizeof(app));
  e2sm_rrc_event_app_init(&app, &ric, 1);
  app_ed_t app_ed_attach = { .app = &app };
  app_ed_attach.ue_ed = subscribe_ue_attach(&app, ue_attach, &app_ed_attach);
  app_ed_t app_ed_conn_complete = { .app = &app };
  app_ed_conn_complete.ue_ed = subscribe_ue_conn_complete(&app, ue_conn_complete, &app_ed_conn_complete);
  app_ed_t app_ed_release = { .app = &app };
  app_ed_release.ue_ed = subscribe_ue_release(&app, ue_release, &app_ed_release);

  pthread_t server;
  int rc = pthread_create(&server, NULL, start_server, &ric);
  assert(rc == 0);

  const plmn_t plmn = {.mcc = 10, .mnc = 15, .mnc_digit_len = 2};
  const global_e2_node_id_t ge2ni = {.type = ngran_gNB, .plmn = plmn, .nb_id = 5555};
  e2ap_agent_t* ag = e2ap_init_agent("127.0.0.1", 36421, ge2ni);
  rrc_event_report_style_t style = { .type = rrc_event_ReportStyleType_Minimal };
  sm_rrc_event_register_ran_function(ag, &style, 1);

  app_ag_t app_ag = { .app = &app, .ag = ag, .run_thread = true };
  rc = pthread_create(&app_ag.thread, NULL, periodic_rrc_event_trigger, &app_ag);
  assert(rc == 0);

  pthread_t stop;
  rc = pthread_create(&stop, NULL, stop_all, &app_ag);
  assert(rc == 0);
  e2ap_start_agent(ag); /* blocking */

  e2ap_free_server(&ric);
}
