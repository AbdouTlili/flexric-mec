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
                    ngran_node_t ran_type,
                    sm_io_ag_t io,
                    fr_args_t const* args)
{
  assert(agent == NULL);
  assert(nb_id > 0);
  assert(mcc > 0);
  assert(mnc > 0);
  assert(mnc_digit_len > 0);

  char* server_ip_str = get_conf_ip(args);
  char* server_port_str = get_conf_e2port(args);
  // TODO: fix the type from config
  // ngran_node_t rantype = get_conf_rantype(args);
  uint64_t cu_du_id = 0;
  if (ran_type == ngran_gNB_DU || ran_type == ngran_gNB_CU || ran_type == ngran_gNB || ran_type == ngran_eNB) {
    // TODO: fix the CU/DU id from config
    // char* cu_du_id_str = get_conf_cu_du_id(args);
    // cu_du_id = atoi(cu_du_id_str);
    if (ran_type == ngran_gNB_DU || ran_type == ngran_gNB_CU)
      cu_du_id = abs(rand()%9999);
    uint64_t id = 0;
    if (ran_type == ngran_gNB) id = nb_id;
    else id = cu_du_id;
    printf("[E2 AGENT]: nearRT-RIC IP Address = %s, PORT = %s, RAN type = %d, nb_id = %ld\n", server_ip_str, server_port_str, ran_type, id);
  } else {
    assert(0 != 0 && "not support RAN type\n");
  }
  const plmn_t plmn = {.mcc = mcc, .mnc = mnc, .mnc_digit_len = mnc_digit_len};
  const global_e2_node_id_t ge2ni = {.type = ran_type, .plmn = plmn, .nb_id = nb_id, .cu_du_id = cu_du_id};
  const int e2ap_server_port = atoi(server_port_str);

  agent = e2_init_agent(server_ip_str, e2ap_server_port, ge2ni, io, args);

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


