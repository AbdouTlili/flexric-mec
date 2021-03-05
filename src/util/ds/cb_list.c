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

#include <string.h>
#include <assert.h>

#include "cb_list.h"

void cb_list_create(cb_list_t* lP)
{
  assert(lP);
  assert(lP->cb_list == NULL);
  lP->cb_list = calloc(4, sizeof(*lP->cb_list));
  assert(lP->cb_list);
  lP->siz = 4;
  lP->len = 0;
  lP->iter = -1;
  lP->next_ev_id = 0;
}

void cb_list_destroy(cb_list_t* lP)
{
  assert(lP);
  assert(lP->cb_list);
  free(lP->cb_list);
  lP->cb_list = NULL;
  lP->siz = 0;
  lP->len = 0;
  lP->next_ev_id = 0;
}

uint64_t cb_list_subscribe(cb_list_t* lP, void* cb, void* ud)
{
  assert(lP);
  if (lP->len == lP->siz) {
    lP->cb_list = reallocarray(lP->cb_list, lP->siz * 2, sizeof(*lP->cb_list));
    assert(lP->cb_list);
    lP->siz *= 2;
  }

  lP->cb_list[lP->len++] = (struct cb_data) { .ev_id = lP->next_ev_id, .cb = cb, .d = ud };
  return lP->next_ev_id++;
}

void cb_list_unsubscribe(cb_list_t* lP, uint64_t ev_id)
{
  // TODO: put on "delete" stack
  for (ssize_t i = 0; i < lP->len; ++i) {
    if (ev_id == lP->cb_list[i].ev_id) {
      //void *data = lP->cb_list[i].d;
      memmove(&lP->cb_list[i],
              &lP->cb_list[i+1],
              (lP->len - i - 1) * sizeof(*lP->cb_list));
      /* while iterating the list (as tracked in variable iter): if we remove
       * an element in a position that is before the currently iterated one, we
       * need to also decrease the list index of the iterated items */
      if (i <= lP->iter)
        lP->iter--;
      lP->len--;
      memset(&lP->cb_list[lP->len], 0, sizeof(*lP->cb_list));
      return;
    }
  }
  assert(0 && "callback not found");
}

void cb_list_foreach(cb_list_t* lP, void (*cb_handler)(void* cb, void* signal_data, void *user_data), void *signal_data)
{
  /* iterates complete list of callbacks even if one handler adds/removes an
   * element during iteration */
  for (lP->iter = 0; lP->iter < lP->len; ++lP->iter) {
    struct cb_data* cbd = &lP->cb_list[lP->iter];
    cb_handler(cbd->cb, signal_data, cbd->d);
  }
  lP->iter = -1;
  /* TODO: check stack and remove elements */
}
