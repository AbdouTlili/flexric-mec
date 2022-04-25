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


#include "../../src/ric/iApp/e42_iapp_api.h"
#include "../../src/util/alg_ds/alg/defer.h"
#include "near_ric_emulator.h"

#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

static
void fill_mac_ran_func(ran_function_t* r)
{
  assert(r != NULL); 
  r->id = 142;
  const char* str = "MAC_STATS_V0";
  r->def.buf = malloc(strlen(str));
  assert( r->def.buf != NULL && "memory exhausted");
  memcpy(r->def.buf, str, strlen(str) );
  r->def.len = strlen(str);
  r->oid = NULL;
  r->rev = 0;
}

static
void fill_rlc_ran_func(ran_function_t* r)
{
  assert(r != NULL); 
  r->id = 142;
  const char* str = "MAC_STATS_V0";
  r->def.buf = malloc(strlen(str));
  assert( r->def.buf != NULL && "memory exhausted");
  memcpy(r->def.buf, str, strlen(str) );
  r->def.len = strlen(str);
  r->oid = NULL;
  r->rev = 0;
}

int main()
{
  const char* addr = "127.0.0.1";

  near_ric_if_t ric_if = {.type = NULL};
  init_iapp_api(addr, ric_if);

  sleep(1);

  plmn_t p = { .mcc = 208,
    .mnc = 92,
    .mnc_digit_len = 2
  };

  global_e2_node_id_t node = {.type = ngran_gNB,
                              .plmn = p,
                              .nb_id = 42}; 

  size_t len = 2;
  ran_function_t ran_func[len];
  fill_mac_ran_func(&ran_func[0]);
  fill_rlc_ran_func(&ran_func[1]);

  add_e2_node_iapp_api(&node, len, ran_func);

  node.nb_id = 43;
  add_e2_node_iapp_api(&node, len, ran_func);

  sleep(1024);

  stop_iapp_api();     

  return EXIT_SUCCESS;
}

