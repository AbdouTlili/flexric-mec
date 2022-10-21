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



#ifndef E2AP_TYPE_DEFS_H
#define E2AP_TYPE_DEFS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "util/ngran_types.h"
#include "util/byte_array.h"
#include "lib/ap/e2ap_types/ric_control_request.h"

#include "lib/ap/e2ap_types/e2_setup_request.h"
#include "lib/ap/e2ap_types/e2_setup_response.h"
#include "lib/ap/e2ap_types/ric_indication.h"
#include "lib/ap/e2ap_types/ric_control_request.h"
#include "lib/ap/e2ap_types/ric_subscription_failure.h"
#include "lib/ap/e2ap_types/ric_subscription_request.h"
#include "lib/ap/e2ap_types/ric_subscription_response.h"
#include "lib/ap/e2ap_types/ric_subscription_delete_request.h"
#include "lib/ap/e2ap_types/ric_subscription_delete_response.h"
#include "lib/ap/e2ap_types/ric_subscription_delete_failure.h"
#include "lib/ap/e2ap_types/ric_control_ack.h"
#include "lib/ap/e2ap_types/ric_control_failure.h"
#include "lib/ap/e2ap_types/e2ap_error_indication.h"
#include "lib/ap/e2ap_types/e2_setup_failure.h"
#include "lib/ap/e2ap_types/e2ap_reset_request.h"
#include "lib/ap/e2ap_types/e2ap_reset_response.h"
#include "lib/ap/e2ap_types/ric_service_update.h"
#include "lib/ap/e2ap_types/ric_service_update_ack.h"
#include "lib/ap/e2ap_types/ric_service_update_failure.h"
#include "lib/ap/e2ap_types/ric_service_query.h"
#include "lib/ap/e2ap_types/e2_node_configuration_update.h"
#include "lib/ap/e2ap_types/e2_node_configuration_update_ack.h"
#include "lib/ap/e2ap_types/e2_node_configuration_update_failure.h"
#include "lib/ap/e2ap_types/e2_node_connection_update.h"
#include "lib/ap/e2ap_types/e2_node_connection_update_ack.h"
#include "lib/ap/e2ap_types/e2_node_connection_update_failure.h"

#include "lib/ap/e2ap_types/e42_setup_request.h"
#include "lib/ap/e2ap_types/e42_setup_response.h"
#include "lib/ap/e2ap_types/e42_ric_subscription_request.h"
#include "lib/ap/e2ap_types/e42_ric_subscription_delete_request.h"
#include "lib/ap/e2ap_types/e42_ric_control_request.h"

//////////////////////////////////////////////////

typedef enum {
  RIC_SUBSCRIPTION_REQUEST = 0,
  RIC_SUBSCRIPTION_RESPONSE =1,
  RIC_SUBSCRIPTION_FAILURE = 2,
  RIC_SUBSCRIPTION_DELETE_REQUEST= 3,
  RIC_SUBSCRIPTION_DELETE_RESPONSE = 4,
  RIC_SUBSCRIPTION_DELETE_FAILURE = 5,
  RIC_INDICATION = 6,
  RIC_CONTROL_REQUEST = 7,
  RIC_CONTROL_ACKNOWLEDGE = 8,
  RIC_CONTROL_FAILURE = 9,
  E2AP_ERROR_INDICATION = 10,
  E2_SETUP_REQUEST = 11,
  E2_SETUP_RESPONSE = 12,
  E2_SETUP_FAILURE = 13,
  E2AP_RESET_REQUEST = 14,
  E2AP_RESET_RESPONSE = 15,
  RIC_SERVICE_UPDATE =16,
  RIC_SERVICE_UPDATE_ACKNOWLEDGE = 17,
  RIC_SERVICE_UPDATE_FAILURE = 18,
  RIC_SERVICE_QUERY = 19,
  E2_NODE_CONFIGURATION_UPDATE = 20,
  E2_NODE_CONFIGURATION_UPDATE_ACKNOWLEDGE = 21,
  E2_NODE_CONFIGURATION_UPDATE_FAILURE = 22,
  E2_CONNECTION_UPDATE = 23,
  E2_CONNECTION_UPDATE_ACKNOWLEDGE = 24,
  E2_CONNECTION_UPDATE_FAILURE = 25,

  E42_SETUP_REQUEST = 26,
  E42_SETUP_RESPONSE = 27,
  E42_RIC_SUBSCRIPTION_REQUEST = 28,
  E42_RIC_SUBSCRIPTION_DELETE_REQUEST = 29,
  E42_RIC_CONTROL_REQUEST = 30,

  // Last type to indicate no message
  NONE_E2_MSG_TYPE = 31,
} e2_msg_type_t;

typedef struct e2ap_msg_s {
  const e2_msg_type_t type; // the message type for logic handling 
  union {
    ric_subscription_request_t ric_sub_req;
    ric_subscription_response_t ric_sub_resp;
    ric_subscription_failure_t ric_sub_fail;
    ric_subscription_delete_request_t ric_sub_del_req;
    ric_subscription_delete_response_t ric_sub_del_resp;
    ric_subscription_delete_failure_t ric_sub_del_fail;
    ric_indication_t ric_ind;
    ric_control_request_t ric_ctrl_req;
    ric_control_acknowledge_t ric_ctrl_ack;
    ric_control_failure_t ric_ctrl_fail;
    e2ap_error_indication_t err_ind;
    e2_setup_request_t e2_stp_req;
    e2_setup_response_t e2_stp_resp;
    e2_setup_failure_t e2_stp_fail;
    e2ap_reset_request_t rst_req;
    e2ap_reset_response_t rst_resp;
    ric_service_update_t ric_serv_updt;
    ric_service_update_ack_t ric_serv_updt_ack;
    ric_service_update_failure_t ric_serv_updt_fail;
    ric_service_query_t ric_serv_query;
    e2_node_configuration_update_t e2_node_conf_updt;
    e2_node_configuration_update_ack_t e2_node_conf_updt_ack;
    e2_node_configuration_update_failure_t e2_node_conf_updt_fail;
    e2_node_connection_update_t e2_conn_updt;
    e2_node_connection_update_ack_t  e2_conn_updt_ack;
    e2_node_connection_update_failure_t e2_conn_updt_fail;

    e42_setup_request_t e42_stp_req;
    e42_setup_response_t e42_stp_resp;
    e42_ric_subscription_request_t e42_ric_sub_req;
    e42_ric_subscription_delete_request_t e42_ric_sub_del_req;
    e42_ric_control_request_t e42_ric_ctrl_req;
  } u_msgs;
  int64_t tstamp; // for debugginf purposes;
} e2ap_msg_t;

#endif // E2AP_TYPE_DEFS_H 

