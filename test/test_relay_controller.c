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
#include "relay_app_rf.h"
#include "e2sm_hw_fb_rf.h"

static
void* start_server(void* arg)
{
  e2ap_ric_t* ric = (e2ap_ric_t*) arg;
  e2ap_start_server(ric);
  return NULL;
}

typedef struct stuff_s {
  e2sm_hw_fb_app_t* hw_app;
  e2ap_agent_t* ag;
} stuff_t;
static
void* stop_all(void* arg)
{
  stuff_t* stuff = (stuff_t*) arg;
  assert(stuff->ag);
  assert(stuff->hw_app);

  sleep(5);
  e2sm_hw_fb_app_free(stuff->hw_app);
  e2ap_free_agent(stuff->ag);
  return NULL;
}

int main(int argc, char* argv[])
{
  if (argc > 2) {
    fprintf(stderr, "usage: %s [<hops>]\n", argv[0]);
    exit(1);
  }
  int hops = 1;
  if (argc == 2)
    hops = atoi(argv[1]);
  if (hops < 1){
    fprintf(stderr, "cannot have less than one hop\n");
    exit(2);
  }
  printf("measuring agent with %d hops\n", hops);

  /* n hops, ports:
   * top ric             : port 36421 + hops
   * hops'th - 1 relay   : port 36421 + hops - 1
   * ...
   * hops'th - hops relay: port 36421 + hops - hops
   * agent:              : to port 36421
   */

  const char* addr = "127.0.0.1";
  int top_ric_port = 36421 + hops;
  e2ap_ric_t top_ric;
  memset(&top_ric, 0, sizeof(top_ric));
  e2ap_init_server(&top_ric, addr, top_ric_port);
  e2sm_hw_fb_app_t hw_app;
  memset(&hw_app, 0, sizeof(hw_app));
  e2sm_hw_fb_app_init(&hw_app, &top_ric, 1000);
  pthread_t server;
  int rc = pthread_create(&server, NULL, start_server, &top_ric);
  assert(rc == 0);

  e2ap_ric_t relay_ric[hops];
  memset(relay_ric, 0, hops * sizeof(e2ap_ric_t));
  relay_t relay[hops];
  memset(relay, 0, hops * sizeof(relay_t));
  for (int i = hops - 1; i >= 0; --i) {
    e2ap_init_server(&relay_ric[i], addr, 36421 + i);
    relay_app_init(&relay[i], &relay_ric[i], addr, 36421 + i + 1);
    rc = pthread_create(&server, NULL, start_server, &relay_ric[i]);
    assert(rc == 0);
  }

  const plmn_t plmn = {.mcc = 10, .mnc = 15, .mnc_digit_len = 2};
  const global_e2_node_id_t ge2ni = {.type = ngran_gNB, .plmn = plmn, .nb_id = 5555};
  e2ap_agent_t* ag = e2ap_init_agent(addr, 36421, ge2ni);
  e2sm_hw_fb_register_ran_function(ag);

  pthread_t stop;
  stuff_t stuff = { .hw_app = &hw_app, .ag = ag };
  rc = pthread_create(&stop, NULL, stop_all, &stuff);
  assert(rc == 0);

  e2ap_start_agent(ag); /* blocking */

  for (int i = 0; i < hops; ++i) {
    relay_app_free(&relay[i]);
    e2ap_free_server(&relay_ric[i]);
  }
  e2ap_free_server(&top_ric);
}
