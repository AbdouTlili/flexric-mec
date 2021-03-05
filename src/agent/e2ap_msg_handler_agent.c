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

#include "e2ap_msg_handler_agent.h"
#include "e2ap_msg_dec_generic.h"
#include "e2ap_msg_enc_generic.h"
#include "e2ap_msg_free.h"
#include "e2ap_encode.h"

#include <stdio.h>

static
bool check_valid_msg_type(e2_msg_type_t msg_type )
{
  return msg_type == RIC_SUBSCRIPTION_REQUEST
      || msg_type == RIC_SUBSCRIPTION_DELETE_REQUEST
      || msg_type == RIC_CONTROL_REQUEST
      || msg_type == E2AP_ERROR_INDICATION
      || msg_type == E2_SETUP_RESPONSE
      || msg_type == E2_SETUP_FAILURE
      || msg_type == E2AP_RESET_REQUEST
      || msg_type == E2AP_RESET_RESPONSE
      || msg_type == RIC_SERVICE_UPDATE_ACKNOWLEDGE
      || msg_type == RIC_SERVICE_UPDATE_FAILURE
      || msg_type == RIC_SERVICE_QUERY
      || msg_type == E2_NODE_CONFIGURATION_UPDATE_ACKNOWLEDGE
      || msg_type == E2_NODE_CONFIGURATION_UPDATE_FAILURE
      || msg_type == E2_CONNECTION_UPDATE;
}




void e2ap_msg_handle_agent(e2ap_handle_msg_fp_agent (*handle_msg)[26], e2ap_agent_t* ag, ep_id_t ep_id, const e2ap_msg_t* msg)
{
  assert(msg != NULL);
  const e2_msg_type_t msg_type = msg->type;
  assert(check_valid_msg_type(msg_type) == true);
  assert(handle_msg[ msg_type ] != NULL);
  (*handle_msg)[msg_type](ag, ep_id, msg);
}

void e2ap_handle_subscription_request_agent(e2ap_agent_t* ag, ep_id_t ep_id, const e2ap_msg_t* msg)
{
  assert(ag != NULL);
  assert(ep_id >= 0 && ep_id < ag->num_ep);
  assert(msg != NULL);

  assert(msg->type == RIC_SUBSCRIPTION_REQUEST);

  ric_subscription_request_t* sr = deep_copy_sub_req(&msg->u_msgs.ric_sub_req);

  // ToDo
  // assert that the ric is actually the one that the agent is connected to

  ran_function_container_t* rf_container = e2ap_get_ran_fun_agent(ag, sr->ric_id.ran_func_id);
  // assert that the agent really has the ran function ID requested by the RIC
  assert(rf_container);

  subscription_t* sub = malloc(sizeof(*sub));
  sub->req = sr;
  *(ep_id_t*)&sub->ep_id = ep_id;

  const bool rc = rf_container->cbs.handle_subscription_request(ag, sub, rf_container->data);
  if (rc) {
    // persist this subscription request
    e2ap_register_sub_req_agent(rf_container, ep_id, sub);
  } else {
    assert(!sub->data);
    // the subscription is not accepted, we don't need it
    free_subscription_request(sr);
    free(sr);
    free(sub);
  }
}

void e2ap_handle_subscription_delete_request_agent(e2ap_agent_t* ag, ep_id_t ep_id, const e2ap_msg_t* msg)
{
  assert(ag != NULL);
  assert(ep_id >= 0 && ep_id < ag->num_ep);
  assert(msg != NULL);
  assert(msg->type == RIC_SUBSCRIPTION_DELETE_REQUEST);

  const ric_subscription_delete_request_t* sdr = &msg->u_msgs.ric_sub_del_req;
  ran_function_container_t* rf_container = e2ap_get_ran_fun_agent(ag, sdr->ric_id.ran_func_id);
  assert(rf_container);
  subscription_t* s = e2ap_get_sub_req_agent(rf_container, ep_id, sdr->ric_id);
  bool del_accepted = rf_container->cbs.handle_subscription_delete_request(ag, s, rf_container->data);

  if (del_accepted) {
    // use hack to get mutable sub req (which is not modified except for freeing)
    ric_subscription_request_t* sr = (ric_subscription_request_t*) s->req;
    free_subscription_request(sr);
    free(sr);
    free(s);
    e2ap_delete_sub_req_agent(rf_container, ep_id, sdr->ric_id);
  }
}

