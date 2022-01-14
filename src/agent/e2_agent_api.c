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



#include "e2_agent_api.h"

#include <assert.h>                                        // for assert
#include <pthread.h>                                       // for pthread_cr...
#include <stdlib.h>
#include <stdio.h>                                         // for NULL
#include "e2_agent.h"                                      // for e2_free_agent
#include "lib/ap/e2ap_types/common/e2ap_global_node_id.h"  // for global_e2_...
#include "lib/ap/e2ap_types/common/e2ap_plmn.h"            // for plmn_t
#include "util/ngran_types.h"                              // for ngran_gNB
#include "util/conf_file.h"


static
e2_agent_t* agent = NULL;

static
pthread_t thrd_agent;

static inline
void* static_start_agent(void* a)
{
  (void)a;
  // Blocking...
  e2_start_agent(agent);
  return NULL;
}

void init_agent_api(int mcc, 
                    int mnc, 
                    int mnc_digit_len,
                    int nb_id,
                    sm_io_ag_t io
                    )
{
  assert(agent == NULL);  
  assert(mcc > 0);
  assert(mnc > 0);
  assert(mnc_digit_len > 0);
  assert(nb_id > 0);


  char* server_ip_str = get_near_ric_ip();
  printf("[E2 AGENT]: RIC IP Address = %s\n", server_ip_str);

  const plmn_t plmn = {.mcc = mcc, .mnc = mnc, .mnc_digit_len = mnc_digit_len};
  const global_e2_node_id_t ge2ni = {.type = ngran_gNB, .plmn = plmn, .nb_id = nb_id };
  const int e2ap_server_port = 36421;

  agent = e2_init_agent(server_ip_str, e2ap_server_port, ge2ni, io);

  // Spawn a new thread for the agent
  const int rc = pthread_create(&thrd_agent, NULL, static_start_agent, NULL);
  assert(rc == 0);
  free(server_ip_str);
}

void stop_agent_api(void)
{
  assert(agent != NULL);
  e2_free_agent(agent);
  int const rc = pthread_join(thrd_agent,NULL);
  assert(rc == 0);
}


