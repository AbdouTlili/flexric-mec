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

#include "flexric_agent.h"
#include "e2ap_server.h"
#include "type_defs.h"
#include "events.h"

#include <assert.h>
#include <stdio.h>

#include <pthread.h>

static
pthread_t t_server;

static
pthread_t t_xapps;

static
pthread_t t_agent_stop;

static
pthread_t t_unsubscribe;

static
ran_t* g_ran;

typedef struct data
{
  char* server_ip_str;
  int server_port; 
  e2ap_ric_t* ric;
} data_t;

void print_indication(const ric_indication_t *i, void* data)
{
  assert(i);
  assert(!data);
  printf("[Ind Header  ID %2d/%2d] %.*s\n",
         i->ric_id.ric_req_id, i->ric_id.ric_inst_id,
         (int) i->hdr.len, (char*)i->hdr.buf);
  printf("[Ind Message ID %2d/%2d] %.*s\n",
         i->ric_id.ric_req_id, i->ric_id.ric_inst_id,
         (int) i->msg.len, (char*)i->msg.buf);
}

void print_control_ack(const ric_control_acknowledge_t* a, void* data)
{
  assert(a);
  assert(!data);
  printf("[Ctrl Ack ID %2d/%2d] Control msg ack received with status %s\n",
         a->ric_id.ric_req_id, a->ric_id.ric_inst_id,
         a->status == RIC_CONTROL_STATUS_SUCCESS
           ? "SUCCESS"
           : (a->status == RIC_CONTROL_STATUS_REJECTED ? "REJECTED" : "FAILED"));
}

void sub_resp_cb(const ric_subscription_response_t *r, void *data)
{
  assert(r);
  assert(!data);
  printf("[RIC Sub Resp Message ID %2d/%2d]\n", r->ric_id.ric_req_id, r->ric_id.ric_inst_id);
}

void sub_fail_cb(const ric_subscription_failure_t* f, void *data)
{
  assert(f);
  assert(!data);
  assert(0 && "unexpected subscription failure received");
}

void sub_del_resp_cb(const ric_subscription_delete_response_t *r, void *data)
{
  assert(r);
  assert(!data);
  printf("[RIC Sub Del Resp Message ID %2d/%2d]\n", r->ric_id.ric_req_id, r->ric_id.ric_inst_id);
}

void sub_del_fail_cb(const ric_subscription_delete_failure_t* f, void *data)
{
  assert(f);
  assert(!data);
  assert(0 && "unexpected subscription delete failure received");
}

void ctrl_fail_cb(const ric_control_failure_t* f, void* data) {
  assert(f);
  assert(!data);
  assert(0 && "unexpected control failure received");
}

void subscribe_first_req(const ran_t* ran, void *data)
{
  data_t* d = (data_t*)data;
  assert(d);
  assert(ran);
  g_ran = (ran_t*)ran;

  assert(ran->entities.split == SPLIT_NONE && "expected full base station");
  char *ran_name = generate_ran_name(ran->generated_ran_key);
  printf("%s(): sending subscription after RAN %s connected\n", __func__, ran_name);
  free(ran_name);

  cb_sub_t cb = {
    .sub_resp = sub_resp_cb,
    .sub_fail = sub_fail_cb,
    .ind = print_indication
  };

  ric_gen_id_t r = {.ric_req_id = 1, .ric_inst_id = 1, .ran_func_id = 1};
  const char* c = "10ms";
  BYTE_ARRAY_STACK(ba, strlen(c));
  memcpy(ba_BuF, c, strlen(c));
  ric_action_t action = { .id = 42, .type = RIC_ACT_REPORT, .definition = NULL, .subseq_action = NULL };
  ric_subscription_request_t sr = {
   .ric_id = r,
   .event_trigger = ba,
   .action = &action,
   .len_action = 1,
  };

  int assoc_id = ran->entities.nosplit.full->assoc_id;
  e2ap_subscribe(d->ric, assoc_id, &sr, cb, NULL);
}

void* unsubscribe_first_req(void* arg)
{
  data_t* d = (data_t*)arg;
  assert(d);
  sleep(2);

  assert(g_ran);
  assert(g_ran->entities.split == SPLIT_NONE && "expected full base station");
  const ric_gen_id_t r = {.ric_req_id = 1, .ric_inst_id = 1, .ran_func_id = 1};
  const ric_subscription_delete_request_t sr = { .ric_id = r, };
  int assoc_id = g_ran->entities.nosplit.full->assoc_id;
  cb_sub_del_t cb = {
    .sub_del_resp = sub_del_resp_cb,
    .sub_del_fail = sub_del_fail_cb,
  };
  e2ap_unsubscribe(d->ric, assoc_id, &sr, cb);
  return NULL;
}

