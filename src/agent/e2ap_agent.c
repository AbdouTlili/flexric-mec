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

#include "e2ap_agent.h"

#define _GNU_SOURCE
#include <sys/socket.h>

#include <arpa/inet.h>
#include <assert.h>

#include <errno.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h> 

#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>

#include "e2ap_msg_dec_generic.h"
#include "e2ap_msg_enc_generic.h"
#include "e2ap_msg_handler_agent.h"
#include "e2ap_multi_control.h"
#include "e2ap_msg_free.h"
#include "e2ap_endpoint_agent.h"
#include "e2ap_encode.h"

//////////////////
///// epoll //////
//////////////////
// associates timer file descriptors to RAN functions (container) and
// associated data the timer user wishes to receive
typedef struct {
  // the function to call for this timer
  timer_cb_t cb;
  // user-defined data that might be passed
  void* data;
} timer_to_ran_func_t;
timer_to_ran_func_t timer_to_ran_func[1024];

void set_fd_non_blocking(int sfd)
{
  int flags = fcntl (sfd, F_GETFL, 0);  
  flags |= O_NONBLOCK;
  fcntl (sfd, F_SETFL, flags);
}

void add_fd_epoll(int efd, int fd)
{
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

int e2ap_add_timer_epoll_ms_agent(e2ap_agent_t* ag, long initial_ms, long interval_ms, timer_cb_t cb, void* data)
{
  assert(ag != NULL);
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

  add_fd_epoll(ag->efd, tfd);
  const timer_to_ran_func_t ttrf = { .cb = cb, .data = data };
  timer_to_ran_func[tfd] = ttrf;
  return tfd;
}

void e2ap_remove_timer_epoll_agent(int tfd)
{
  assert(tfd > -1);

  int rc = close(tfd);
  assert(rc != -1);
  const timer_to_ran_func_t ttrf = { .cb = NULL, /*.rf_container = NULL,*/ .data = NULL };
  timer_to_ran_func[tfd] = ttrf;
}

/*
void e2ap_stop_timer_epoll_by_subscription_agent(const ric_subscription_request_t* sr)
{
  assert(sr);
  for (size_t i = 0; i < sizeof(timer_to_ran_func)/sizeof(timer_to_ran_func[0]); ++i) {
    if (timer_to_ran_func[i].sub_req == sr) {
      assert(timer_to_ran_func[i].data == NULL && "Don't know how to free data of timer");
      e2ap_remove_timer_epoll_agent(i);
    }
  }
}

static
void e2ap_stop_timer_epoll_by_ran_function_agent(const ran_function_container_t* rfc)
{
  assert(rfc);
  for (size_t i = 0; i < sizeof(timer_to_ran_func)/sizeof(timer_to_ran_func[0]); ++i) {
    if (timer_to_ran_func[i].rf_container == rfc) {
      assert(timer_to_ran_func[i].data == NULL && "Don't know how to free data of timer");
      e2ap_remove_timer_epoll_agent(i);
    }
  }
}
*/

////////////////////////////////////
//// Subscription Management ///////
////////////////////////////////////
static
uint64_t get_ric_subscription_request_id(ep_id_t ep_id, const ric_gen_id_t* id)
{
  return ((uint64_t) ep_id << 32)
       | ((uint64_t) id->ric_req_id << 16)
       | ((uint64_t) id->ric_inst_id);
}

static
int cmp_sub_reg(const void* a_v, const void* b_v)
{
  const pair_sub_data_t* a = (pair_sub_data_t*)a_v;
  const pair_sub_data_t* b = (pair_sub_data_t*)b_v;

  if(a->key < b->key) return 1;
  if(a->key == b->key) return 0;
  return -1;
}

pair_sub_data_t* init_pair_sub_data(uint64_t key, subscription_t* sub)
{
  pair_sub_data_t* ksr = malloc(sizeof(*ksr));
  assert(ksr);
  ksr->key = key;
  ksr->sub = sub;
  return ksr;
}

void free_pair_sub_data(pair_sub_data_t* fr)
{
  assert(fr != NULL);
  free(fr);
}

void free_subscription_request(ric_subscription_request_t* sr)
{
  free_byte_array(sr->event_trigger);
  for (size_t i = 0; i < sr->len_action; ++i) {
    if (sr->action[i].definition) {
      free_byte_array(*sr->action[i].definition);
      free(sr->action[i].definition);
    }
    if(sr->action[i].subseq_action) {
      if(sr->action[i].subseq_action->time_to_wait_ms)
        free(sr->action[i].subseq_action->time_to_wait_ms);
      free(sr->action[i].subseq_action);
    }
  }
  if (sr->action)
    free(sr->action);
}

static
void sta_free_sub_req(void* a_v)
{
  assert(a_v != NULL);
  pair_sub_data_t* a = (pair_sub_data_t*)a_v;

  // use hack to get mutable sub req (which is not modified except for freeing)
  ric_subscription_request_t *sr = (ric_subscription_request_t*)a->sub->req;
  free_subscription_request(sr);
  free(sr);

  free_pair_sub_data(a);
}

ric_subscription_request_t* deep_copy_sub_req(const ric_subscription_request_t* sr)
{
  ric_subscription_request_t* new_sr = calloc(1, sizeof(*new_sr));
  new_sr->ric_id = sr->ric_id;
  new_sr->event_trigger = copy_byte_array(sr->event_trigger);
  new_sr->len_action = sr->len_action;
  new_sr->action = calloc(sr->len_action, sizeof(*new_sr->action));
  for (size_t i = 0; i < new_sr->len_action; ++i) {
    ric_action_t* new_action = &new_sr->action[i];
    const ric_action_t* action = &sr->action[i];
    new_action->id = action->id;
    new_action->type = action->type;
    if (action->definition) {
      new_action->definition = malloc(sizeof(*new_action->definition));
      *new_action->definition = copy_byte_array(*action->definition);
    }
    if (action->subseq_action) {
      new_action->subseq_action = malloc(sizeof(*new_action->subseq_action));
      new_action->subseq_action->type = action->subseq_action->type;
    }
  }
  return new_sr;
}

/*
subscription_t* e2ap_get_sub_data(ran_function_container_t* rfc, uint64_t id)
{
  assert(rfc);
  pair_sub_data_t ksr = {.key = id, .sub = NULL};
  rb_tree_t* tree = &rfc->sub_reqs;
  node_t* node = find_rb_tree(tree, tree->root, (void*)&ksr);
  assert(node != tree->dummy && "Subscription request not found in the Agent");
  const pair_sub_data_t* found = (pair_sub_data_t *)node->key;
  assert(found->key == id);
  return found->sub;
}
*/

void e2ap_register_sub_req_agent(ran_function_container_t* rfc, ep_id_t ep_id, subscription_t* sub)
{
  assert(sub->req);
  uint64_t sr_id = get_ric_subscription_request_id(ep_id, &sub->req->ric_id);
  pair_sub_data_t ksr = {.key = sr_id, .sub = NULL};
  node_t* n = find_rb_tree(&rfc->sub_reqs, rfc->sub_reqs.dummy, &ksr);
  assert(n == rfc->sub_reqs.dummy && "Key/Subscription Request ID already registered");
  // we do not copy the subscription request, since at this point RAN functions
  // might have associated data to this subscription_t* */
  pair_sub_data_t* p = init_pair_sub_data(sr_id, sub);
  n = create_node_rb_tree(&rfc->sub_reqs, p);
  insert_rb_tree(&rfc->sub_reqs, n);
}

subscription_t* e2ap_get_sub_req_agent(ran_function_container_t* rfc, ep_id_t ep_id, ric_gen_id_t ric_id)
{
  assert(rfc);
  uint64_t sr_id = get_ric_subscription_request_id(ep_id, &ric_id);
  pair_sub_data_t ksr = {.key = sr_id, .sub = NULL};
  rb_tree_t* tree = &rfc->sub_reqs;
  node_t* node = find_rb_tree(tree, tree->root, (void*)&ksr);
  if (node == tree->dummy)
    return NULL;
  pair_sub_data_t* found = (pair_sub_data_t *)node->key;
  assert(found->key == sr_id);
  return found->sub;
}

void e2ap_delete_sub_req_agent(ran_function_container_t* rfc, ep_id_t ep_id, ric_gen_id_t ric_id)
{
  assert(rfc);
  uint64_t sr_id = get_ric_subscription_request_id(ep_id, &ric_id);
  pair_sub_data_t ksr = {.key = sr_id, .sub = NULL};
  rb_tree_t* tree = &rfc->sub_reqs;
  node_t* node = find_rb_tree(tree, tree->root, (void*)&ksr);
  assert(node != tree->dummy && "Subscription request not found in the Agent");
  pair_sub_data_t* found = (pair_sub_data_t *)node->key;
  assert(found->key == sr_id);
  delete_rb_tree(&rfc->sub_reqs, node);
  free_pair_sub_data(found);
}

static
int cmp_fun_reg(const void* a_v, const void* b_v)
{
  const pair_ran_fun_t* a = (pair_ran_fun_t*)a_v;
  const pair_ran_fun_t* b = (pair_ran_fun_t*)b_v;

  if(a->key < b->key) return 1;
  if(a->key == b->key) return 0;
  return -1;
}

static
ran_function_t* deep_copy_ran_fun(const ran_function_t* r)
{
  ran_function_t* new_r = calloc(1,sizeof(*new_r));
  new_r->def.buf = malloc(r->def.len);
  memcpy(new_r->def.buf, r->def.buf, r->def.len);
  new_r->def.len = r->def.len;
  new_r->id = r->id;
  new_r->rev = r->rev;
  if(r->oid != NULL){
    new_r->oid = malloc(sizeof(byte_array_t));
    *new_r->oid = copy_byte_array(*r->oid);
  }
  return new_r;
}

pair_ran_fun_t* init_pair_fun_reg(uint64_t key, ran_function_t* r, service_model_cb_t cbs, void *rfdata)
{
  pair_ran_fun_t* fr = malloc(sizeof(pair_ran_fun_t));
//  assert(r != NULL);
  fr->key = key;
  fr->rf_container = calloc(1, sizeof(*fr->rf_container));
  fr->rf_container->r = r;
  fr->rf_container->cbs = cbs;
  fr->rf_container->data = rfdata;
  init_rb_tree(&fr->rf_container->sub_reqs, cmp_sub_reg);
  return fr;
}

void free_fun_reg(pair_ran_fun_t* fr)
{
  assert(fr != NULL);
  free_rb_tree(&fr->rf_container->sub_reqs, sta_free_sub_req);
  free(fr->rf_container);
  free(fr);
}

static
void sta_free_fun_reg(void* a_v)
{
  assert(a_v != NULL);
  pair_ran_fun_t* a = (pair_ran_fun_t*)a_v; 

  /* stop all timers of this RAN function */
  //e2ap_stop_timer_epoll_by_ran_function_agent(a->rf_container);

  // free ran function
  ran_function_t *r = a->rf_container->r;
  free_byte_array(r->def);

  if(r->oid != NULL){
    free_byte_array(*r->oid );
    free(r->oid);
  }
  free(r);

  free_fun_reg(a);
}

static
void rb_tree_copy_fun_ran(void* container_v, void* node_v)
{
  ran_function_t** ran_func = (ran_function_t**)container_v; 
  pair_ran_fun_t* p = (pair_ran_fun_t*)node_v;
  ran_function_t* node = p->rf_container->r;

  const size_t size_ran_func = node->def.len;
  (*ran_func)->def.buf = malloc(size_ran_func);
  memcpy((*ran_func)->def.buf, node->def.buf, size_ran_func);
  (*ran_func)->def.len = size_ran_func;

  (*ran_func)->id = node->id;
  (*ran_func)->rev = node->rev;
  if(node->oid != NULL){
    (*ran_func)->oid = malloc(sizeof(byte_array_t)); 
    *(*ran_func)->oid = copy_byte_array(*node->oid);
  }
  (*ran_func)++;
}

static
void init_handle_msg_agent(e2ap_handle_msg_fp_agent (*handle_msg)[26])
{
  memset((*handle_msg), 0, sizeof(e2ap_handle_msg_fp_agent)*26);

  (*handle_msg)[RIC_SUBSCRIPTION_REQUEST] = e2ap_handle_subscription_request_agent;
  (*handle_msg)[RIC_SUBSCRIPTION_DELETE_REQUEST] =  e2ap_handle_subscription_delete_request_agent;
  (*handle_msg)[RIC_CONTROL_REQUEST] = e2ap_handle_control_request_agent;
  (*handle_msg)[E2AP_ERROR_INDICATION] = e2ap_handle_error_indication_agent;
  (*handle_msg)[E2_SETUP_RESPONSE] =  e2ap_handle_setup_response_agent;
  (*handle_msg)[E2_SETUP_FAILURE] =  e2ap_handle_setup_failure_agent;
  (*handle_msg)[E2AP_RESET_REQUEST] =  e2ap_handle_reset_request_agent;
  (*handle_msg)[E2AP_RESET_RESPONSE] =  e2ap_handle_reset_response_agent;
  (*handle_msg)[RIC_SERVICE_UPDATE_ACKNOWLEDGE] =  e2ap_handle_service_update_ack_agent;
  (*handle_msg)[RIC_SERVICE_UPDATE_FAILURE] =  e2ap_handle_service_update_failure_agent;
  (*handle_msg)[RIC_SERVICE_QUERY] = e2ap_handle_service_query_agent;
  (*handle_msg)[E2_NODE_CONFIGURATION_UPDATE_ACKNOWLEDGE] =  e2ap_handle_node_configuration_update_ack_agent;
  (*handle_msg)[E2_NODE_CONFIGURATION_UPDATE_FAILURE] =  e2ap_handle_node_configuration_update_failure_agent;
  (*handle_msg)[E2_CONNECTION_UPDATE] =  e2ap_handle_connection_update_agent;
}

e2ap_agent_t* e2ap_create_instance_agent(void)
{
  e2ap_agent_t* ag = calloc(1, sizeof(*ag));
  assert(ag);
  ag->cap_ep = 4;
  ag->ep = calloc(ag->cap_ep, sizeof(*ag->ep));
  ag->num_ep = 0;

  ag->efd = init_epoll();
  init_enc(&ag->enc.type);

  init_handle_msg_agent(&ag->handle_msg);

  ag->stop_token = false;
  ag->agent_stopped = false;

  init_rb_tree(&ag->fun_registered, cmp_fun_reg);
  return ag;
}

e2ap_agent_t* e2ap_init_agent(const char* addr, int port, global_e2_node_id_t ge2nid)
{
  assert(addr != NULL);
  assert(port > 0 && port < 65535);

  printf("initializing e2ap agent\n");

  e2ap_agent_t* ag = e2ap_create_instance_agent();
  e2ap_init_ep_agent(&ag->ep[ag->num_ep], addr, port);
  ag->num_ep += 1;

  ag->global_e2_node_id = ge2nid;
  return ag;
}

void e2ap_event_loop_agent(e2ap_agent_t* ag)
{
  const int maxevents = 1;
  struct epoll_event events[maxevents];
  const int timeout_ms = 1000;
  while (ag->stop_token == false) {
    const int events_ready = epoll_wait(ag->efd, events, maxevents, timeout_ms);
    if (events_ready < 0) {
      fprintf(stderr, "epoll_wait() returned -1: errno %d, %s\n", errno, strerror(errno));
      assert(errno == EINTR);
    }
    for(int i =0; i < events_ready; ++i){
      assert((events[i].events & EPOLLERR) == 0);
      ep_id_t ep_id = get_endpoint(ag, events[i].data.fd);
      if (ep_id > -1) {
        e2ap_msg_t msg = e2ap_recv_msg_agent(&ag->ep[ep_id], &ag->enc);
        e2ap_msg_handle_agent(&ag->handle_msg, ag, ep_id, &msg);
        e2ap_msg_free(&ag->enc.type.free_msg, &msg);
      } else {
        uint64_t value;
        read(events[i].data.fd, &value, 8);

        const int fd = events[i].data.fd;
        const timer_to_ran_func_t* ttrf = &timer_to_ran_func[fd];
        //assert(ttrf->rf_container);
        ttrf->cb(ag, fd, ttrf->data);
      }
    }
  }
  ag->agent_stopped = true;
}

void e2ap_start_agent(e2ap_agent_t* ag)
{
  set_fd_non_blocking(ag->efd);
  add_fd_epoll(ag->efd, ag->ep[0].base.fd);

  const size_t len_rf = ag->fun_registered.size;
  ran_function_t* ran_func = calloc(len_rf, sizeof(*ran_func));

  // You need a copy as it after the copy_all won't point to the beginning of the array
  ran_function_t* it = ran_func;
  iterate_rb_tree(&ag->fun_registered, ag->fun_registered.root, &it, rb_tree_copy_fun_ran);

  e2_setup_request_t sr = {
    .id = ag->global_e2_node_id,
    .ran_func_item = ran_func,
    .len_rf = len_rf,
    .comp_conf_update = NULL,
    .len_ccu = 0
  };

  printf("sending e2 setup request\n");
  byte_array_t ba = e2ap_enc_setup_request(&sr, &ag->enc.type);
  e2ap_free_setup_request(&sr); 

  e2ap_send_bytes_agent(&ag->ep[0], ba);
  free_byte_array(ba);

  e2ap_event_loop_agent(ag);
}

void e2ap_free_agent(e2ap_agent_t* ag)
{
  ag->stop_token = true;
  while(ag->agent_stopped == false){
    usleep(1000);
  }
   free_rb_tree(&ag->fun_registered, sta_free_fun_reg);
  for (size_t iep = ag->num_ep - 1; iep > 0; --iep)
    remove_endpoint(ag, iep);
  free(ag->ep);
  free(ag);
}

void e2ap_register_ran_fun(e2ap_agent_t* ag, const ran_function_t* r, service_model_cb_t cbs, void *rfdata)
{
  pair_ran_fun_t prf = { .key = r->id, .rf_container = NULL };
  node_t* n = find_rb_tree(&ag->fun_registered, ag->fun_registered.root, &prf);
  assert(n == ag->fun_registered.dummy && "Key, RAN function ID already registered ");
  ran_function_t* new_r = deep_copy_ran_fun(r);
  pair_ran_fun_t* p = init_pair_fun_reg(r->id, new_r, cbs, rfdata);
  n = create_node_rb_tree(&ag->fun_registered, p);
  assert(ag->fun_registered.size <= (size_t)MAX_NUM_RAN_FUNC_ID);
  insert_rb_tree(&ag->fun_registered, n);
}

ran_function_container_t* e2ap_get_ran_fun_agent(e2ap_agent_t* ag, uint16_t ran_func_id)
{
  assert(ag);
  assert(ran_func_id > 0);
  pair_ran_fun_t fr = {.key = ran_func_id, .rf_container = NULL};
  rb_tree_t* tree = &ag->fun_registered;
  node_t* node = find_rb_tree(tree, tree->root, (void*)&fr);
  assert(node != tree->dummy && "RAN function not found in the Agent");
  const pair_ran_fun_t* found = (pair_ran_fun_t *)node->key;
  assert(found->key == ran_func_id);
  return found->rf_container;
}

void e2ap_send_subscription_response(e2ap_agent_t* ag, ep_id_t ep_id, const ric_subscription_response_t* sr)
{
  assert(ep_id >= 0 && ep_id <= ag->num_ep);
  byte_array_t ba = e2ap_enc_subscription_response(sr, &ag->enc.type);
  e2ap_send_bytes_agent(&ag->ep[ep_id], ba);
  free_byte_array(ba);
}

void e2ap_send_subscription_failure(e2ap_agent_t* ag, ep_id_t ep_id, const ric_subscription_failure_t* sf)
{
  assert(ep_id >= 0 && ep_id <= ag->num_ep);
  byte_array_t ba = e2ap_enc_subscription_failure(sf, &ag->enc.type);
  e2ap_send_bytes_agent(&ag->ep[ep_id], ba);
  free_byte_array(ba);
}

void e2ap_send_indication_agent(e2ap_agent_t* ag, ep_id_t ep_id, const ric_indication_t* indication)
{
  assert(ep_id >= 0 && ep_id <= ag->num_ep);
  byte_array_t ba = e2ap_enc_indication(indication, &ag->enc.type);
  e2ap_send_bytes_agent(&ag->ep[ep_id], ba);
  free_byte_array(ba);
}

void e2ap_send_subscription_delete_response(e2ap_agent_t* ag, ep_id_t ep_id, const ric_subscription_delete_response_t* sdr)
{
  assert(ep_id >= 0 && ep_id <= ag->num_ep);
  byte_array_t ba = e2ap_enc_subscription_delete_response(sdr, &ag->enc.type);
  e2ap_send_bytes_agent(&ag->ep[ep_id], ba);
  free_byte_array(ba);
}

void e2ap_send_subscription_delete_failure(e2ap_agent_t* ag, ep_id_t ep_id, const ric_subscription_delete_failure_t* sdf)
{
  assert(ep_id >= 0 && ep_id <= ag->num_ep);
  byte_array_t ba = e2ap_enc_subscription_delete_failure(sdf, &ag->enc.type);
  e2ap_send_bytes_agent(&ag->ep[ep_id], ba);
  free_byte_array(ba);
}

void e2ap_send_control_acknowledge(e2ap_agent_t* ag, ep_id_t ep_id, const ric_control_acknowledge_t* ca)
{
  assert(ep_id >= 0 && ep_id <= ag->num_ep);
  byte_array_t ba = e2ap_enc_control_acknowledge(ca, &ag->enc.type);
  e2ap_send_bytes_agent(&ag->ep[ep_id], ba);
  free_byte_array(ba);
}

void e2ap_send_control_failure(e2ap_agent_t* ag, ep_id_t ep_id, const ric_control_failure_t* cf)
{
  assert(ep_id >= 0 && ep_id <= ag->num_ep);
  byte_array_t ba = e2ap_enc_control_failure(cf, &ag->enc.type);
  e2ap_send_bytes_agent(&ag->ep[ep_id], ba);
  free_byte_array(ba);
}

subscription_t* e2ap_find_subscription(e2ap_agent_t* ag, ep_id_t ep_id, ric_gen_id_t id)
{
  assert(ag);
  ran_function_container_t* rf_container = e2ap_get_ran_fun_agent(ag, id.ran_func_id);
  assert(rf_container);
  if (!rf_container)
    return NULL;
  return e2ap_get_sub_req_agent(rf_container, ep_id, id);
}

void e2ap_delete_subscription(e2ap_agent_t* ag, ep_id_t ep_id, ric_gen_id_t id)
{
  assert(ag);
  ran_function_container_t* rfc = e2ap_get_ran_fun_agent(ag, id.ran_func_id);
  assert(rfc);
  if (!rfc)
    return;
  subscription_t* sub = e2ap_get_sub_req_agent(rfc, ep_id, id);
  assert(sub);
  if (!sub)
    return;
  ric_subscription_request_t* sr = (ric_subscription_request_t*) sub->req;
  free_subscription_request(sr);
  free(sr);
  free(sub);
  e2ap_delete_sub_req_agent(rfc, ep_id, id);
}
