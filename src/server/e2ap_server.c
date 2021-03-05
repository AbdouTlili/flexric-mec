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

#include "e2ap_server.h"
#include "e2ap_msg_handler_ric.h"
#include "e2ap_msg_free.h"
#include "type_defs.h"
#include "e2ap_recv_msg.h"
#include "e2ap_msg_dec_generic.h"
#include "e2ap_msg_enc_generic.h"
#include "events_internal.h"

#include <assert.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <stdio.h>

//////////////////
///// epoll //////
//////////////////
// associates timer file descriptors to RAN functions (container) and
// associated data the timer user wishes to receive
typedef struct {
  // the function to call for this timer
  ric_timer_cb_t cb;
  // user-defined data that might be passed
  void* data;
} timer_to_cb_t;
timer_to_cb_t timer_to_cb[1024];

static
void set_fd_non_blocking(int sfd)
{
  int flags = fcntl (sfd, F_GETFL, 0);  
  flags |= O_NONBLOCK;
  fcntl (sfd, F_SETFL, flags);
}

static
void add_fd_epoll(int efd, int fd)
{
  set_fd_non_blocking(efd);
  const int op = EPOLL_CTL_ADD;
  const epoll_data_t e_data = {.fd = fd};
  const int e_events = EPOLLIN; // open for reading
  struct epoll_event event = {.events = e_events, .data = e_data};
  int rc = epoll_ctl(efd, op, fd, &event);
  assert(rc != -1);
}

static
int init_epoll()
{
  const int flags = EPOLL_CLOEXEC; 
  const int efd = epoll_create1(flags);  
  assert(efd != -1);
  return efd;
}

int e2ap_add_timer_epoll_ms_server(e2ap_ric_t* ric, long initial_ms, long interval_ms, ric_timer_cb_t cb, void* data)
{
  assert(ric != NULL);
  assert(initial_ms > -1);
  assert(interval_ms > -1);

  // Create the timer
  const int clockid = CLOCK_MONOTONIC;
  const int flags = TFD_NONBLOCK | TFD_CLOEXEC;
  const int tfd = timerfd_create(clockid, flags);
  assert(tfd != -1);
  // we currently use a dumb array to associate a timer with a fptr/data
  assert(tfd < 1024);

  const time_t initial_sec = initial_ms / 1000;
  const long initial_nsec = (initial_ms * 1000000) % 1000000000;
  /* Initial expiration */
  const struct timespec it_value = {.tv_sec = initial_sec, .tv_nsec = initial_nsec};

  const time_t interval_sec = interval_ms / 1000;
  const long interval_nsec = (interval_ms * 1000000) % 1000000000;
  /* Interval for periodic timer */
  const struct timespec it_interval = {.tv_sec = interval_sec, .tv_nsec = interval_nsec};

  const int flags_2 = 0;
  struct itimerspec *old_value = NULL; // not interested in how the timer was previously configured
  const struct itimerspec new_value = {.it_interval = it_interval, .it_value = it_value};
  int rc = timerfd_settime(tfd, flags_2, &new_value, old_value);
  assert(rc != -1);

  add_fd_epoll(ric->efd, tfd);
  const timer_to_cb_t ttcb = { .cb = cb, .data = data };
  timer_to_cb[tfd] = ttcb;
  return tfd;
}

void e2ap_remove_timer_epoll_server(int tfd)
{
  assert(tfd > -1);

  int rc = close(tfd);
  assert(rc != -1);
  const timer_to_cb_t ttcb = { .cb = NULL, /*.rf_container = NULL,*/ .data = NULL };
  timer_to_cb[tfd] = ttcb;
}

static
void init_handle_msg_ric(e2ap_handle_msg_fp_ric (*handle_msg)[26])
{
  memset((*handle_msg), 0, sizeof(e2ap_handle_msg_fp_ric)*26);
  (*handle_msg)[RIC_SUBSCRIPTION_RESPONSE] =  e2ap_handle_subscription_response_ric;
  (*handle_msg)[RIC_SUBSCRIPTION_FAILURE] =  e2ap_handle_subscription_failure_ric;
  (*handle_msg)[RIC_SUBSCRIPTION_DELETE_RESPONSE] =  e2ap_handle_subscription_delete_response_ric;
  (*handle_msg)[RIC_SUBSCRIPTION_DELETE_FAILURE] =  e2ap_handle_subscription_delete_failure_ric;
  (*handle_msg)[RIC_INDICATION] =  e2ap_handle_indication_ric;
  (*handle_msg)[RIC_CONTROL_ACKNOWLEDGE] =  e2ap_handle_control_ack_ric;
  (*handle_msg)[RIC_CONTROL_FAILURE] =  e2ap_handle_control_failure_ric;
  (*handle_msg)[E2AP_ERROR_INDICATION] =  e2ap_handle_error_indication_ric;
  (*handle_msg)[E2_SETUP_REQUEST] =  e2ap_handle_setup_request_ric;
  (*handle_msg)[E2AP_RESET_REQUEST] =  e2ap_handle_reset_request_ric;
  (*handle_msg)[E2AP_RESET_RESPONSE] =  e2ap_handle_reset_response_ric;
  (*handle_msg)[RIC_SERVICE_UPDATE] =  e2ap_handle_service_update_ric;
  (*handle_msg)[E2_NODE_CONFIGURATION_UPDATE] =  e2ap_handle_node_configuration_update_ric;
  (*handle_msg)[E2_CONNECTION_UPDATE_ACKNOWLEDGE] =  e2ap_handle_connection_update_ack_ric;
  (*handle_msg)[E2_CONNECTION_UPDATE_FAILURE] =  e2ap_handle_connection_update_failure_ric;
}

