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


#include "e42_xapp.h"

#include "act_proc.h"
#include "asio_xapp.h"
#include "async_event_xapp.h"
#include "endpoint_xapp.h"
#include "msg_handler_xapp.h"
#include "msg_generator_xapp.h"
#include "pending_event_xapp.h"

#include "../lib/pending_events.h"
#include "../lib/ap/e2ap_ap.h"
#include "../lib/ap/free/e2ap_msg_free.h"

#include "../util/alg_ds/alg/alg.h"
#include "../util/alg_ds/ds/seq_container/seq_generic.h"
#include "../util/alg_ds/ds/lock_guard/lock_guard.h"
#include "../util/compare.h"
#include "../util/time_now_us.h"

#include "../sm/mac_sm/mac_sm_id.h"
#include "../sm/rlc_sm/rlc_sm_id.h"
#include "../sm/pdcp_sm/pdcp_sm_id.h"
#include "../sm/slice_sm/slice_sm_id.h"
#include "../sm/tc_sm/tc_sm_id.h"
#include "../sm/gtp_sm/gtp_sm_id.h"
#include "../sm/kpm_sm_v2.02/kpm_sm_id.h"


#include <assert.h>
#include <time.h>
#include <stdio.h>
#include <pthread.h>



static inline
bool net_pkt(const e42_xapp_t* xapp, int fd)
{
  assert(xapp != NULL);
  assert(fd > 0);
  return fd == xapp->ep.base.fd;
}

static inline
bool pend_event(e42_xapp_t* xapp, int fd, pending_event_t** p_ev)
{
  assert(xapp != NULL);
  assert(fd > 0);
  assert(*p_ev == NULL);

  lock_guard(&xapp->pending.pend_mtx);
  bi_map_t* map = &xapp->pending.pending; 

  void* it = assoc_front(&map->left);
  void* end = assoc_end(&map->left);

  it = find_if(&map->left, it, end, &fd, eq_fd);

  assert(it != end);
  *p_ev = assoc_value(&map->left ,it);
  return *p_ev != NULL;
}


static
async_event_xapp_t find_event_type(e42_xapp_t* xapp, int fd)
{
  assert(xapp != NULL);
  assert(fd > 0);
  async_event_xapp_t e = {.type = UNKNOWN_EVENT };
  if (net_pkt(xapp, fd) == true){
    e.type = NETWORK_EVENT;
//  } else if (ind_event(xapp, fd, &e.i_ev) == true) {
//    e.type = INDICATION_EVENT;

//  } else if (find_pending_event_fd(&xapp->pending, fd)  == true) { //, &e.p_ev) == true){
  } else if (pend_event(xapp, fd, &e.p_ev)  == true) { //, &e.p_ev) == true){
    e.type = PENDING_EVENT;

  } else{
    assert(0!=0 && "Unknown event happened!");
  }
  return e;
}

static
void consume_fd(int fd)
{
  assert(fd > 0);
  uint64_t read_buf = 0;
  ssize_t const bytes = read(fd,&read_buf, sizeof(read_buf));
  assert(bytes == sizeof(read_buf));
}

static
void read_xapp(sm_ag_if_rd_t* data)
{
  assert(data != NULL);
  assert(0!=0 && "Program should never come here");
  exit(-1);
}

static
sm_ag_if_ans_t write_xapp(sm_ag_if_wr_t const* data)
{
  assert(data != NULL);
  assert(0!=0 && "Program should never come here");
  exit(-1);
  sm_ag_if_ans_t ans = {0};
  return ans; 
}

