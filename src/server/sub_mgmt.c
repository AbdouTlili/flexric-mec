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

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "sub_mgmt.h"

typedef struct sub_data_s {
  cb_sub_t cb_sub;
  cb_sub_del_t cb_sub_del;
  void* user_data;
  /* TODO: time out timer? */
} sub_data_t;

typedef struct ctrl_data_s {
  cb_ctrl_t cb_ctrl;
  void* user_data;
  /* TODO: time out timer? */
} ctrl_data_t;

static
hkey_t generate_key(int assoc_id, ric_gen_id_t id)
{
  const hkey_t k = {
    .word = {
      ((uint32_t) assoc_id >> 16) & 0xffff,
      assoc_id & 0xffff,
      id.ric_req_id,
      id.ric_inst_id,
      id.ran_func_id
    },
    .len = 5
  };
  return k;
}

void sub_mgmt_init(sub_mgmt_t* sub_mgmt)
{
  assert(sub_mgmt);
  memset(sub_mgmt, 0, sizeof(*sub_mgmt));
  hash_table_init(&sub_mgmt->sub_tab, 8);
  hash_table_init(&sub_mgmt->ctrl_tab, 8);
}

static
void free_htable_entry(hentry_t* e)
{
  free(e->data);
}

void sub_mgmt_free(sub_mgmt_t* sub_mgmt)
{
  assert(sub_mgmt);
  hash_table_destroy(&sub_mgmt->sub_tab, free_htable_entry);
  hash_table_destroy(&sub_mgmt->ctrl_tab, free_htable_entry);
}

size_t sub_mgmt_get_num_sub(const sub_mgmt_t* sub_mgmt)
{
  assert(sub_mgmt);
  return sub_mgmt->sub_tab.filled;
}

void sub_mgmt_sub_add(sub_mgmt_t* sub_mgmt, int assoc_id, const ric_gen_id_t id, cb_sub_t cb, void *data)
{
  assert(sub_mgmt);
  assert(cb.sub_resp && cb.sub_fail && cb.sub_fail);

  sub_data_t* sub_data = malloc(sizeof(*sub_data));
  assert(sub_data);
  sub_data->cb_sub = cb;
  memset(&sub_data->cb_sub_del, 0, sizeof(sub_data->cb_sub_del)); /* no active subscription delete */
  sub_data->user_data = data;

  const hkey_t key = generate_key(assoc_id, id);
  hash_table_insert(&sub_mgmt->sub_tab, key, sub_data);

  /* TODO: set timer and handle the case that no response is received? */
}

void sub_mgmt_sub_add_confirm(sub_mgmt_t* sub_mgmt, int assoc_id, const ric_subscription_response_t* resp)
{
  assert(sub_mgmt);
  assert(resp);

  const hkey_t key = generate_key(assoc_id, resp->ric_id);
  const hentry_t* f = hash_table_find(&sub_mgmt->sub_tab, key);
  assert(f);
  assert(f->data && "no data for this entry found");
  sub_data_t* sub_data = f->data;
  assert(sub_data->cb_sub_del.sub_del_resp == NULL
      && sub_data->cb_sub_del.sub_del_fail == NULL && "there is an ongoing delete request?");
  sub_data->cb_sub.sub_resp(resp, sub_data->user_data);

  /* Confirmed: we don't have anything to do. TODO: if we had a timer running,
   * we would need to reset it here */
}

void sub_mgmt_sub_add_reject(sub_mgmt_t* sub_mgmt, int assoc_id, const ric_subscription_failure_t* fail)
{
  assert(sub_mgmt);
  assert(fail);

  const hkey_t key = generate_key(assoc_id, fail->ric_id);
  hentry_t* e = hash_table_find(&sub_mgmt->sub_tab, key);
  assert(e);

  sub_data_t* sub_data = e->data;
  assert(sub_data->cb_sub_del.sub_del_resp == NULL
      && sub_data->cb_sub_del.sub_del_fail == NULL && "there is an ongoing delete request?");
  sub_data->cb_sub.sub_fail(fail, sub_data->user_data);

  /* clean up */
  free(e->data);
  const bool rem = hash_table_remove(&sub_mgmt->sub_tab, key);
  assert(rem && "no data for this entry found");

  /* TODO: if we had a timer running, we would need to reset it now */
}

void sub_mgmt_indication(sub_mgmt_t *sub_mgmt, int assoc_id, const ric_indication_t *ind)
{
  assert(sub_mgmt);
  assert(ind);

  const hkey_t key = generate_key(assoc_id, ind->ric_id);
  const hentry_t* f = hash_table_find(&sub_mgmt->sub_tab, key);
  assert(f);
  assert(f->data && "no data for this entry found");
  sub_data_t* sub_data = f->data;
  sub_data->cb_sub.ind(ind, sub_data->user_data);
}

