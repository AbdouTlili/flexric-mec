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

#include "events.h"
#include "events_internal.h"
#include <assert.h>
#include "cb_list.h"

/***********************************************************
 ***************** E2 Node Connect event *******************
 ***********************************************************/
ed_t subscribe_e2node_connect(events_t* ev, e2node_connect_cb_t cb, void* data)
{
  assert(ev);
  return (ed_t) {
    .ev_id = cb_list_subscribe(&ev->e2node_connect_cb_list, (void*) cb, data),
    .type = E2_NODE_CONNECT
  };
}

void trigger_e2node_connect(events_t* ev, const ran_t* ran, int assoc_id)
{
  assert(ev);
  void handle_e2node_connect(void* cb, void* signal_data, void* user_data) {
    e2node_connect_cb_t real_cb = (e2node_connect_cb_t) cb;
    const e2node_event_t *sd = (e2node_event_t *) signal_data;
    real_cb(sd, user_data);
  }
  e2node_event_t e = { .ran = ran, .assoc_id = assoc_id };
  cb_list_foreach(&ev->e2node_connect_cb_list, handle_e2node_connect, &e);
}

/***********************************************************
 *************** E2 Node Disconnect event ******************
 ***********************************************************/
/*
ed_t subscribe_e2node_disconnect(events_t* ev, e2node_disconnect_cb_t cb, void* data)
{
  assert(ev);
  return (ed_t) {
    .ev_id = cb_list_subscribe(&ev->e2node_disconnect_cb_list, (void*) cb, data),
    .type = E2_NODE_DISCONNECT
  };
}

void trigger_e2node_disconnect(events_t* ev, const ran_t* ran, int assoc_id)
{
  assert(ev);
  void handle_e2node_disconnect(void* cb, void* signal_data, void* user_data) {
    e2node_disconnect_cb_t real_cb = (e2node_disconnect_cb_t) cb;
    const e2node_event_t* sd = (e2node_event_t *) signal_data;
    real_cb(sd, user_data);
  }
  e2node_event_t ev = { .ran = ran, .assoc_id = assoc_id };
  cb_list_foreach(&ev->e2node_disconnect_cb_list, handle_e2node_disconnect, &ev);
}
*/

/***********************************************************
 ******************* RAN Connect event *********************
 ***********************************************************/
ed_t subscribe_ran_connect(events_t* ev, ran_connect_cb_t cb, void* data)
{
  assert(ev);
  return (ed_t) {
    .ev_id = cb_list_subscribe(&ev->ran_connect_cb_list, (void*) cb, data),
    .type = RAN_CONNECT
  };
}

void trigger_ran_connect(events_t* ev, const ran_t* ran)
{
  assert(ev);
  void handle_ran_connect(void* cb, void* signal_data, void* user_data) {
    ran_connect_cb_t real_cb = (ran_connect_cb_t) cb;
    const ran_t *sd = (const ran_t *) signal_data;
    real_cb(sd, user_data);
  }
  cb_list_foreach(&ev->ran_connect_cb_list, handle_ran_connect, (void*)ran);
}

/***********************************************************
 ***************** RAN Disconnect event ********************
 ***********************************************************/
/*
ed_t subscribe_ran_disconnect(events_t* ev, ran_disconnect_cb_t cb, void* data)
{
  assert(ev);
  return (ed_t) {
    .ev_id = cb_list_subscribe(&ev->ran_disconnect_cb_list, (void*) cb, data),
    .type = RAN_DISCONNECT
  };
}

void trigger_ran_disconnect(events_t* ev, const ran_t* ran)
{
  assert(ev);
  void handle_ran_disconnect(void* cb, void* signal_data, void* user_data) {
    ran_disconnect_cb_t real_cb = (ran_disconnect_cb_t) cb;
    const ran_t* sd = (const ran_t *) signal_data;
    real_cb(sd, user_data);
  }
  cb_list_foreach(&ev->ran_disconnect_cb_list, handle_ran_disconnect, &ran);
}
*/

void unsubscribe(events_t* ev, ed_t ed)
{
  assert(ev);
  if (ed.type == E2_NODE_CONNECT)
    cb_list_unsubscribe(&ev->e2node_connect_cb_list, ed.ev_id);
  //else if (ed.type == E2_NODE_DISCONNECT)
  //  cb_list_unsubscribe(&ev->e2node_disconnect_cb_list, ed.ev_id);
  else if (ed.type == RAN_CONNECT)
    cb_list_unsubscribe(&ev->ran_connect_cb_list, ed.ev_id);
  //else if (ed.type == RAN_DISCONNECT)
  //  cb_list_unsubscribe(&ev->ran_disconnect_cb_list, ed.ev_id);
  else
    assert(0!=0 && "Bad ed value...");
}

void events_init(events_t* ev)
{
  assert(ev);
  cb_list_create(&ev->e2node_connect_cb_list);
  //cb_list_create(&ev->e2node_disconnect_cb_list);
  cb_list_create(&ev->ran_connect_cb_list);
  //cb_list_create(&ev->ran_disconnect_cb_list);
}

void events_reset(events_t* ev)
{
  assert(ev);
  cb_list_destroy(&ev->e2node_connect_cb_list);
  //cb_list_destroy(&ev->e2node_disconnect_cb_list);
  cb_list_destroy(&ev->ran_connect_cb_list);
  //cb_list_destroy(&ev->ran_disconnect_cb_list);
}