void* static_sctp_start_server(void* arg)
{
  data_t* d = (data_t*)arg;
  e2ap_init_server(d->ric, d->server_ip_str, d->server_port);
  ed_t ed = subscribe_ran_connect(&d->ric->events, subscribe_first_req, d);
  (void) ed;
  int rc = pthread_create(&t_unsubscribe, NULL, unsubscribe_first_req, arg);
  assert(rc == 0);
  e2ap_start_server(d->ric);
  e2ap_free_server(d->ric);
  return NULL;
}

void* static_subscribe_to_server(void* arg)
{
  data_t* d = (data_t*)arg;
  assert(d && d->ric);
  usleep(500000);

  /* get all RANs */
  ran_t* ran;
  const ssize_t num_rans = ran_mgmt_get_rans(&d->ric->ran_mgmt, &ran, 1);
  assert(num_rans != 0 && "test: RAN is not connected yet?");
  assert(num_rans == 1 && "test: how can there be more than one RAN?");
  assert(g_ran->entities.split == SPLIT_NONE && "expected full base station");

  ric_gen_id_t r = {.ric_req_id = 1, .ric_inst_id = 2, .ran_func_id = 1};
  const char* c = "10ms";
  byte_array_t ba = { .buf = (uint8_t*) c, .len = strlen(c) };
  ric_action_t action = { .id = 42, .type = RIC_ACT_REPORT, .definition = NULL, .subseq_action = NULL };
  ric_subscription_request_t sr = {
   .ric_id = r,
   .event_trigger = ba,
   .action = &action,
   .len_action = 1,
  };

  cb_sub_t cb = {
    .sub_resp = sub_resp_cb,
    .sub_fail = sub_fail_cb,
    .ind = print_indication
  };

  int assoc_id = ran->entities.nosplit.full->assoc_id;
  e2ap_subscribe(d->ric, assoc_id, &sr, cb, NULL);
  sleep(1);

  ric_gen_id_t rc = { .ric_req_id = 3,  .ric_inst_id = 3, .ran_func_id=1 };
  const char* hdr_str = "Header string";
  byte_array_t hdr = {.buf = (uint8_t*) hdr_str, .len = strlen(hdr_str) };
  const char* msg_str = "Message string";
  byte_array_t msg = {.buf = (uint8_t*) msg_str, .len = strlen(msg_str) } ;
  //ric_control_ack_req_t* ack_req = malloc(sizeof(ric_control_ack_req_t)); // optional
  //*ack_req = RIC_CONTROL_REQUEST_ACK;

  ric_control_request_t ric_req = {
    .ric_id = rc,
    .call_process_id = NULL,
    .hdr = hdr,
    .msg = msg,
    .ack_req = NULL
  };
  cb_ctrl_t cbr = {
    .ctrl_ack = print_control_ack,
    .ctrl_fail = ctrl_fail_cb
  };
  e2ap_control_req(d->ric, assoc_id, &ric_req, cbr, NULL);
  sleep(1);
  const ric_gen_id_t rd = {.ric_req_id = 1, .ric_inst_id = 2, .ran_func_id = 1};
  const ric_subscription_delete_request_t sdr = { .ric_id = rd, };
  cb_sub_del_t cbd = {
    .sub_del_resp = sub_del_resp_cb,
    .sub_del_fail = sub_del_fail_cb,
  };
  e2ap_unsubscribe(d->ric, assoc_id, &sdr, cbd);
  return NULL;
}

void* static_stop_agent(void* arg)
{
  sleep(4);
  e2ap_agent_t* ag = (e2ap_agent_t*) arg;
  printf("****** stopping agent ******\n");
  e2ap_free_agent(ag);
  return NULL;
}

typedef struct timer_data_s {
  int tfd;
  const ric_subscription_request_t* sr;
} timer_data_t;

void handle_timer(e2ap_agent_t* ag, int tfd, void* data)
{
  assert(ag);
  assert(data);
  assert(tfd > -1);
  timer_data_t* timer_data = (timer_data_t*) data;
  assert(timer_data->tfd == tfd);
  assert(timer_data->sr && timer_data->sr->len_action == 1);

  const char* header = "This is the header";
  byte_array_t hdr = { .buf = (uint8_t*) header, .len = strlen(header) };
  const char* message = "And this is the message";
  byte_array_t msg = { .buf = (uint8_t*) message, .len = strlen(message) };

  const ric_indication_t ind = {
    .ric_id = timer_data->sr->ric_id,
    .action_id = timer_data->sr->action[0].id,
    .sn = NULL,
    .type = RIC_IND_REPORT,
    .hdr = hdr,
    .msg = msg,
    .call_process_id = NULL,
  };
  e2ap_send_indication_agent(ag, 0, &ind);
}