void sub_mgmt_sub_del(sub_mgmt_t* sub_mgmt, int assoc_id, ric_gen_id_t id, cb_sub_del_t cb)
{
  assert(sub_mgmt);
  assert(cb.sub_del_resp && cb.sub_del_fail);

  const hkey_t key = generate_key(assoc_id, id);
  const hentry_t* f = hash_table_find(&sub_mgmt->sub_tab, key);
  assert(f);
  assert(f->data && "no data for this entry found");
  sub_data_t* sub_data = f->data;
  assert(sub_data->cb_sub_del.sub_del_resp == NULL
      && sub_data->cb_sub_del.sub_del_fail == NULL && "there is an ongoing delete request?");
  sub_data->cb_sub_del = cb;
}

void sub_mgmt_sub_del_confirm(sub_mgmt_t* sub_mgmt, int assoc_id, const ric_subscription_delete_response_t* resp)
{
  assert(sub_mgmt);
  assert(resp);

  const hkey_t key = generate_key(assoc_id, resp->ric_id);
  const hentry_t* e = hash_table_find(&sub_mgmt->sub_tab, key);

  assert(e->data && "no data for this entry found");
  sub_data_t* sub_data = e->data;
  assert(sub_data->cb_sub_del.sub_del_resp);
  sub_data->cb_sub_del.sub_del_resp(resp, sub_data->user_data);

  /* free allocated memory */
  free(e->data);

  /* remove entry */
  const bool rem = hash_table_remove(&sub_mgmt->sub_tab, key);
  assert(rem && "no data for this entry found");

  /* TODO: if we had a timer running, we would need to reset it now */
}

void sub_mgmt_sub_del_reject(sub_mgmt_t* sub_mgmt, int assoc_id, const ric_subscription_delete_failure_t* fail)
{
  assert(sub_mgmt);
  assert(fail);

  const hkey_t key = generate_key(assoc_id, fail->ric_id);
  const hentry_t* f = hash_table_find(&sub_mgmt->sub_tab, key);
  assert(f);
  assert(f->data && "no data for this entry found");
  sub_data_t* sub_data = f->data;
  assert(sub_data->cb_sub_del.sub_del_resp);
  sub_data->cb_sub_del.sub_del_fail(fail, sub_data->user_data);

  /* TODO: if we had a timer running, we would need to reset it now */
}

size_t sub_mgmt_get_num_ctrl(const sub_mgmt_t* sub_mgmt)
{
  assert(sub_mgmt);
  return sub_mgmt->ctrl_tab.filled;
}

void sub_mgmt_ctrl_req(sub_mgmt_t* sub_mgmt, int assoc_id, ric_gen_id_t id, cb_ctrl_t cb, void* data)
{
  assert(sub_mgmt);
  assert(cb.ctrl_ack && cb.ctrl_fail);

  ctrl_data_t* ctrl_data = malloc(sizeof(*ctrl_data));
  assert(ctrl_data);
  ctrl_data->cb_ctrl = cb;
  ctrl_data->user_data = data;

  const hkey_t key = generate_key(assoc_id, id);
  hentry_t* e = hash_table_insert(&sub_mgmt->ctrl_tab, key, ctrl_data);

  /* it might happen that an entry has not been deleted (RIC Control Ack
   * Request "NAck": Control Ack only required for failure -> it is inserted
   * here, but there was no failure, so no answer). In this case, there is
   * still the old data, and we need to overwrite it */
  ctrl_data_t* old = e->data;
  if (old != ctrl_data) {
    free(old);
    e->data = ctrl_data;
  }

  /* TODO: set timer and handle the case that no response is received? */
}

void sub_mgmt_ctrl_req_ack(sub_mgmt_t* sub_mgmt, int assoc_id, const ric_control_acknowledge_t* ack)
{
  assert(sub_mgmt);
  assert(ack);

  const hkey_t key = generate_key(assoc_id, ack->ric_id);
  hentry_t* e = hash_table_find(&sub_mgmt->ctrl_tab, key);
  assert(e);

  ctrl_data_t* ctrl_data = e->data;
  ctrl_data->cb_ctrl.ctrl_ack(ack, ctrl_data->user_data);

  /* clean up */
  free(e->data);
  const bool rem = hash_table_remove(&sub_mgmt->ctrl_tab, key);
  assert(rem && "no data for this entry found");

  /* TODO: if we had a timer running, we would need to reset it now */
}

void sub_mgmt_ctrl_req_fail(sub_mgmt_t* sub_mgmt, int assoc_id, const ric_control_failure_t* fail)
{
  assert(sub_mgmt);
  assert(fail);

  const hkey_t key = generate_key(assoc_id, fail->ric_id);
  hentry_t* e = hash_table_find(&sub_mgmt->ctrl_tab, key);
  assert(e);

  ctrl_data_t* ctrl_data = e->data;
  ctrl_data->cb_ctrl.ctrl_fail(fail, ctrl_data->user_data);

  /* clean up */
  free(e->data);
  const bool rem = hash_table_remove(&sub_mgmt->ctrl_tab, key);
  assert(rem && "no data for this entry found");

  /* TODO: if we had a timer running, we would need to reset it now */
}