e42_xapp_t* init_e42_xapp(fr_args_t const* args)
{
  assert(args != NULL);

  printf("[xAapp]: Initializing ... \n");

  e42_xapp_t* xapp = calloc(1, sizeof(*xapp));
  assert(xapp != NULL && "Memory exhausted");

  uint32_t const port = 36422;

  char* addr = get_near_ric_ip(args);
  defer({ free(addr); } );

  printf("[xApp]: RIC IP Address = %s\n", addr);

  e2ap_init_ep_xapp(&xapp->ep, addr, port);

  init_asio_xapp(&xapp->io); 

  add_fd_asio_xapp(&xapp->io, xapp->ep.base.fd);

  init_ap(&xapp->ap.base.type);

  init_handle_msg_xapp(&xapp->handle_msg);

  sm_io_ag_t io = {.read = read_xapp, .write = write_xapp };
  init_plugin_ag(&xapp->plugin_ag, args->libs_dir, io);
  init_plugin_ric(&xapp->plugin_ric, args->libs_dir);

  init_reg_e2_node(&xapp->e2_nodes); 

  init_sync_ui(&xapp->sync);

  init_pending_events(&xapp->pending);

  init_act_proc(&xapp->act_proc);

  init_msg_dispatcher(&xapp->msg_disp);

  const char* dir = XAPP_DB_DIR; // + "xapp_db.sqlite3"; //  XAPP_DB_DIR; // + "/xapp_db";
  assert(strlen(dir) < 128 && "String too large" );

  int64_t const now = time_now_us(); 
  char filename[256] = {0};
  int n = snprintf(filename, 255, "%s_%ld", dir, now);
  assert(n < 256 && "Overflow");

  printf("Filename = %s \n ", filename );

  init_db_xapp(&xapp->db, filename);

  xapp->connected = false;
  xapp->stop_token = false;
  xapp->stopped = false;
  
  return xapp;
}

static inline
void send_setup_request(e42_xapp_t* xapp)
{
  assert(xapp != NULL);
  assert(xapp->handle_msg[E42_SETUP_REQUEST]!= NULL);
  xapp->handle_msg[E42_SETUP_REQUEST](xapp, NULL);
}

static
void e2_event_loop_xapp(e42_xapp_t* xapp)
{
  assert(xapp != NULL);
  while (xapp->stop_token == false) {
    int fd = event_asio_xapp(&xapp->io);
    if(fd == -1) continue; // no event happened. Just for checking the stop_token condition

    async_event_xapp_t const e = find_event_type(xapp,fd);

    assert(e.type != UNKNOWN_EVENT && "Unknown event triggered ");

    if(e.type == NETWORK_EVENT){ 

      byte_array_t ba = e2ap_recv_msg_xapp(&xapp->ep);
      defer( {free_byte_array(ba);} );

      e2ap_msg_t msg = e2ap_msg_dec_xapp(&xapp->ap, ba);
      defer( { e2ap_msg_free_xapp(&xapp->ap, &msg);} );

      e2ap_msg_t ans = e2ap_msg_handle_xapp(xapp, &msg);
      defer( { e2ap_msg_free_xapp(&xapp->ap, &ans);} );

      if(ans.type != NONE_E2_MSG_TYPE){
        byte_array_t ba_ans = e2ap_msg_enc_xapp(&xapp->ap, &ans); 
        defer ({free_byte_array(ba_ans); } );

        e2ap_send_bytes_xapp(&xapp->ep, ba_ans);
      }
    } else if(e.type == PENDING_EVENT){
      assert(( *e.p_ev == E42_SETUP_REQUEST_PENDING_EVENT 
            || *e.p_ev == E42_RIC_SUBSCRIPTION_REQUEST_PENDING_EVENT
            || *e.p_ev == E42_RIC_SUBSCRIPTION_DELETE_REQUEST_PENDING_EVENT 
            || *e.p_ev == E42_RIC_CONTROL_REQUEST_PENDING_EVENT ) && "Unforeseen pending event happened!" );

      assert(*e.p_ev != E42_RIC_SUBSCRIPTION_REQUEST_PENDING_EVENT && "Timeout waiting for Report. Connection lost with the RIC?");
      assert(*e.p_ev != E42_RIC_SUBSCRIPTION_DELETE_REQUEST_PENDING_EVENT  && "Timeout waiting for Subscription Delete. Connection lost with the RIC?");
      assert(*e.p_ev != E42_RIC_CONTROL_REQUEST_PENDING_EVENT && "Timeout waiting for Control ACK. Connection lost with the RIC?");

      // Resend the subscription request message
      e42_setup_request_t sr = generate_e42_setup_request(xapp);
      defer({ e2ap_free_e42_setup_request(&sr);  } );

      printf("[E2AP]: Resending Setup Request after timeout\n");
      byte_array_t ba = e2ap_enc_e42_setup_request_xapp(&xapp->ap, &sr);
      defer({free_byte_array(ba); } );

      e2ap_send_bytes_xapp(&xapp->ep, ba);

      consume_fd(fd);
    } else {
      assert(0!=0 && "An interruption that it is not a network pkt, or a timer expired pending event happened!");
    }

  }
  xapp->stopped = true;
}


void start_e42_xapp(e42_xapp_t* xapp)
{
  assert(xapp != NULL);

  send_setup_request(xapp);

  e2_event_loop_xapp(xapp);
}

