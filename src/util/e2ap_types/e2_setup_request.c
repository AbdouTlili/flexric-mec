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

#include "e2_setup_request.h"

bool eq_e2_setup_request(const e2_setup_request_t* m0, const e2_setup_request_t* m1)
{
  if(m0 == m1) return true;

  if(m0 == NULL || m1 == NULL) return false;

  if(eq_global_e2_node_id(&m0->id,&m1->id) == false)
    return false;

  if(m0->len_rf != m1->len_rf)
    return false;

  for(size_t i = 0; i < m0->len_rf; ++i){
    if(eq_ran_function(&m0->ran_func_item[i], &m1->ran_func_item[i]) == false)
      return false;
  }

  if(m0->len_ccu != m1->len_ccu)
    return false;

  for(size_t i = 0; i < m0->len_ccu; ++i){
    if( eq_e2_node_component_config_update(&m0->comp_conf_update[i], &m1->comp_conf_update[i]) == false) 
      return false;
  }
  return true;
}

