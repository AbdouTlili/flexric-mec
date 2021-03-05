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
#include <stdio.h>

#include "e2ap_msg_enc_asn.h"
#include "e2ap_endpoint_ric.h"

#include "e2ap_msg_handler_ric.h"
#include "e2ap_msg_dec_generic.h" 
#include "e2ap_encode.h"
#include "e2ap_msg_enc_generic.h"
#include "e2ap_msg_free.h"
#include "e2ap_server.h"
#include "events_internal.h"


static
bool check_valid_msg_type(e2_msg_type_t msg_type)
{
  if(msg_type == RIC_SUBSCRIPTION_RESPONSE ||
      msg_type == RIC_SUBSCRIPTION_FAILURE ||
      msg_type == RIC_SUBSCRIPTION_DELETE_RESPONSE ||
      msg_type == RIC_SUBSCRIPTION_DELETE_FAILURE ||
      msg_type == RIC_INDICATION ||
      msg_type == RIC_CONTROL_ACKNOWLEDGE ||
      msg_type == RIC_CONTROL_FAILURE ||
      msg_type == E2AP_ERROR_INDICATION ||
      msg_type == E2_SETUP_REQUEST ||
      msg_type == E2AP_RESET_REQUEST ||
      msg_type == E2AP_RESET_RESPONSE ||
      msg_type == RIC_SERVICE_UPDATE ||
      msg_type == E2_NODE_CONFIGURATION_UPDATE ||
      msg_type == E2_CONNECTION_UPDATE_ACKNOWLEDGE ||
      msg_type == E2_CONNECTION_UPDATE_FAILURE )
    return true;

  return false;
}

void e2ap_msg_handle_ric(e2ap_handle_msg_fp_ric (*handle_msg)[26], e2ap_ric_t* ric, const e2ap_msg_t* msg)
{
  assert(ric != NULL);
  assert(msg != NULL);
  const e2_msg_type_t msg_type = msg->type;
  assert(check_valid_msg_type(msg_type) == true);
  assert(handle_msg[ msg_type ] != NULL);
  (*handle_msg)[msg_type](ric, msg);
}



///////////////////////////////////////////////////////////////////////////////////////////////////
// O-RAN E2APv01.01: Messages for Global Procedures ///////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// E2 -> RIC 
void e2ap_handle_subscription_response_ric(e2ap_ric_t* ric, const e2ap_msg_t* msg)
{
  assert(ric != NULL);
  assert(msg != NULL);
  assert(msg->type == RIC_SUBSCRIPTION_RESPONSE);
  int assoc_id = ric->ep.base.sri.sinfo_assoc_id;
  sub_mgmt_sub_add_confirm(&ric->sub_mgmt, assoc_id, &msg->u_msgs.ric_sub_resp);
}

//E2 -> RIC 
void e2ap_handle_subscription_failure_ric(e2ap_ric_t* ric, const e2ap_msg_t* msg)
{
  assert(ric != NULL);
  assert(msg != NULL);
  assert(msg->type == RIC_SUBSCRIPTION_FAILURE);
  int assoc_id = ric->ep.base.sri.sinfo_assoc_id;
  sub_mgmt_sub_add_reject(&ric->sub_mgmt, assoc_id, &msg->u_msgs.ric_sub_fail);
}

// E2 -> RIC
void e2ap_handle_subscription_delete_response_ric(e2ap_ric_t* ric, const e2ap_msg_t* msg)
{
  assert(ric != NULL);
  assert(msg != NULL);
  assert(msg->type == RIC_SUBSCRIPTION_DELETE_RESPONSE);
  int assoc_id = ric->ep.base.sri.sinfo_assoc_id;
  sub_mgmt_sub_del_confirm(&ric->sub_mgmt, assoc_id, &msg->u_msgs.ric_sub_del_resp);
}

//E2 -> RIC
void e2ap_handle_subscription_delete_failure_ric(e2ap_ric_t* ric, const e2ap_msg_t* msg)
{
  assert(ric != NULL);
  assert(msg != NULL);
  assert(msg->type == RIC_SUBSCRIPTION_DELETE_FAILURE);
  int assoc_id = ric->ep.base.sri.sinfo_assoc_id;
  sub_mgmt_sub_del_reject(&ric->sub_mgmt, assoc_id, &msg->u_msgs.ric_sub_del_fail);
}

// E2 -> RIC
void e2ap_handle_indication_ric(e2ap_ric_t* ric, const e2ap_msg_t* msg)
{
  assert(ric != NULL);
  assert(msg != NULL);
  assert(msg->type == RIC_INDICATION);
  int assoc_id = ric->ep.base.sri.sinfo_assoc_id;
  sub_mgmt_indication(&ric->sub_mgmt, assoc_id, &msg->u_msgs.ric_ind);
}

