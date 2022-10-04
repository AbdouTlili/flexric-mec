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


#include "e2ap_global_node_id.h"

#include <assert.h>
#include <stddef.h>

bool eq_global_e2_node_id(const global_e2_node_id_t* m0, const global_e2_node_id_t* m1)
{
  if(m0 == m1) return true;

  if(m0 == NULL || m1 == NULL) return false;

  if(m0->type != m1->type)
    return false;

  if(m0->nb_id != m1->nb_id)
    return false;

  if(eq_plmn(&m0->plmn, &m1->plmn) == false)
    return false;

  return true;
}


global_e2_node_id_t cp_global_e2_node_id(global_e2_node_id_t const* src)
{
  assert(src != NULL);
  global_e2_node_id_t dst = {0};

  dst.type = src->type;
  dst.plmn = cp_plmn(&src->plmn);
  dst.nb_id = src->nb_id;

  return dst;
}

bool eq_global_e2_node_id_wrapper(const void* m0_v, const void* m1_v)
{
  assert(m0_v != NULL);
  assert(m1_v != NULL);

  global_e2_node_id_t* m0 = (global_e2_node_id_t*)m0_v;
  global_e2_node_id_t* m1 = (global_e2_node_id_t*)m1_v;

  return eq_global_e2_node_id(m0,m1);
}

int cmp_global_e2_node_id(const global_e2_node_id_t* m0, const global_e2_node_id_t* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);

  if (m0->type < m1->type)
    return -1;
  else if(m0->type > m1->type)
    return 1;

  int rc = cmp_plmn(&m0->plmn, &m1->plmn);
  if(rc != 0)
    return rc;

  if (m0->nb_id < m1->nb_id)
    return -1;
  else if(m0->nb_id > m1->nb_id)
    return 1;

  return 0;
}

int cmp_global_e2_node_id_wrapper(const void* m0_v, const void* m1_v)
{
  assert(m0_v != NULL);
  assert(m1_v != NULL);

  global_e2_node_id_t* m0 = (global_e2_node_id_t*)m0_v;
  global_e2_node_id_t* m1 = (global_e2_node_id_t*)m1_v;

  return cmp_global_e2_node_id(m0,m1);
}




