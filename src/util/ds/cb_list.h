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

#ifndef CB_LIST_H
#define CB_LIST_H

#include <stdlib.h>
#include <stdint.h>

/* TODO: this is a (very) poor man's ticket map. Ideally, we replace it with a
 * real ticket map. See issue
 * https://gitlab.eurecom.fr/schmidtr/flexric-agent/-/issues/7 */

typedef struct cb_list_s {
  struct cb_data {
    uint64_t ev_id;
    void* cb;
    void* d;
  } *cb_list;
  ssize_t len, siz, iter;
  uint64_t next_ev_id;
} cb_list_t;

/* create a list of callbacks */
void cb_list_create(cb_list_t* lP);
/* destroy a list of callbacks */
void cb_list_destroy(cb_list_t* lP);
/* subscribe a new callback with user_data, returns a token to be passed in
 * cb_list_unsubscribe() */
uint64_t cb_list_subscribe(cb_list_t* lP, void* cb, void* ud);
/* unsubscribe an already subscribed callback using the token returned by
 * cb_list_subscribe(). */
void cb_list_unsubscribe(cb_list_t* lP, uint64_t ev_id);
/* iterates over the callbacks. For each callback, calls the function pointer
 * cb_handler to which is passed the callback, the signal data passed to
 * cb_list_foreach(), and the user_data that has been associated to the
 * callback. It is safe to add or remove callbacks during cb_list_foreach() */
void cb_list_foreach(cb_list_t* lP, void (*cb_handler)(void* cb, void* signal_data, void *user_data), void *signal_data);

#endif /* CB_LIST_H */