// The purpose of the RIC Control procedure is to initiate or resume a specific functionality in the E2 Node.
void e2ap_handle_control_request_agent(e2ap_agent_t* ag, ep_id_t ep_id, const e2ap_msg_t* msg)
{
  assert(ag != NULL);
  assert(ep_id >= 0 && ep_id < ag->num_ep);
  assert(msg != NULL);
  assert(msg->type == RIC_CONTROL_REQUEST);


  // ToDo
  // assert that the ric is actually the one that the agent is connected to

  const ric_control_request_t* cr = &msg->u_msgs.ric_ctrl_req;
  const uint16_t ran_func = cr->ric_id.ran_func_id;
  const ran_function_container_t* rf_container = e2ap_get_ran_fun_agent(ag, ran_func);
  // assert that the agent really has the ran function ID requested by the RIC
  assert(rf_container);

  rf_container->cbs.handle_control_request(ag, ep_id, cr, rf_container->data);
}

void e2ap_handle_error_indication_agent(e2ap_agent_t* ag, ep_id_t ep_id, const e2ap_msg_t* msg)
{
  assert(ag != NULL);
  assert(ep_id >= 0 && ep_id < ag->num_ep);
  assert(msg != NULL);
  assert(0!=0 && "Not implemented");


}

void  e2ap_handle_setup_response_agent(e2ap_agent_t* ag, ep_id_t ep_id, const e2ap_msg_t* msg)
{
  assert(ag != NULL);
  assert(ep_id >= 0 && ep_id < ag->num_ep);
  assert(msg != NULL);
  assert(msg->type == E2_SETUP_RESPONSE);
  printf("Setup response received in the agent\n");

}

void e2ap_handle_setup_failure_agent(e2ap_agent_t* ag, ep_id_t ep_id, const e2ap_msg_t* msg)
{
  assert(ag != NULL);
  assert(ep_id >= 0 && ep_id < ag->num_ep);
  assert(msg != NULL);
  assert(0!=0 && "Not implemented");


}

void e2ap_handle_reset_request_agent(e2ap_agent_t* ag, ep_id_t ep_id, const e2ap_msg_t* msg)
{
  assert(ag != NULL);
  assert(ep_id >= 0 && ep_id < ag->num_ep);
  assert(msg != NULL);
  assert(0!=0 && "Not implemented");


}

void e2ap_handle_reset_response_agent(e2ap_agent_t* ag, ep_id_t ep_id, const e2ap_msg_t* msg)
{
  assert(ag != NULL);
  assert(ep_id >= 0 && ep_id < ag->num_ep);
  assert(msg != NULL);
  assert(0!=0 && "Not implemented");


}
  
void e2ap_handle_service_update_ack_agent(e2ap_agent_t* ag, ep_id_t ep_id, const e2ap_msg_t* msg)
{
  assert(ag != NULL);
  assert(ep_id >= 0 && ep_id < ag->num_ep);
  assert(msg != NULL);
  assert(0!=0 && "Not implemented");

}

void e2ap_handle_service_update_failure_agent(e2ap_agent_t* ag, ep_id_t ep_id, const e2ap_msg_t* msg)
{
  assert(ag != NULL);
  assert(ep_id >= 0 && ep_id < ag->num_ep);
  assert(msg != NULL);
  assert(0!=0 && "Not implemented");


}

void e2ap_handle_service_query_agent(e2ap_agent_t* ag, ep_id_t ep_id, const e2ap_msg_t* msg)
{
  assert(ag != NULL);
  assert(ep_id >= 0 && ep_id < ag->num_ep);
  assert(msg != NULL);
  assert(0!=0 && "Not implemented");

}

void e2ap_handle_node_configuration_update_ack_agent(e2ap_agent_t* ag, ep_id_t ep_id, const e2ap_msg_t* msg)
{
  assert(ag != NULL);
  assert(ep_id >= 0 && ep_id < ag->num_ep);
  assert(msg != NULL);
  assert(0!=0 && "Not implemented");


}

void e2ap_handle_node_configuration_update_failure_agent(e2ap_agent_t* ag, ep_id_t ep_id, const e2ap_msg_t* msg)
{
  assert(ag != NULL);
  assert(ep_id >= 0 && ep_id < ag->num_ep);
  assert(msg != NULL);
  assert(0!=0 && "Not implemented");


}

void e2ap_handle_connection_update_agent(e2ap_agent_t* ag, ep_id_t ep_id, const e2ap_msg_t* msg)
{
  assert(ag != NULL);
  assert(ep_id >= 0 && ep_id < ag->num_ep);
  assert(msg != NULL);
  assert(0!=0 && "Not implemented");


}