void free_e42_xapp(e42_xapp_t* xapp)
{
  assert(xapp != NULL);

  xapp->stop_token = true;
  while(xapp->stopped == false){
    usleep(1000);
  }

  free_reg_e2_node(&xapp->e2_nodes); 

  free_plugin_ag(&xapp->plugin_ag);
  free_plugin_ric(&xapp->plugin_ric);

  free_pending_events(&xapp->pending);

  free_sync_ui(&xapp->sync);

  free_act_proc(&xapp->act_proc);

  free_msg_dispatcher(&xapp->msg_disp);

  close_db_xapp(&xapp->db);

  free(xapp);
}

e2_node_arr_t e2_nodes_xapp(e42_xapp_t* xapp)
{
  assert(xapp != NULL);
  e2_node_arr_t ans = generate_e2_node_arr(&xapp->e2_nodes); 
  return ans;
}

static
void send_subscription_request(e42_xapp_t* xapp, global_e2_node_id_t* id, ric_gen_id_t ric_id, inter_xapp_e i)
{
  assert(xapp != NULL);
  assert(id != NULL);
  assert(xapp->handle_msg[E42_RIC_SUBSCRIPTION_REQUEST]!= NULL);

  char* cmd = NULL;
  if(i == ms_1 ){
    cmd = "1_ms";
  } else if(i == ms_2){
    cmd = "2_ms";
  } else if(i == ms_5 ){
    cmd = "5_ms";
  } else {
    assert(0!=0 && "Unsupported interval type. Check the SM on_subscription for details");
  }

  sm_ric_t* sm = sm_plugin_ric(&xapp->plugin_ric, ric_id.ran_func_id);

  ric_subscription_request_t sr = generate_subscription_request( ric_id, sm, cmd);
  e42_ric_subscription_request_t e42_sr = {
    .xapp_id = xapp->id,
    .id = cp_global_e2_node_id(id),
    .sr = sr
  }; 
  defer({ e2ap_free_e42_ric_subscription_request(&e42_sr);};);

  e2ap_msg_t msg = {.type = E42_RIC_SUBSCRIPTION_REQUEST,
                    .u_msgs.e42_ric_sub_req = e42_sr 
  };

  xapp->handle_msg[E42_RIC_SUBSCRIPTION_REQUEST](xapp, &msg);
}

static
bool valid_ran_func_id(uint16_t ran_func_id)\
{
  if(ran_func_id == SM_SLICE_ID 
      || ran_func_id == SM_MAC_ID
      || ran_func_id == SM_RLC_ID
      || ran_func_id == SM_PDCP_ID
      || ran_func_id == SM_TC_ID
      || ran_func_id == SM_GTP_ID
      || ran_func_id == SM_KPM_ID
    )
    return true;

  return false;
}


static
ric_gen_id_t generate_ric_gen_id(e42_xapp_t* xapp, act_proc_val_e type, uint16_t ran_func_id, global_e2_node_id_t const* id, sm_cb cb)
{
  assert(xapp != NULL);
  assert(valid_ran_func_id(ran_func_id) == true);

  ric_gen_id_t ric_req = {.ric_inst_id = 0, .ran_func_id = ran_func_id };
  uint32_t const req_id = add_act_proc(&xapp->act_proc, type, ric_req, id, cb); 
  printf("Generated of req_id = %d \n", req_id);
  assert(req_id < 1 << 16 && "Overflow detected");
  ric_req.ric_req_id = req_id;

  return ric_req;
}


sm_ans_xapp_t report_sm_sync_xapp(e42_xapp_t* xapp, global_e2_node_id_t* id, uint16_t ran_func_id, inter_xapp_e i, sm_cb cb)
{
  assert(xapp != NULL);
  assert(id != NULL);

  // Generate and registry the ric_req_id
  ric_gen_id_t ric_id = generate_ric_gen_id(xapp, RIC_SUBSCRIPTION_PROCEDURE_ACTIVE ,ran_func_id, id, cb);

  // Send message 
  send_subscription_request(xapp, id, ric_id , i);

  // Wait for the answer (it will arrive in the event loop)
  cond_wait_sync_ui(&xapp->sync, xapp->sync.wait_ms);

  // Answer arrived
  printf("[xApp]: Successfully SUBSCRIBED to ran function = %d \n", ran_func_id );

  // The RIC_SUBSCRIPTION_PROCEDURE is still active

  sm_ans_xapp_t ans = {0};
  ans.success = true;
  ans.u.handle = ric_id.ric_req_id;

  return ans;
}

