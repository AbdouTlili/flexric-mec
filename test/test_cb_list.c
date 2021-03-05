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

#include <stdint.h>
#include <assert.h>
#include <string.h>
#include "cb_list.h"

typedef void (*test_cb)(int*, int*);
void handle_cb(void* cb, void* sd, void* ud) {
  ((test_cb) cb)((int*)sd, (int*)ud);
}

void test_passed_data_correct()
{
  cb_list_t cbl;
  memset(&cbl, 0, sizeof(cbl));
  cb_list_create(&cbl);
  int signal_data = 12;
  int user_data = 1;
  void cb(int* sd, int* ud) {
    assert(*sd == signal_data);
    assert(*ud == user_data);
  }
  (void) cb_list_subscribe(&cbl, cb, &user_data);
  cb_list_foreach(&cbl, handle_cb, &signal_data);
  cb_list_destroy(&cbl);
}

void test_remove_cb_during_iter()
{
  cb_list_t cbl;
  memset(&cbl, 0, sizeof(cbl));
  cb_list_create(&cbl);
  int calls = 0;
  uint64_t ev_id = 0xfff;
  void cb(int* sd, int* ud) {
    (void) sd;
    (void) ud;
    cb_list_unsubscribe(&cbl, ev_id);
    calls++;
  }
  ev_id = cb_list_subscribe(&cbl, cb, NULL);
  cb_list_foreach(&cbl, handle_cb, NULL);
  cb_list_foreach(&cbl, handle_cb, NULL);
  assert(calls == 1);
  cb_list_destroy(&cbl);
}

void test_add_cb_during_iter()
{
  cb_list_t cbl;
  memset(&cbl, 0, sizeof(cbl));
  cb_list_create(&cbl);
  int calls2 = 0;
  void cb2(int* sd, int* ud) {
    (void) sd;
    (void) ud;
    calls2++;
  }
  int calls1 = 0;
  void cb1(int* sd, int* ud) {
    (void) sd;
    (void) ud;
    if (calls1 == 0) /* subscribe once */
      (void) cb_list_subscribe(&cbl, cb2, NULL);
    calls1++;
  }

  (void) cb_list_subscribe(&cbl, cb1, NULL);
  cb_list_foreach(&cbl, handle_cb, NULL);
  cb_list_foreach(&cbl, handle_cb, NULL);
  assert(calls1 == 2);
  assert(calls2 == 2);
  cb_list_destroy(&cbl);
}

int main()
{
  test_passed_data_correct();
  test_remove_cb_during_iter();
  test_add_cb_during_iter();
}