void e2ap_init_server(e2ap_ric_t* ric, const char* addr, int port)
{
  assert(ric != NULL);
  assert(addr != NULL);
  assert(port > 0 && port < 65535);

  e2ap_init_ep_ric(&ric->ep, addr, port);
  ric->efd = init_epoll();
  init_enc(&ric->enc.type);

  ran_mgmt_init(&ric->ran_mgmt);
  sub_mgmt_init(&ric->sub_mgmt);
  events_init(&ric->events);

  init_handle_msg_ric(&ric->handle_msg);

  ric->stop_token = false;
  ric->server_stopped = false;
  printf("Init server done\n");
  fflush(stdout);
}

void e2ap_free_server(e2ap_ric_t* ric)
{
  e2ap_stop_server(ric);
  ran_mgmt_free(&ric->ran_mgmt);
  sub_mgmt_free(&ric->sub_mgmt);
  events_reset(&ric->events);
}

void e2ap_start_server(e2ap_ric_t* ric)
{
  assert(ric != NULL);

  add_fd_epoll(ric->efd, ric->ep.base.fd);

  const int maxevents = 1;
  struct epoll_event events[maxevents];
  const int timeout_ms = 1000;

  while(ric->stop_token == false){ 
    const int events_ready = epoll_wait(ric->efd, events, maxevents, timeout_ms); 
    if (events_ready < 0) {
      fprintf(stderr, "epoll_wait(): error %d, %s\n", errno, strerror(errno));
      assert(errno == EINTR);
    }
    for(int i = 0; i < events_ready; ++i){
      assert((events[i].events & EPOLLERR) == 0);
      if (events[i].data.fd == ric->ep.base.fd) {
        e2ap_msg_t msg = e2ap_recv_msg_ric(&ric->ep, &ric->enc);
        e2ap_msg_handle_ric(&ric->handle_msg, ric, &msg);
        //printf("handled message type %d\n", msg.type);
        e2ap_msg_free(&ric->enc.type.free_msg, &msg);
      } else {
        uint64_t value;
        read(events[i].data.fd, &value, 8);

        const int fd = events[i].data.fd;
        const timer_to_cb_t* ttcb = &timer_to_cb[fd];
        ttcb->cb(ric, fd, ttcb->data);
      }
    } 
  }
  ric->server_stopped = true; 
}

void e2ap_stop_server(e2ap_ric_t* ric)
{
  assert(ric != NULL);
  ric->stop_token = true;
  while(ric->server_stopped == false){
    sleep(1);
  }
}

void e2ap_subscribe(e2ap_ric_t* ric, int assoc_id, const ric_subscription_request_t* req, cb_sub_t cb, void* data)
{
  assert(ric);
  assert(req);

  /* I don't know how to mux to different associations. For the time being,
   * just check that the association is what we use internally */
  assert(assoc_id == ric->ep.base.sri.sinfo_assoc_id);

  sub_mgmt_sub_add(&ric->sub_mgmt, assoc_id, req->ric_id, cb, data);

  byte_array_t ba2 = e2ap_enc_subscription_request(req, &ric->enc.type);
  e2ap_send_bytes_ric(&ric->ep, ba2);
  free_byte_array(ba2);
}

void e2ap_unsubscribe(e2ap_ric_t* ric, int assoc_id, const ric_subscription_delete_request_t* del, cb_sub_del_t cb)
{
  assert(ric);
  assert(del);

  /* I don't know how to mux to different associations. For the time being,
   * just check that the association is what we use internally */
  assert(assoc_id == ric->ep.base.sri.sinfo_assoc_id);

  sub_mgmt_sub_del(&ric->sub_mgmt, assoc_id, del->ric_id, cb);

  byte_array_t ba2 = e2ap_enc_subscription_delete_request(del, &ric->enc.type);
  e2ap_send_bytes_ric(&ric->ep, ba2);
  free_byte_array(ba2);
}

void e2ap_control_req(e2ap_ric_t* ric, int assoc_id, const ric_control_request_t* req, cb_ctrl_t cb, void* data)
{
  assert(ric);
  assert(req);

  sub_mgmt_ctrl_req(&ric->sub_mgmt, assoc_id, req->ric_id, cb, data);

  byte_array_t ba2 = e2ap_enc_control_request(req, &ric->enc.type);
  e2ap_send_bytes_ric(&ric->ep, ba2);
  free_byte_array(ba2);
}

