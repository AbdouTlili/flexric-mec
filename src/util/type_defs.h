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

#include "ngran_types.h"
#include "byte_array.h"


#include "ric_control_request.h"

//#include "e2ap_types/ric_control_request.h"
//#include "e2ap_types/e2_setup_request.h"

#include "e2_setup_request.h"
#include "e2_setup_response.h"
#include "ric_indication.h"
#include "ric_control_request.h"
#include "ric_subscription_failure.h"
#include "ric_subscription_request.h"
#include "ric_subscription_response.h"
#include "ric_subscription_delete_request.h"
#include "ric_subscription_delete_response.h"
#include "ric_subscription_delete_failure.h"
#include "ric_control_ack.h"
#include "ric_control_failure.h"
#include "e2ap_error_indication.h"
#include "e2_setup_failure.h"
#include "e2ap_reset_request.h"
#include "e2ap_reset_response.h"
#include "ric_service_update.h"
#include "ric_service_update_ack.h"
#include "ric_service_update_failure.h"
#include "ric_service_query.h"
#include "e2_node_configuration_update.h"
#include "e2_node_configuration_update_ack.h"
#include "e2_node_configuration_update_failure.h"
#include "e2_node_connection_update.h"
#include "e2_node_connection_update_ack.h"
#include "e2_node_connection_update_failure.h"


/*
//bool cmp_ric_gen_id(ric_gen_id_t a, ric_gen_id_t b);
//ric_gen_id_t gen_ric_id(uint16_t ric_req_id, uint16_t ric_inst_id, uint16_t ran_func_id);

bool cmp_byte_array(const byte_array_t* a, const byte_array_t* b);

typedef enum { RIC_SERVICES, TRANSPORT_LAYER, PROTOCOL, MISC } cause_group_t;


typedef enum {
  RAN_FUNCTION_ID_INVALID,
  ACTION_NOT_SUPPORTED,
  EXCESSIVE_ACTIONS,
  DUPLICATE_ACTION,
  DUPLICATE_EVENT_TRIGGER,
  FUNCTION_RESOURCE_LIMIT,
  RIC_REQUEST_ID_UNKNOWN,
  INCONSISTENT_ACTION_SUBSEQUENT_ACTION_SEQUENCE,
  CONTROL_MESSAGE_INVALID,
  RIC_CALL_PROCESS_ID_INVALID,
  RIC_REQ_UNSPECIFIED
} ric_request_t;

typedef enum {
  FUNCTION_NOT_REQUIRED,
  EXCESSIVE_FUNCTIONS,
  RIC_RESOURCE_LIMIT
} ric_service_t;

typedef enum { UNSPECIFIED, TRANSPORT_RESOURCE_UNAVAILABLE } transport_layer_cause_t;

typedef enum {
  TRANSFER_SYNTAX_ERROR,
  ABSTRACT_SYNTAX_ERROR_REJECT,
  ABSTRACT_SYNTAX_ERROR_IGNORE_AND_NOTIFY,
  MESSAGE_NOT_COMPATIBLE_WITH_RECEIVER_STATE,
  SEMANTIC_ERROR,
  ABSTRACT_SYNTAX_ERROR_FALSELY_CONSTRUCTED_MESSAGE,
  PROT_UNSPECIFIED
} protocol_cause_t;

typedef enum {
  CONTROL_PROCESSING_OVERLOAD,
  HARDWARE_FAILURE,
  O_M_INTERVENTION,
  MISC_UNSPECIFIED
} misc_cause_t;

typedef struct transport_layer_information {
  uint8_t address[20];
  uint16_t port;
} transport_layer_information_t;

// inside of E2 Setup msgs

//bool cmp_ran_function(const ran_function_t* a, const ran_function_t* b);

//void free_ran_function(ran_function_t* rf);

// inside of E2 Setup msgs
//typedef struct deleted_ran_function{
//  uint16_t id;
//  uint16_t revision;
//} deleted_ran_function_t;


//bool cmp_e2_node_component_config_update(const e2_node_component_config_update_t* a,
//                                         const e2_node_component_config_update_t* b);
//void free_e2_node_component_config_update(e2_node_component_config_update_t* nccu);

//bool cmp_plmn(plmn_t a, plmn_t b);

//bool cmp_global_e2_node_id(const global_e2_node_id_t* a, const global_e2_node_id_t* b);

*/

/*
*/