static
void send_ric_subscription_delete(e42_xapp_t* xapp, ric_gen_id_t ric_id)
{
  assert(xapp != NULL);
  assert(xapp->handle_msg[E42_RIC_SUBSCRIPTION_DELETE_REQUEST] != NULL);

  e2ap_msg_t msg = {.type = E42_RIC_SUBSCRIPTION_DELETE_REQUEST };

  msg.u_msgs.e42_ric_sub_del_req.sdr.ric_id = ric_id;
  msg.u_msgs.e42_ric_sub_del_req.xapp_id = xapp->id;

  xapp->handle_msg[E42_RIC_SUBSCRIPTION_DELETE_REQUEST](xapp, &msg );
}


void rm_report_sm_sync_xapp(e42_xapp_t* xapp, int ric_req_id)
{
  assert(xapp != NULL);
  assert(ric_req_id  > -1 && ric_req_id < 1 << 16);

  act_proc_ans_t ans = find_act_proc(&xapp->act_proc, ric_req_id);
  if(ans.ok == false){
    printf("%s \n", ans.error); 
    assert(0!=0 && "ric_req_id not registered");
    exit(-1);
  }

  // Send message
  send_ric_subscription_delete(xapp, ans.val.id);

  // Wait for the answer (it will arrive in the event loop)
  cond_wait_sync_ui(&xapp->sync,xapp->sync.wait_ms);

  // Answer arrived
  printf("[xApp]: Successfully received SUBSCRIPTION-DELETE-RESPONSE \n");

  // Remove the active procedure  
  rm_act_proc(&xapp->act_proc, ric_req_id ); 
}


static
void send_control_request(e42_xapp_t* xapp,  global_e2_node_id_t* id, ric_gen_id_t ric_req, sm_ag_if_wr_t const* ctrl_msg)
{
  assert(xapp != NULL);
  assert(id != NULL);
  assert(ctrl_msg != NULL);
  assert(xapp->handle_msg[E42_RIC_CONTROL_REQUEST] != NULL);

  sm_ric_t* sm = sm_plugin_ric(&xapp->plugin_ric, ric_req.ran_func_id);
  
  ric_control_request_t ctrl_req = generate_ric_control_request(ric_req, sm, ctrl_msg);

  e42_ric_control_request_t e42_cr = { .xapp_id = xapp->id,
                                       .id = cp_global_e2_node_id(id),
                                       .ctrl_req = ctrl_req 
                                      };
  defer({ e2ap_free_e42_ric_control_request(&e42_cr);};);

  e2ap_msg_t msg = {.type = E42_RIC_CONTROL_REQUEST,
                    .u_msgs.e42_ric_ctrl_req = e42_cr 
                    };

  xapp->handle_msg[E42_RIC_CONTROL_REQUEST](xapp, &msg);
}


sm_ans_xapp_t control_sm_sync_xapp(e42_xapp_t* xapp, global_e2_node_id_t* id, uint16_t ran_func_id, sm_ag_if_wr_t const* ctrl_msg)
{
  assert(xapp != NULL);
  assert(id != NULL);
  assert(valid_ran_func_id(ran_func_id) == true);
  assert(ctrl_msg != NULL);

  // Generate and registry the ric_req_id
  ric_gen_id_t ric_id = generate_ric_gen_id(xapp, RIC_CONTROL_PROCEDURE_ACTIVE, ran_func_id, id, NULL);

  // Send the message
  send_control_request(xapp, id, ric_id, ctrl_msg);  

  // Wait for the answer (it will arrive in the event loop)
  cond_wait_sync_ui(&xapp->sync, xapp->sync.wait_ms);

  // Answer received
  printf("[xApp]: Successfully received CONTROL-ACK \n");
 
  // Remove the active procedure, control request  
  rm_act_proc(&xapp->act_proc, ric_id.ric_req_id ); 
 
  sm_ans_xapp_t const ans = {0};
  return ans;
}


bool connected_e42_xapp( e42_xapp_t* xapp)
{
  assert(xapp != NULL);
  return xapp->connected;
}


size_t not_dispatch_msg(e42_xapp_t* xapp)
{
  assert(xapp != NULL);

  return size_msg_dispatcher(&xapp->msg_disp );
}