// E2 -> RIC
void e2ap_handle_control_ack_ric(e2ap_ric_t* ric, const e2ap_msg_t* msg)
{
  assert(ric != NULL);
  assert(msg != NULL);
  assert(msg->type == RIC_CONTROL_ACKNOWLEDGE);
  int assoc_id = ric->ep.base.sri.sinfo_assoc_id;
  sub_mgmt_ctrl_req_ack(&ric->sub_mgmt, assoc_id, &msg->u_msgs.ric_ctrl_ack);
  // ToDO: Look at the previously sent control command, reset the timer and free memory
}

// E2 -> RIC
void e2ap_handle_control_failure_ric(e2ap_ric_t* ric, const e2ap_msg_t* msg)
{
  assert(ric != NULL);
  assert(msg != NULL);
  assert(msg->type == RIC_CONTROL_FAILURE);
  int assoc_id = ric->ep.base.sri.sinfo_assoc_id;
  sub_mgmt_ctrl_req_fail(&ric->sub_mgmt, assoc_id, &msg->u_msgs.ric_ctrl_fail);
}
  
///////////////////////////////////////////////////////////////////////////////////////////////////
// O-RAN E2APv01.01: Messages for Global Procedures ///////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// RIC <-> E2 
void e2ap_handle_error_indication_ric(e2ap_ric_t* ric, const e2ap_msg_t* msg)
{
  assert(ric != NULL);
  assert(msg != NULL);
  assert(0 != 0 && "Not Implemented");


}

// E2 -> RIC
void e2ap_handle_setup_request_ric(e2ap_ric_t* ric, const e2ap_msg_t* msg)
{
  assert(ric != NULL);
  assert(msg != NULL);

  printf("[E2AP] Received SETUP-REQUEST\n");
  fflush(stdout);
  const e2_setup_request_t* req = &msg->u_msgs.e2_stp_req;

  const plmn_t* plmn = &req->id.plmn;
  printf("from PLMN %3d.%*d ID %d\n", plmn->mcc, plmn->mnc_digit_len, plmn->mnc, req->id.nb_id);

  const size_t len_acc = req->len_rf;
  accepted_ran_function_t accepted[len_acc];
  for (size_t i = 0; i < len_acc; ++i) {
    accepted[i] = req->ran_func_item[i].id;
    printf("accepting RAN function ID %d\n", accepted[i]);
  }

  e2_setup_response_t sr = {
      .id.plmn = req->id.plmn, 
      .id.near_ric_id.double_word = 25,
      .accepted = accepted,
      .len_acc = len_acc,
      .rejected = NULL,
      .len_rej = 0,
      . comp_conf_update_ack_list = NULL,
      .len_ccual = 0
  };

  byte_array_t ba = e2ap_enc_setup_response(&sr, &ric->enc.type);

  e2ap_send_bytes_ric(&ric->ep, ba);
  free_byte_array(ba);

  int assoc_id = ric->ep.base.sri.sinfo_assoc_id;
  const ran_t* ran = ran_mgmt_add_e2_node(&ric->ran_mgmt, assoc_id, req);

  trigger_e2node_connect(&ric->events, ran, assoc_id);
  if (ran_mgmt_is_ran_formed(ran))
    trigger_ran_connect(&ric->events, ran);
}

// RIC <-> E2
void e2ap_handle_reset_request_ric(e2ap_ric_t* ric, const e2ap_msg_t* msg)
{
  assert(ric != NULL);
  assert(msg != NULL);
  assert(0 != 0 && "Not Implemented");


}

// RIC <-> E2
void e2ap_handle_reset_response_ric(e2ap_ric_t* ric, const e2ap_msg_t* msg)
{
  assert(ric != NULL);
  assert(msg != NULL);
  assert(0 != 0 && "Not Implemented");
}
  
// E2 -> RIC
void e2ap_handle_service_update_ric(e2ap_ric_t* ric, const e2ap_msg_t* msg)
{
  assert(ric != NULL);
  assert(msg != NULL);
  assert(0 != 0 && "Not Implemented");
}

// E2 -> RIC
void e2ap_handle_node_configuration_update_ric(e2ap_ric_t* ric, const e2ap_msg_t* msg)
{
  assert(ric != NULL);
  assert(msg != NULL);
  assert(0 != 0 && "Not Implemented");
}

// E2 -> RIC
void e2ap_handle_connection_update_ack_ric(e2ap_ric_t* ric, const e2ap_msg_t* msg)
{
  assert(ric != NULL);
  assert(msg != NULL);
  assert(0 != 0 && "Not Implemented");


}

// E2 -> RIC
void e2ap_handle_connection_update_failure_ric(e2ap_ric_t* ric, const e2ap_msg_t* msg)
{
  assert(ric != NULL);
  assert(msg != NULL);
  assert(0 != 0 && "Not Implemented");


}

