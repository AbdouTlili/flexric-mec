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
#include "e2sm_hw_fb_app.h"

#include "flexric_agent.h"
#include "e2ap_agent.h"
#include "e2ap_multi_control.h"
#include "e2sm_hw_fb_rf.h"

static
void* start_server(void* arg)
{
  e2ap_ric_t* ric = (e2ap_ric_t*) arg;
  e2ap_start_server(ric);
  return NULL;
}

const global_e2_node_id_t ge2ni = {.type = ngran_gNB, .plmn = {.mcc = 10, .mnc = 15, .mnc_digit_len = 2}, .nb_id = 5555};
static
void* add_server(void* arg)
{
  e2ap_agent_t* ag = (e2ap_agent_t*) arg;
  usleep(500000); // add after half a second

  /* peak into agent to get the RFs */
  const size_t len_rf = ag->fun_registered.size;
  ran_function_t ran_func[len_rf];
  size_t rf_idx = 0;
  void copy_rf(void* container_v, void* node_v) {
    pair_ran_fun_t* p = (pair_ran_fun_t*)node_v;
    ran_function_t* node = p->rf_container->r;
    memcpy(&ran_func[rf_idx], node, sizeof(*node));
    rf_idx += 1;
  }
  iterate_rb_tree(&ag->fun_registered, ag->fun_registered.root, NULL, copy_rf);
  e2_setup_request_t sr = {
    .id = ge2ni,
    .ran_func_item = ran_func,
    .len_rf = len_rf,
    .comp_conf_update = NULL,
    .len_ccu = 0
  };
  ep_id_t ep_id = add_endpoint(ag, "127.0.0.1", 36422, &sr);
  printf("added new controller: ep_id %d\n", ep_id);
  return NULL;
}

typedef struct app_ag_s {
  e2sm_hw_fb_app_t* app1;
  e2sm_hw_fb_app_t* app2;
  e2ap_agent_t* ag;
} app_ag_t;
static
void* stop_all(void* arg)
{
  app_ag_t* app_ag = (app_ag_t*) arg;
  assert(app_ag->app1);
  assert(app_ag->app2);
  assert(app_ag->ag);

  sleep(5);
  e2sm_hw_fb_app_free(app_ag->app1); /* should be freed before freeing the agent, blocking */
  e2sm_hw_fb_app_free(app_ag->app2); /* should be freed before freeing the agent, blocking */
  e2ap_free_agent(app_ag->ag);
  return NULL;
}

int main()
{
  e2ap_ric_t ric1;
  memset(&ric1, 0, sizeof(ric1));
  e2ap_init_server(&ric1, "127.0.0.1", 36421);
  e2sm_hw_fb_app_t hw_app1;
  memset(&hw_app1, 0, sizeof(hw_app1));
  e2sm_hw_fb_app_init(&hw_app1, &ric1, 500);

  e2ap_ric_t ric2;
  memset(&ric2, 0, sizeof(ric2));
  e2ap_init_server(&ric2, "127.0.0.1", 36422);
  e2sm_hw_fb_app_t hw_app2;
  memset(&hw_app2, 0, sizeof(hw_app2));
  e2sm_hw_fb_app_init(&hw_app2, &ric2, 1000);

  pthread_t server;
  int rc = pthread_create(&server, NULL, start_server, &ric1);
  assert(rc == 0);
  rc = pthread_create(&server, NULL, start_server, &ric2);
  assert(rc == 0);

  e2ap_agent_t* ag = e2ap_init_agent("127.0.0.1", 36421, ge2ni);
  e2sm_hw_fb_register_ran_function(ag);

  pthread_t add_control;
  rc = pthread_create(&add_control, NULL, add_server, ag);
  assert(rc == 0);

  pthread_t stop;
  app_ag_t app_ag = { .app1 = &hw_app1, .app2 = &hw_app2, .ag = ag };
  rc = pthread_create(&stop, NULL, stop_all, &app_ag);
  assert(rc == 0);

  e2ap_start_agent(ag); /* blocking */

  e2ap_free_server(&ric1);
  e2ap_free_server(&ric2);
}
