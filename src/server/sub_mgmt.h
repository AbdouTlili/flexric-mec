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

#ifndef SUB_MGMT_H
#define SUB_MGMT_H

#include "type_defs.h"
#include "subscription.h"
#include "hash_table.h"

typedef struct sub_mgmt_s {
  htable_t sub_tab;
  htable_t ctrl_tab;
} sub_mgmt_t;

void sub_mgmt_init(sub_mgmt_t* sub_mgmt);
void sub_mgmt_free(sub_mgmt_t* sub_mgmt);

size_t sub_mgmt_get_num_sub(const sub_mgmt_t* sub_mgmt);

void sub_mgmt_sub_add(sub_mgmt_t* sub_mgmt, int assoc_id, ric_gen_id_t id, cb_sub_t cb, void* data);
void sub_mgmt_sub_add_confirm(sub_mgmt_t* sub_mgmt, int assoc_id, const ric_subscription_response_t* resp);
void sub_mgmt_sub_add_reject(sub_mgmt_t* sub_mgmt, int assoc_id, const ric_subscription_failure_t* fail);
void sub_mgmt_indication(sub_mgmt_t *sub_mgmt, int assoc_id, const ric_indication_t *ind);

void sub_mgmt_sub_del(sub_mgmt_t* sub_mgmt, int assoc_id, ric_gen_id_t id, cb_sub_del_t cb);
void sub_mgmt_sub_del_confirm(sub_mgmt_t* sub_mgmt, int assoc_id, const ric_subscription_delete_response_t* sub);
void sub_mgmt_sub_del_reject(sub_mgmt_t* sub_mgmt, int assoc_id, const ric_subscription_delete_failure_t* sub);

size_t sub_mgmt_get_num_ctrl(const sub_mgmt_t* sub_mgmt);

void sub_mgmt_ctrl_req(sub_mgmt_t* sub_mgmt, int assoc_id, ric_gen_id_t id, cb_ctrl_t cb, void* data);
void sub_mgmt_ctrl_req_ack(sub_mgmt_t* sub_mgmt, int assoc_id, const ric_control_acknowledge_t* ack);
void sub_mgmt_ctrl_req_fail(sub_mgmt_t* sub_mgmt, int assoc_id, const ric_control_failure_t* fail);

#endif /* SUB_MGMT_H */
