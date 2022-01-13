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


#include "e2ap_ran_function.h"

#include <assert.h>
#include <stdlib.h>


ran_function_t cp_ran_function(const ran_function_t* src)
{
  assert(src != NULL);
  ran_function_t dst = {.id = src->id, .rev = src->rev }; 
  dst.def = copy_byte_array(src->def);
  if(src->oid != NULL){
    dst.oid = calloc(1, sizeof(byte_array_t));
    assert(dst.oid != NULL && "Memory exhausted");
    *dst.oid = copy_byte_array(*src->oid);
  }
  return dst;
}

void free_ran_function(ran_function_t* src)
{
  assert(src != NULL);
  free_byte_array(src->def);
  if(src->oid != NULL){
    free_byte_array(*src->oid);
    free(src->oid);
  }
}

bool eq_ran_function(const ran_function_t* m0, const ran_function_t* m1)
{
  if(m0 == m1) return true;

  if(m0 == NULL || m1 == NULL)
    return false;

  if(eq_byte_array(&m0->def, &m1->def) == false)
    return false;

  if(m0->id != m1->id)
    return false;

  if(m0->rev != m1->rev)
    return false;

  if(eq_byte_array(m0->oid, m1->oid) == false)
    return false;

  return true;
}
