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

#ifndef E2AP_MSG_ENC_ASN_H
#define E2AP_MSG_ENC_ASN_H

#include "ngran_types.h"
#include "type_defs.h"


/* These are forward declarations so that we can include this header without
 * needing the actual definition of the E2AP types. TODO take them out once the
 * function signatures only depend on our own types defined in type_defs.h */

struct E2AP_PDU;
struct RANfunction_Item;
struct RANfunctionID_Item;
struct RICaction_ToBeSetup_Item;
struct RANfunctionIDcause_Item;
struct RICsubsequentAction;
struct Cause;
struct CriticalityDiagnostics;
struct E2nodeComponentConfigUpdate_Item;
struct E2nodeComponentConfigUpdateAck_Item;
struct E2connectionUpdate_Item;
struct E2nodeComponentConfigUpdateAck_Item;
struct TNLinformation;
struct RANfunctionIDcause_Item_t;
struct E2nodeComponentConfigUpdate_Item;
struct E2connectionUpdate_Item;
struct E2connectionSetupFailed_Item;


byte_array_t e2ap_enc_asn_pdu_ba(struct E2AP_PDU* pdu);

///////////////////////////////////////////////////////////////////////////////////////////////////
// O-RAN E2APv01.01: Messages for Near-RT RIC Functional Procedures////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// RIC -> E2
byte_array_t e2ap_enc_subscription_request_asn(const ric_subscription_request_t* sr);
struct E2AP_PDU* e2ap_enc_subscription_request_asn_pdu(const ric_subscription_request_t* sr);   

// E2 -> RIC 
byte_array_t e2ap_enc_subscription_response_asn(const ric_subscription_response_t* sr);
struct E2AP_PDU* e2ap_enc_subscription_response_asn_pdu(const ric_subscription_response_t* sr);

//E2 -> RIC 
byte_array_t e2ap_enc_subscription_failure_asn(const ric_subscription_failure_t* sf);
struct E2AP_PDU* e2ap_enc_subscription_failure_asn_pdu(const ric_subscription_failure_t* sf);

//RIC -> E2
byte_array_t e2ap_enc_subscription_delete_request_asn(const ric_subscription_delete_request_t* dr); 
struct E2AP_PDU* e2ap_enc_subscription_delete_request_asn_pdu(const ric_subscription_delete_request_t* dr); 

// E2 -> RIC
byte_array_t e2ap_enc_subscription_delete_response_asn(const ric_subscription_delete_response_t* dr); 
struct E2AP_PDU* e2ap_enc_subscription_delete_response_asn_pdu(const ric_subscription_delete_response_t* dr); 

//E2 -> RIC
byte_array_t e2ap_enc_subscription_delete_failure_asn(const ric_subscription_delete_failure_t* df);
struct E2AP_PDU* e2ap_enc_subscription_delete_failure_asn_pdu(const ric_subscription_delete_failure_t* df);

// E2 -> RIC
byte_array_t e2ap_enc_indication_asn(const ric_indication_t* ind );
struct E2AP_PDU* e2ap_enc_indication_asn_pdu( const ric_indication_t* ind );

// RIC -> E2
byte_array_t e2ap_enc_control_request_asn(const ric_control_request_t* ric_req);
struct E2AP_PDU* e2ap_enc_control_request_asn_pdu(const ric_control_request_t* ric_req);

// E2 -> RIC
byte_array_t e2ap_enc_control_ack_asn(const ric_control_acknowledge_t* ca);
struct E2AP_PDU* e2ap_enc_control_ack_asn_pdu(const ric_control_acknowledge_t* ca);

// E2 -> RIC
byte_array_t e2ap_enc_control_failure_asn(const ric_control_failure_t* cf);
struct E2AP_PDU* e2ap_enc_control_failure_asn_pdu(const ric_control_failure_t* cf);

