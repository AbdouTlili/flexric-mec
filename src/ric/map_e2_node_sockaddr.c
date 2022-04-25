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

#include "map_e2_node_sockaddr.h"

#include "../util/alg_ds/ds/lock_guard/lock_guard.h"
#include "../util/alg_ds/alg/alg.h"
#include "../lib/ep/sctp_msg.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

static inline
void free_sctp_info(void* key, void* value)
{
  assert(key != NULL);
  assert(value != NULL);

  sctp_info_t* s = (sctp_info_t*)value;
  free(s);
}

void init_map_e2_node_sad(map_e2_node_sockaddr_t* m)
{
  assert(m != NULL);

  pthread_mutexattr_t *mtx_attr = NULL;
#ifdef DEBUG
  *mtx_attr = PTHREAD_MUTEX_ERRORCHECK; 
#endif

  int rc = pthread_mutex_init(&m->mtx, mtx_attr);
  assert(rc == 0);

  const size_t key_sz = sizeof(global_e2_node_id_t);
  assoc_init(&m->tree, key_sz, cmp_global_e2_node_id_wrapper, free_sctp_info);
}

void free_map_e2_node_sad(map_e2_node_sockaddr_t* m)
{
  assert(m != NULL);

  int rc = pthread_mutex_destroy(&m->mtx);
  assert(rc == 0);
  assoc_free(&m->tree);
}

void add_map_e2_node_sad(map_e2_node_sockaddr_t* m, global_e2_node_id_t const* id,sctp_info_t* s)
{
  assert(m != NULL);
  assert(id != NULL);
  assert(s != NULL);

  lock_guard(&m->mtx);

  void* it = assoc_front(&m->tree);
  void* end = assoc_end(&m->tree);

  it = find_if(&m->tree, it, end, id, eq_global_e2_node_id_wrapper);
  assert(it == end && "E2 Node already registered in the tree");

  sctp_info_t* info = calloc(1, sizeof(sctp_info_t)); 
  assert(info != NULL && "Memory exhausted");

  info->addr.sin_addr.s_addr = s->addr.sin_addr.s_addr;
  info->addr.sin_port = s->addr.sin_port;
  info->addr.sin_family = s->addr.sin_family;

  info->sri = s->sri;

  printf("adding nb id %d with port number %d \n", id->nb_id, info->addr.sin_port);
  assoc_insert(&m->tree, id, sizeof(global_e2_node_id_t), info);
}

void rm_map_e2_node_sad(map_e2_node_sockaddr_t* m, global_e2_node_id_t* id)
{
  assert(m != NULL);
  assert(id != NULL);

  assert(0!=0 && "Not tested, it should work");

  lock_guard(&m->mtx);
  sctp_info_t* s = assoc_extract(&m->tree, id);
  free(s);
}

sctp_info_t find_map_e2_node_sad(map_e2_node_sockaddr_t* m, global_e2_node_id_t const* id)
{
  assert(m != NULL);
  assert(id != NULL);

  lock_guard(&m->mtx);

  void* it = assoc_front(&m->tree);
  void* end = assoc_end(&m->tree);

  it = find_if(&m->tree, it, end, (global_e2_node_id_t*)id, eq_global_e2_node_id_wrapper);
  assert(it != end && "E2 Node not found in the tree");

  sctp_info_t* s = assoc_value(&m->tree, it);  

  printf("[NEAR-RIC]: nb_id %d port = %d  \n", id->nb_id, s->addr.sin_port);

  return *s;
}

