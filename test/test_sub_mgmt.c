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
#include <stdint.h>
#include "sub_mgmt.h"

void test_sub_ind(void)
{
  const int assoc_id = 1337;
  const ric_gen_id_t ric_id = { .ric_req_id = 1, .ric_inst_id = 2, .ran_func_id = 3 };

  int some_data = 12;

  int sub_resp_count = 0;
  const ric_subscription_response_t rsr = { .ric_id = ric_id };
  void sub_resp_cb(const ric_subscription_response_t *r, void *data) {
    assert(data == &some_data);
    assert(r == &rsr);
    sub_resp_count++;
  }

  //const ric_subscription_failure_t rsf = { .ric_id = ric_id };
  void sub_fail_cb(const ric_subscription_failure_t *r, void *data) {
    assert(data == &some_data);
    assert(r);
    //assert(r == &rsf);
    assert(0 && "this function should not be called");
  }

  int ind_count = 0;
  const ric_indication_t ind = { .ric_id = ric_id };
  void ind_cb(const ric_indication_t *i, void* data) {
    assert(data == &some_data);
    assert(i == &ind);
    ind_count++;
  }

  sub_mgmt_t sub_mgmt;
  sub_mgmt_init(&sub_mgmt);

  cb_sub_t cb = {
    .sub_resp = sub_resp_cb,
    .sub_fail = sub_fail_cb,
    .ind = ind_cb
  };
  sub_mgmt_sub_add(&sub_mgmt, assoc_id, ric_id, cb, &some_data);
  sub_mgmt_sub_add_confirm(&sub_mgmt, assoc_id, &rsr);
  sub_mgmt_indication(&sub_mgmt, assoc_id, &ind);
  sub_mgmt_indication(&sub_mgmt, assoc_id, &ind);
  sub_mgmt_indication(&sub_mgmt, assoc_id, &ind);
  assert(sub_mgmt_get_num_sub(&sub_mgmt) == 1);

  int sub_del_resp_count = 0;
  const ric_subscription_delete_response_t rsdr = { .ric_id = ric_id };
  void sub_del_resp_cb(const ric_subscription_delete_response_t *r, void *data) {
    assert(data == &some_data);
    assert(r == &rsdr);
    sub_del_resp_count++;
  }

  void sub_del_fail_cb(const ric_subscription_delete_failure_t *r, void *data) {
    assert(data == &some_data);
    assert(r);
    assert(0 && "this function should not be called");
  }

  cb_sub_del_t cb_del = {
    .sub_del_resp = sub_del_resp_cb,
    .sub_del_fail = sub_del_fail_cb
  };
  sub_mgmt_sub_del(&sub_mgmt, assoc_id, ric_id, cb_del);
  sub_mgmt_sub_del_confirm(&sub_mgmt, assoc_id, &rsdr);
  // sub_mgmt uses the hsearch hash map which has no "delete" functionality
  //assert(sub_mgmt_get_num_sub(&sub_mgmt) == 0);

  assert(sub_resp_count == 1);
  assert(ind_count == 3);
  assert(sub_del_resp_count == 1);

  sub_mgmt_free(&sub_mgmt);
}

void test_sub_ctrl(void)
{
  const int assoc_id = 1337;
  const ric_gen_id_t ric_id = { .ric_req_id = 1, .ric_inst_id = 2, .ran_func_id = 3 };

  int some_data = 12;

  int ctrl_ack_count = 0;
  const ric_control_acknowledge_t rca = { .ric_id = ric_id };
  void ctrl_ack_cb(const ric_control_acknowledge_t* a, void *data) {
    assert(data == &some_data);
    assert(a == &rca);
    ctrl_ack_count++;
  }

  int ctrl_fail_count = 0;
  const ric_control_failure_t rcf = { .ric_id = ric_id };
  void ctrl_fail_cb(const ric_control_failure_t* f, void* data) {
    assert(data == &some_data);
    assert(f == &rcf);
    ctrl_fail_count++;
  }

  sub_mgmt_t sub_mgmt;
  sub_mgmt_init(&sub_mgmt);

  cb_ctrl_t cb = {
    .ctrl_ack = ctrl_ack_cb,
    .ctrl_fail = ctrl_fail_cb
  };

  sub_mgmt_ctrl_req(&sub_mgmt, assoc_id, ric_id, cb, &some_data);
  assert(sub_mgmt_get_num_ctrl(&sub_mgmt) == 1);
  sub_mgmt_ctrl_req_ack(&sub_mgmt, assoc_id, &rca);
  assert(sub_mgmt_get_num_ctrl(&sub_mgmt) == 0);

  sub_mgmt_ctrl_req(&sub_mgmt, assoc_id, ric_id, cb, &some_data);
  assert(sub_mgmt_get_num_ctrl(&sub_mgmt) == 1);
  sub_mgmt_ctrl_req_fail(&sub_mgmt, assoc_id, &rcf);
  assert(sub_mgmt_get_num_ctrl(&sub_mgmt) == 0);

  sub_mgmt_ctrl_req(&sub_mgmt, assoc_id, ric_id, cb, &some_data);
  assert(sub_mgmt_get_num_ctrl(&sub_mgmt) == 1);
  /* no ack nor fail! */

  sub_mgmt_ctrl_req(&sub_mgmt, assoc_id, ric_id, cb, &some_data);
  assert(sub_mgmt_get_num_ctrl(&sub_mgmt) == 1);
  sub_mgmt_ctrl_req_fail(&sub_mgmt, assoc_id, &rcf);
  assert(sub_mgmt_get_num_ctrl(&sub_mgmt) == 0);

  assert(ctrl_ack_count == 1);
  assert(ctrl_fail_count == 2);

  sub_mgmt_free(&sub_mgmt);
}

int main()
{
  test_sub_ind();
  test_sub_ctrl();
}