///////////////////////////////////////////////////////////////////////////////////////////////////
// O-RAN E2APv01.01: Messages for Global Procedures ///////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// RIC <-> E2 
byte_array_t e2ap_enc_error_indication_asn(const e2ap_error_indication_t* ei);
struct E2AP_PDU* e2ap_enc_error_indication_asn_pdu(const e2ap_error_indication_t* ei);

// E2 -> RIC
byte_array_t e2ap_enc_setup_request_asn(const e2_setup_request_t* sr);
struct E2AP_PDU* e2ap_enc_setup_request_asn_pdu(const e2_setup_request_t* sr);

// RIC -> E2
byte_array_t e2ap_enc_setup_response_asn(const e2_setup_response_t* sr);
struct E2AP_PDU* e2ap_enc_setup_response_asn_pdu(const e2_setup_response_t* sr);

// RIC -> E2
byte_array_t e2ap_enc_setup_failure_asn(const e2_setup_failure_t* sf);
struct E2AP_PDU* e2ap_enc_setup_failure_asn_pdu(const e2_setup_failure_t* sf);

// RIC <-> E2
byte_array_t e2ap_enc_reset_request_asn(const e2ap_reset_request_t* rr);
struct E2AP_PDU* e2ap_enc_reset_request_asn_pdu(const e2ap_reset_request_t* rr);

// RIC <-> E2
byte_array_t e2ap_enc_reset_response_asn(const e2ap_reset_response_t* rr);
struct E2AP_PDU* e2ap_enc_reset_response_asn_pdu(const e2ap_reset_response_t* rr);

// E2 -> RIC
byte_array_t e2ap_enc_service_update_asn(const ric_service_update_t* su);
struct E2AP_PDU* e2ap_enc_service_update_asn_pdu(const ric_service_update_t* su);

// RIC -> E2
byte_array_t e2ap_enc_service_update_ack_asn(const ric_service_update_ack_t* su);
struct E2AP_PDU* e2ap_enc_service_update_ack_asn_pdu(const ric_service_update_ack_t* su);

// RIC -> E2
byte_array_t e2ap_enc_service_update_failure_asn(const ric_service_update_failure_t* uf);
struct E2AP_PDU* e2ap_enc_service_update_failure_asn_pdu(const ric_service_update_failure_t* uf);

// RIC -> E2
byte_array_t e2ap_enc_service_query_asn(const ric_service_query_t* sq);
struct E2AP_PDU* e2ap_enc_service_query_asn_pdu(const ric_service_query_t* sq);

// E2 -> RIC
byte_array_t e2ap_enc_node_configuration_update_asn(const e2_node_configuration_update_t* cu);
struct E2AP_PDU* e2ap_enc_node_configuration_update_asn_pdu(const e2_node_configuration_update_t* cu);

// RIC -> E2
byte_array_t e2ap_enc_node_configuration_update_ack_asn(const e2_node_configuration_update_ack_t* cua);
struct E2AP_PDU* e2ap_enc_node_configuration_update_ack_asn_pdu(const e2_node_configuration_update_ack_t* cua);

// RIC -> E2
byte_array_t e2ap_enc_node_configuration_update_failure_asn(const e2_node_configuration_update_failure_t* cuf);
struct E2AP_PDU* e2ap_enc_node_configuration_update_failure_asn_pdu(const e2_node_configuration_update_failure_t* cuf);

// RIC -> E2
byte_array_t e2ap_enc_node_connection_update_asn(const e2_node_connection_update_t* cu);
struct E2AP_PDU* e2ap_enc_node_connection_update_asn_pdu(const e2_node_connection_update_t* cu);

// E2 -> RIC
byte_array_t e2ap_enc_node_connection_update_ack_asn(const e2_node_connection_update_ack_t* ca);
struct E2AP_PDU* e2ap_enc_node_connection_update_ack_asn_pdu(const e2_node_connection_update_ack_t* ca);

// E2 -> RIC
byte_array_t e2ap_enc_node_connection_update_failure_asn(const e2_node_connection_update_failure_t*);
struct E2AP_PDU* e2ap_enc_node_connection_update_failure_asn_pdu(const e2_node_connection_update_failure_t*);

#endif