bool handle_subscription_request(e2ap_agent_t* ag, subscription_t* sub, void* rfdata)
{
  assert(ag);
  assert(sub->req && sub->ep_id == 0);
  assert(!rfdata);
  const ric_subscription_request_t* sr = sub->req;
  assert(sr->len_action == 1);
  assert(sr->action[0].type == RIC_ACT_REPORT);

  /* make a timer */
  timer_data_t* timer_data = malloc(sizeof(*timer_data));
  assert(timer_data);
  timer_data->sr = sr;
  timer_data->tfd = e2ap_add_timer_epoll_ms_agent(ag, 1000, 1000, handle_timer, timer_data);
  sub->data = timer_data;

  /* simply accept all */
  ric_action_admitted_t admitted[sr->len_action];
  for (size_t i = 0; i < sr->len_action; ++i)
    admitted[i].ric_act_id = sr->action[i].id;
  ric_subscription_response_t resp = {
    .ric_id = sr->ric_id,
    .admitted = admitted,
    .len_admitted = sr->len_action
  };
  e2ap_send_subscription_response(ag, 0, &resp);
  /* keep the timer_data so we also stop the timer when receiving the
   * subscription delete */
  return true;
}

bool handle_subscription_delete_request(e2ap_agent_t* ag, subscription_t* sub, void* rfdata)
{
  assert(ag);
  assert(sub->req && sub->ep_id == 0 && sub->data);
  assert(!rfdata);
  // for fun and testing: check whether we can find our own subscription
  const subscription_t* s = e2ap_find_subscription(ag, sub->ep_id, sub->req->ric_id);
  assert(s->req == sub->req);
  assert(s->data == sub->data);
  timer_data_t* td = (timer_data_t*) sub->data;
  e2ap_remove_timer_epoll_agent(td->tfd);
  free(td);
  ric_subscription_delete_response_t sub_del_resp = { .ric_id = sub->req->ric_id };
  e2ap_send_subscription_delete_response(ag, 0, &sub_del_resp);
  return true;
}

void handle_control_request(e2ap_agent_t* ag, ep_id_t ep_id, const ric_control_request_t* cr, void* rfdata)
{
  assert(ag);
  assert(ep_id == 0);
  assert(cr);
  assert(!cr->call_process_id);
  assert(!cr->ack_req);
  assert(!rfdata);
  printf("received control request with ID %d/%d\nhdr: %.*s\nmsg: %.*s\n",
         cr->ric_id.ric_req_id, cr->ric_id.ric_inst_id,
         (int) cr->hdr.len, (char*) cr->hdr.buf,
         (int) cr->msg.len, (char*) cr->msg.buf);
}

int main()
{
  e2ap_ric_t ric;
  memset(&ric, 0, sizeof(ric));
  data_t d;
  d.server_ip_str = "127.0.0.1"; 
  d.server_port = 36421;
  d.ric = &ric;

  int rc = pthread_create(&t_server, NULL, static_sctp_start_server, &d);
  assert(rc == 0);

  rc = pthread_create(&t_xapps, NULL, static_subscribe_to_server, &d);
  assert(rc == 0);

  const plmn_t plmn = {.mcc = 10, .mnc = 15, .mnc_digit_len = 2};
  const global_e2_node_id_t ge2ni = {.type = ngran_gNB, .plmn = plmn, .nb_id = 5555};
  e2ap_agent_t* ag = e2ap_init_agent(d.server_ip_str, d.server_port, ge2ni);
  const char* func_def = "PDCP_stats";
  ran_function_t r;
  memset(&r, 0, sizeof(r));
  r.id = 1;
  r.rev = 0;
  r.def.buf = (uint8_t*) func_def;
  r.def.len = strlen(func_def);
  service_model_cb_t cbs = {
    .handle_subscription_request = handle_subscription_request,
    .handle_subscription_delete_request = handle_subscription_delete_request,
    .handle_control_request = handle_control_request,
  };
  e2ap_register_ran_fun(ag, &r, cbs, NULL);

  rc = pthread_create(&t_agent_stop, NULL, static_stop_agent, ag);
  assert(rc == 0);

  e2ap_start_agent(ag); /* blocking */

  e2ap_stop_server(d.ric);

  return 0;
}