//bool cmp_e2_setup_request(const e2_setup_request_t* a, const e2_setup_request_t* b);
//void free_e2_setup_request(e2_setup_request_t* sr);


/*
typedef struct e2_setup_failure {
  cause_t cause;
  uint32_t* time_to_wait_ms;            // optional
  criticality_diagnostics_t* crit_diag; // optional
  transport_layer_information_t* tl_info; // optional
} e2_setup_failure_t;

typedef struct reset_request {
  cause_t cause;
} reset_request_t;

typedef struct reset_response {
  criticality_diagnostics_t* crit_diag; // optional
} reset_response_t;

typedef struct ric_service_update {
  ran_function_t* added;
  size_t len_added;

  ran_function_t* modified;
  size_t len_modified;

  deleted_ran_function_t* deleted;
  size_t len_deleted;
} ric_service_update_t;

typedef struct ric_service_update_acknowledge {
  uint16_t* accepted;
  size_t len_accepted;

  rejected_ran_function_t* rejected;
  size_t len_rejected;
} ric_service_update_acknowledge_t;

typedef struct ric_service_update_failure {
  rejected_ran_function_t* rejected;
  uint32_t* time_to_wait_ms;
  criticality_diagnostics_t crit_diag;
} ric_service_update_failure_t;

typedef struct ric_service_query {
  // I admit that the following looks strange, but it matches the types 
  deleted_ran_function_t* accepted;
  size_t len_accepted;
} ric_service_query_t;

typedef struct e2_node_configuration_update {
  e2_node_component_config_update_t comp_conf_update_list;
  size_t len_ccul;
} e2_node_configuration_update_t;

typedef struct e2_node_configuration_update_acknowledge {
  e2_node_component_config_update_t* comp_conf_update_ack_list;
  size_t len_ccual;
} e2_node_configuration_update_acknowledge_t;

typedef struct e2_node_configuration_update_failure {
  cause_t cause;
  uint32_t* time_to_wait_ms; // optional
  criticality_diagnostics_t* crit_diag; // optional
} e2_node_configuration_update_failure_t;

typedef struct e2_connection_update {
  // TODO fill 
} e2_connection_update_t;

typedef struct e2_connection_update_acknowledge {
  // TODO fill 
} e2_connection_update_acknowledge_t;

typedef struct e2_connection_update_failure {
  // TODO fill 
} e2_connection_update_failure_t;


typedef enum { ACT_REPORT, ACT_INSERT, ACT_POLICY } ric_action_type_t;
typedef enum { CONTINUE, HALT } ric_subseq_action_type_t;

typedef struct ric_subsequent_action {
  ric_subseq_action_type_t type;
  uint32_t* time_to_wait_ms; // optional
} ric_subsequent_action_t;

typedef struct ric_action {
  uint8_t id;
  ric_action_type_t type;
  byte_array_t* definition; // optional
  ric_subsequent_action_t* subseq_action; // optional
} ric_action_t;

typedef struct ric_subscription_request {
  ric_gen_id_t ric_id;
  byte_array_t event_trigger;
  ric_action_t* action;
  size_t len_action;
} ric_subscription_request_t;

typedef uint16_t ric_action_admitted_t;
typedef struct ric_action_not_admitted {
  uint8_t id;
  cause_t cause;
} ric_action_not_admitted_t;

typedef struct ric_subscription_response {
  ric_gen_id_t ric_id;
  ric_action_admitted_t* admitted;
  size_t len_admitted;
  ric_action_not_admitted_t* not_admitted;
  size_t len_na;
} ric_subscription_response_t;

typedef struct ric_subscription_delete_request {
  ric_gen_id_t ric_id;
} ric_subscription_delete_request_t;

typedef struct ric_subscription_delete_request_t ric_subscription_delete_response_t;

typedef struct ric_subscription_delete_failure {
  ric_gen_id_t ric_id;
  cause_t cause;
  criticality_diagnostics_t* crit_diag; // optional
} ric_subscription_delete_failure_t;

typedef enum { NO_ACK, ACK, NACK } ric_control_ack_req_t;
typedef struct ric_control_request {
  ric_gen_id_t ric_id;
  byte_array_t* call_process_id; // optional
  byte_array_t header;
  byte_array_t message;
  ric_control_ack_req_t* ack_req; // optional
} ric_control_request_t;

typedef enum { SUCCESS, REJECTED, FAILED } ric_control_status_t;

*/

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
  } u_msgs;
} e2ap_msg_t;

#endif // E2AP_TYPE_DEFS_H 
