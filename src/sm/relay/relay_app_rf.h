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

#ifndef RELAY_APP_RF_H
#define RELAY_APP_RF_H

/* A simplistic relay iApp: loaded onto a controller, it will forward one(!)
 * new agent connection to a pre-configured controller. In itself, it is not
 * really a service model, but works as a RAN function towards the higher-layer
 * controller. */

#include <pthread.h>

#include "flexric_agent.h"
#include "e2ap_server.h"

typedef struct relay_s {
  e2ap_ric_t* local_ric;
  const ran_t* ran;

  char* remote_addr;
  int remote_port;

  e2ap_agent_t* ag;
  pthread_t ag_thread;
} relay_t;

void relay_app_init(relay_t* relay, e2ap_ric_t* local_ric, const char* remote_addr, int remote_port);
void relay_app_free(relay_t* relay);

#endif /* RELAY_APP_RF_H */
