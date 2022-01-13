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



#ifndef RIC_SERVICE_UPDATE_H
#define  RIC_SERVICE_UPDATE_H

#include "common/e2ap_ran_function.h"
#include "common/e2ap_ran_function_id_rev.h"

typedef struct {
  ran_function_t* added;
  size_t len_added;

  ran_function_t* modified;
  size_t len_modified;

  e2ap_ran_function_id_rev_t* deleted;
  size_t len_deleted;
} ric_service_update_t;

bool eq_ric_service_update(const ric_service_update_t* m0, const ric_service_update_t* m1);

#endif
