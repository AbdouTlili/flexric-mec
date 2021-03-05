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

#include "e2ap_msg_enc_asn.h"
#include "e2ap_msg_dec_generic.h"
#include "type_defs.h"

#include "E2AP-PDU.h"
#include "asn_application.h"
#include "Cause.h"
#include "RICindicationType.h"
#include "E2nodeComponentConfigUpdateENB.h"
#include "RICaction-ToBeSetup-Item.h"
#include "RICsubsequentAction.h"
#include "RICcontrolAckRequest.h"
#include "RICcontrolStatus.h"
#include "RANfunction-Item.h"
#include "RANfunctionID-Item.h"
#include "RANfunctionIDcause-Item.h"
#include "E2nodeComponentConfigUpdate-Item.h"
#include "E2nodeComponentConfigUpdateAck-Item.h"
#include "E2connectionUpdate-Item.h"

#define BYTE_ARRAY_FROM_STRING(name, string) \
  char *(name##str) = string;              \
  byte_array_t (name) = {.buf = (uint8_t*) (name##str), .len = strlen((name##str))}
/*
static
void try_encode( E2AP_PDU_t* pdu)
{
  assert(pdu != NULL);
  uint8_t buffer[2048];
  xer_fprint(stderr, &asn_DEF_E2AP_PDU, pdu);
  const enum asn_transfer_syntax syntax = ATS_ALIGNED_BASIC_PER; // ATS_BASIC_XER; // ATS_ALIGNED_BASIC_PER;
  asn_enc_rval_t er = asn_encode_to_buffer(NULL, syntax, &asn_DEF_E2AP_PDU, pdu, buffer, 2048);
  fprintf(stdout, "er encoded is %ld\n", er.encoded);
  fflush(stdout);
  assert(er.encoded < 2048);
  if(er.encoded == -1){
    printf("Failed the encoding in type %s and xml_type = %s\n", er.failed_type->name, er.failed_type->xml_tag); 
    fflush(stdout);
  }
  assert(er.encoded > -1);
}

static
void free_pdu(E2AP_PDU_t* pdu)
{
  assert(pdu != NULL);
  ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2AP_PDU,pdu);
  free(pdu);
}

void test_ric_subscription_request()
{
  const uint16_t ric_req_id = 10;
  const uint16_t ric_inst_id = 25;
  const uint16_t ran_func_id = 256;
  BYTE_ARRAY_FROM_STRING(event_trigger, "Trigger the event!!!");

  const int num_ric_act = 5;
  RICaction_ToBeSetup_Item_t ric_act_setup[num_ric_act];
  memset(&ric_act_setup, 0, sizeof(RICaction_ToBeSetup_Item_t)* num_ric_act);
  byte_array_t action_defs[num_ric_act];
  byte_array_t *action_def_arr[num_ric_act];
  char *ad = "With the following action!";
  for(int i =0 ; i < num_ric_act; ++i){
    ric_act_setup[i].ricActionID = i;
    ric_act_setup[i].ricActionType = RICactionType_report;	
    action_defs[i].buf = (uint8_t*)ad;
    action_defs[i].len = strlen(ad);
    action_def_arr[i] = &action_defs[i];
  }
  const int  num_ric_sub_act = 5;
  RICsubsequentAction_t ric_sub_act[num_ric_sub_act];
  memset(&ric_sub_act, 0, sizeof(RICsubsequentAction_t)* num_ric_sub_act);
  for(int i = 0; i < num_ric_sub_act; ++i){
    ric_sub_act->ricSubsequentActionType = RICsubsequentActionType_continue;
    ric_sub_act->ricTimeToWait = RICtimeToWait_w10ms;	
  }

  ric_gen_id_t r = gen_ric_id(ric_req_id, ric_inst_id, ran_func_id);

  //E2AP_PDU_t* pdu = e2ap_gen_subscription_request(r, event_trigger, ric_act_setup, action_def_arr, num_ric_act, ric_sub_act, num_ric_sub_act);
  //try_encode(pdu);
  //free_pdu(pdu);
}

void test_ric_subscription_response()
{
  const uint16_t ric_req_id = 15;
  const uint16_t ric_inst_id = 12;
  const uint16_t ran_func_id = 98;

  uint16_t adm_item_id[3];
  adm_item_id[0] = 0;
  adm_item_id[1] = 20;
  adm_item_id[2] = 200;

  uint16_t not_adm_item_id[5];
  not_adm_item_id[0] = 2;
  not_adm_item_id[1] = 3;
  not_adm_item_id[2] = 4;
  not_adm_item_id[3] = 5;
  not_adm_item_id[4] = 6;

  uint16_t not_adm_item_err_cause[5];
  not_adm_item_err_cause[0] = 1;
  not_adm_item_err_cause[1] = 1;
  not_adm_item_err_cause[2] = 1;
  not_adm_item_err_cause[3] = 1;
  not_adm_item_err_cause[4] = 1;

  uint16_t not_adm_item_err_cause_detail[5];
  not_adm_item_err_cause_detail[0] = 0;
  not_adm_item_err_cause_detail[1] = 0;
  not_adm_item_err_cause_detail[2] = 0;
  not_adm_item_err_cause_detail[3] = 0;
  not_adm_item_err_cause_detail[4] = 0;

  ric_gen_id_t r = gen_ric_id(ric_req_id, ric_inst_id, ran_func_id);

  E2AP_PDU_t* pdu =  e2ap_enc_subscription_response(r, adm_item_id, 3, not_adm_item_id, not_adm_item_err_cause, not_adm_item_err_cause_detail, 5);
  try_encode(pdu);
  free_pdu(pdu);
}

void test_ric_subscription_failure()
{
  const uint16_t ric_req_id = 15;
  const uint16_t ric_inst_id = 12;
  const uint16_t ran_func_id = 98;

  uint16_t not_adm_item_id[5];
  not_adm_item_id[0] = 2;
  not_adm_item_id[1] = 3;
  not_adm_item_id[2] = 4;
  not_adm_item_id[3] = 5;
  not_adm_item_id[4] = 6;

  uint16_t not_adm_item_err_cause[5];
  not_adm_item_err_cause[0] = 1;
  not_adm_item_err_cause[1] = 1;
  not_adm_item_err_cause[2] = 1;
  not_adm_item_err_cause[3] = 1;
  not_adm_item_err_cause[4] = 1;

  uint16_t not_adm_item_err_cause_detail[5];
  not_adm_item_err_cause_detail[0] = 0;
  not_adm_item_err_cause_detail[1] = 0;
  not_adm_item_err_cause_detail[2] = 0;
  not_adm_item_err_cause_detail[3] = 0;
  not_adm_item_err_cause_detail[4] = 0;

  ric_gen_id_t r = gen_ric_id(ric_req_id, ric_inst_id, ran_func_id);
  E2AP_PDU_t* pdu = e2ap_enc_subscription_failure(r , not_adm_item_id, not_adm_item_err_cause, not_adm_item_err_cause_detail, 5, NULL); 
  try_encode(pdu);
  free_pdu(pdu);
}

void test_ric_subscription_delete_request()
{
  const uint16_t ric_req_id = 10;
  const uint16_t ric_inst_id = 25;
  const uint16_t ran_func_id = 256;
 
  ric_gen_id_t r = gen_ric_id(ric_req_id, ric_inst_id, ran_func_id);
  E2AP_PDU_t* pdu = e2ap_enc_subscription_delete_request(r);
  try_encode(pdu);
  free_pdu(pdu);
}

void test_ric_subscription_delete_response()
{
  const uint16_t ric_req_id = 10;
  const uint16_t ric_inst_id = 25;
  const uint16_t ran_func_id = 256;

  ric_gen_id_t r = gen_ric_id(ric_req_id, ric_inst_id, ran_func_id);
  E2AP_PDU_t* pdu = e2ap_enc_subscription_delete_response(r);
  try_encode(pdu);
  free_pdu(pdu);
}

void test_ric_subscription_delete_failure()
{
  const uint16_t ric_req_id = 10;
  const uint16_t ric_inst_id = 25;
  const uint16_t ran_func_id = 256;
  Cause_t c; 
  memset(&c, 0, sizeof(Cause_t));
  c.present = Cause_PR_ricRequest;
  c.choice.ricRequest = CauseRICservice_ric_resource_limit;

  ric_gen_id_t r = gen_ric_id(ric_req_id, ric_inst_id, ran_func_id);
  E2AP_PDU_t* pdu = e2ap_enc_subscription_delete_failure(r , c, NULL);
  try_encode(pdu);
  free_pdu(pdu);
}

void test_ric_indication()
{
  const uint16_t ric_req_id = 10;
  const uint16_t ric_inst_id = 25;
  const uint16_t ran_func_id = 256;
  const uint8_t ric_act_id = 9;
  const uint16_t ric_ind_sn = 90;
  const e_RICindicationType ind_type = RICindicationType_insert;	 
  BYTE_ARRAY_FROM_STRING(ric_ind_hdr, "RIC Header string. It should be encoded through E2SM");
  BYTE_ARRAY_FROM_STRING(ric_ind_msg, "RIC indication message");
  BYTE_ARRAY_FROM_STRING(ric_call_proc, "RIC call proc id = 12345");
  ric_gen_id_t r = gen_ric_id(ric_req_id, ric_inst_id, ran_func_id);
  //E2AP_PDU_t* pdu = e2ap_gen_indication(r, ric_act_id, ric_ind_sn, ind_type, ric_ind_hdr, ric_ind_msg, &ric_call_proc);
  //try_encode(pdu);
  //free_pdu(pdu);
}

void test_ric_control_request()
{
  const uint16_t ric_req_id = 10;
  const uint16_t ric_inst_id = 25;
  const uint16_t ran_func_id = 256;
  BYTE_ARRAY_FROM_STRING(ric_ind_hdr, "RIC Header string. It should be encoded through E2SM");
  BYTE_ARRAY_FROM_STRING(ric_ind_msg, "RIC indication message");
  BYTE_ARRAY_FROM_STRING(ric_call_proc, "RIC call proc id = 12345");
  const e_RICcontrolAckRequest ack_req =  RICcontrolAckRequest_ack;	
  ric_gen_id_t r = gen_ric_id(ric_req_id, ric_inst_id, ran_func_id);
  E2AP_PDU_t* pdu = e2ap_enc_control_request(r, &ric_call_proc, ric_ind_hdr, ric_ind_msg, ack_req);
  try_encode(pdu);
  free_pdu(pdu);
}

void test_ric_control_ack()
{
  const uint16_t ric_req_id = 10;
  const uint16_t ric_inst_id = 25;
  const uint16_t ran_func_id = 256;
  BYTE_ARRAY_FROM_STRING(ric_call_proc, "RIC call proc id = 12345");
  const e_RICcontrolStatus ctrl_status = RICcontrolStatus_success; 
  BYTE_ARRAY_FROM_STRING(ric_ctrl_out, "RIC control Outcome");
  ric_gen_id_t r = gen_ric_id(ric_req_id, ric_inst_id, ran_func_id);
  E2AP_PDU_t* pdu = e2ap_enc_control_ack(r , &ric_call_proc, ctrl_status, &ric_ctrl_out);
  try_encode(pdu);
  free_pdu(pdu);
}

void test_ric_control_failure()
{
  const uint16_t ric_req_id = 10;
  const uint16_t ric_inst_id = 25;
  const uint16_t ran_func_id = 256;
  BYTE_ARRAY_FROM_STRING(ric_call_proc, "RIC call proc id = 12345");
  BYTE_ARRAY_FROM_STRING(ric_ctrl_out, "RIC control Outcome");
  Cause_t c; 
  memset(&c, 0, sizeof(Cause_t));
  c.present = Cause_PR_ricRequest;
  c.choice.ricRequest = CauseRICservice_ric_resource_limit;
  ric_gen_id_t r = gen_ric_id(ric_req_id, ric_inst_id, ran_func_id);
  E2AP_PDU_t* pdu = e2ap_enc_control_failure(r, &ric_call_proc, c, &ric_ctrl_out);
  try_encode(pdu);
  free_pdu(pdu);
}

void test_error_indication()
{
  const uint16_t ric_req_id = 10;
  const uint16_t ric_inst_id = 25;
  const uint16_t ran_func_id = 256;
  Cause_t c; 
  memset(&c, 0, sizeof(Cause_t));
  c.present = Cause_PR_ricRequest;
  c.choice.ricRequest = CauseRICservice_ric_resource_limit;
  ric_gen_id_t r = gen_ric_id(ric_req_id, ric_inst_id, ran_func_id);
  E2AP_PDU_t* pdu = e2ap_enc_error_indication(r , c, NULL);
  try_encode(pdu);
  free_pdu(pdu);
}

void test_e2_setup_request()
{
  const plmn_t plmn = {.mcc = 10, .mnc = 15, .mnc_digit_len = 2};
  const global_e2_node_id_t id = {.type = ngran_gNB, .plmn = plmn, .nb_id = 1};
  const size_t num_ran_func = 5;
  ran_function_t rf[num_ran_func];
  const char* ran_func_def = "Hello world of ORAN";
  const size_t size_ran_func = strlen(ran_func_def);
  for (size_t i = 0; i < num_ran_func; ++i) {
    rf[i].id = i + 12;
    rf[i].def.buf = (uint8_t*) ran_func_def;
    rf[i].def.len = size_ran_func;
    rf[i].rev = 1;
    rf[i].oid = NULL;
  }
  const size_t num_comp_conf_update = 1;
  e2_node_component_config_update_t ccu[num_comp_conf_update];
  const char *ngap = "NGAP";
  const size_t size_ngap = strlen(ngap);
  byte_array_t ba_ngap = {.buf = (uint8_t*) ngap, .len = size_ngap};
  e2_node_component_id_present_t id_present = E2_NODE_COMPONENT_ID_E2_NODE_COMPONENT_TYPE_GNB_CU_UP;
	//E2_NODE_COMPONENT_ID_E2_NODE_COMPONENT_TYPE_GNB_DU

  for (size_t i = 0; i < num_comp_conf_update; ++i) {
    ccu[i].e2_node_component_type = E2_NODE_COMPONENT_TYPE_NG_ENB;
    ccu[i].id_present = &id_present;
    ccu[i].gnb_cu_up_id = 12;
    ccu[i].update_present =E2_NODE_COMPONENT_CONFIG_UPDATE_NG_ENB_CONFIG_UPDATE;

    ccu[i].ng_enb.ngap_ng_enb = &ba_ngap;
    ccu[i].ng_enb.xnap_ng_enb = NULL;
  }

  const e2_setup_request_t e2_sr = {
    .id = id,
    .ran_func_item = rf,
    .len_rf = num_ran_func,
    .comp_conf_update = ccu,
    .len_ccu = num_comp_conf_update
  };

//  byte_array_t ba = e2ap_enc_setup_request(&e2_sr);
//  e2ap_msg_t msg = e2ap_dec_message(ba);
//  assert(msg.type == E2_SETUP_REQUEST);
//  assert(cmp_e2_setup_request(&e2_sr, msg.e2_stp_req));

//  e2ap_free_message(msg);
//  free(ba.buf);
}

void test_e2_setup_response()
{
  const uint16_t mcc = 10;
  const uint16_t mnc = 15;
  const uint8_t mnc_digit_len = 2;
  const uint32_t nb_id = 1;  
  const int num_ran_func_set_res = 5;
  RANfunctionID_Item_t ran_func_item[num_ran_func_set_res];  
  memset(ran_func_item, 0, sizeof( RANfunctionID_Item_t ) * num_ran_func_set_res);
  for(int i = 0; i < num_ran_func_set_res ; ++i){
    ran_func_item[i].ranFunctionID = i + 17;
    ran_func_item[i].ranFunctionRevision = 1;
  }
  const int num_ran_func_rej = 3;
  RANfunctionIDcause_Item_t ran_func_id_cause[num_ran_func_rej];
  memset(ran_func_id_cause, 0, sizeof(RANfunctionIDcause_Item_t)* num_ran_func_rej);
  for(int i =0; i < num_ran_func_rej; ++i){
    ran_func_id_cause[i].ranFunctionID = i + 19;
    Cause_t	c;
    memset(&c, 0, sizeof(Cause_t));
    c.present = Cause_PR_ricService;
    c.choice.ricService = CauseRICservice_function_not_required;	
    ran_func_id_cause[i].cause = c;
  }

//  E2AP_PDU_t* pdu = e2ap_gen_setup_response(mcc, mnc, mnc_digit_len , nb_id, ran_func_item, num_ran_func_set_res, ran_func_id_cause, num_ran_func_rej, NULL , 0);
//  try_encode(pdu);
//  free_pdu(pdu);
}

void test_e2_setup_failure()
{
    Cause_t	c;
    memset(&c, 0, sizeof(Cause_t));
    c.present = Cause_PR_ricService;
    c.choice.ricService = CauseRICservice_function_not_required;	
 
  E2AP_PDU_t* pdu = e2ap_enc_setup_failure(c, NULL, NULL, NULL); 
  try_encode(pdu);
  free_pdu(pdu);
}

void test_reset_request() 
{
  Cause_t c; 
  memset(&c, 0, sizeof(Cause_t));
  c.present = Cause_PR_ricRequest;
  c.choice.ricRequest = CauseRICservice_ric_resource_limit;

  E2AP_PDU_t* pdu = e2ap_enc_reset_request(c);
  try_encode(pdu);
  free_pdu(pdu);
}

void test_reset_response() 
{
  E2AP_PDU_t* pdu6 = e2ap_enc_reset_response(NULL);
  try_encode(pdu6);
  free_pdu(pdu6);
}

void test_ric_service_update() 
{
  const int num_ran_func = 2; 
  const char* ran_func_def = "Hello world of ORAN";
  const size_t size_ran_func = strlen(ran_func_def ); 
  RANfunction_Item_t ran_func[num_ran_func];
  memset(&ran_func, 0, sizeof(RANfunction_Item_t )*num_ran_func);
  for(int i = 0; i < num_ran_func; ++i){
    ran_func[i].ranFunctionDefinition.buf = malloc(size_ran_func); 
    memcpy(ran_func[i].ranFunctionDefinition.buf, ran_func_def, size_ran_func);
    ran_func[i].ranFunctionDefinition.size = size_ran_func;
    ran_func[i].ranFunctionID = i + 12;
    ran_func[i].ranFunctionRevision = 1;
  }

  E2AP_PDU_t* pdu5 = e2ap_enc_service_update(ran_func, num_ran_func, NULL, 0, NULL, 0); 
  try_encode(pdu5);
  free_pdu(pdu5);
}

void test_ric_service_update_ack() 
{
  const int num_ran_func = 2; 
  RANfunction_Item_t ran_func[num_ran_func];
  memset(&ran_func, 0, sizeof(RANfunction_Item_t )*num_ran_func);
  for(int i = 0; i < num_ran_func; ++i){
    ran_func[i].ranFunctionID = i + 12;
    ran_func[i].ranFunctionRevision = 1;
  }

  const int num_ran_func_rej = 3;
  RANfunctionIDcause_Item_t ran_func_id_cause[num_ran_func_rej];
  memset(ran_func_id_cause, 0, sizeof(RANfunctionIDcause_Item_t)* num_ran_func_rej);
  for(int i =0; i < num_ran_func_rej; ++i){
    ran_func_id_cause[i].ranFunctionID = i + 19;
    Cause_t	c;
    memset(&c, 0, sizeof(Cause_t));
    c.present = Cause_PR_ricService;
    c.choice.ricService = CauseRICservice_function_not_required;	
    ran_func_id_cause[i].cause = c;
  }

  E2AP_PDU_t* pdu = e2ap_enc_service_update_ack(ran_func, num_ran_func, ran_func_id_cause, num_ran_func_rej); 
  try_encode(pdu);
  free_pdu(pdu);
}

void test_ric_service_update_failure() 
{
  const int num_ran_func_rej = 3;
  RANfunctionIDcause_Item_t ran_func_id_cause[num_ran_func_rej];
  memset(ran_func_id_cause, 0, sizeof(RANfunctionIDcause_Item_t)* num_ran_func_rej);
  for(int i =0; i < num_ran_func_rej; ++i){
    ran_func_id_cause[i].ranFunctionID = i + 19;
    Cause_t	c;
    memset(&c, 0, sizeof(Cause_t));
    c.present = Cause_PR_ricService;
    c.choice.ricService = CauseRICservice_function_not_required;	
    ran_func_id_cause[i].cause = c;
  }

  E2AP_PDU_t* pdu = e2ap_enc_service_update_failure(ran_func_id_cause, 3, NULL, NULL);
  try_encode(pdu);
  free_pdu(pdu);
}

void test_ric_service_query() 
{
  const int num_ran_func = 2; 
  RANfunction_Item_t ran_func[num_ran_func];
  memset(&ran_func, 0, sizeof(RANfunction_Item_t )*num_ran_func);
  for(int i = 0; i < num_ran_func; ++i){
    ran_func[i].ranFunctionID = i + 12;
    ran_func[i].ranFunctionRevision = 1;
  }

  E2AP_PDU_t* pdu = e2ap_enc_service_query(ran_func, num_ran_func);
  try_encode(pdu);
  free_pdu(pdu);
}

void test_e2_node_configuration_update()  
{
  const int num_func_conf = 2;
  E2nodeComponentConfigUpdate_Item_t n[num_func_conf];
  memset(n, 0, sizeof(E2nodeComponentConfigUpdate_Item_t)*num_func_conf);
  for(int i = 0; i < num_func_conf; ++i){
    n[i].e2nodeComponentType = E2nodeComponentType_eNB;	
    n[i].e2nodeComponentConfigUpdate.choice.eNBconfigUpdate = calloc(1, sizeof(E2nodeComponentConfigUpdateENB_t));
    n[i].e2nodeComponentConfigUpdate.present = E2nodeComponentConfigUpdate_PR_eNBconfigUpdate;
  }
  E2AP_PDU_t* pdu = e2ap_node_configuration_update(n, num_func_conf);
  try_encode(pdu);
  free_pdu(pdu);
}

void test_e2_node_configuration_update_ack()  
{
  const int num_func_conf = 2;
  E2nodeComponentConfigUpdateAck_Item_t it[num_func_conf];
  memset(it, 0, sizeof(E2nodeComponentConfigUpdateAck_Item_t)* num_func_conf);
  for(int i =0; i < num_func_conf; ++i){
    it[i].e2nodeComponentType = E2nodeComponentType_eNB;	 
    it[i].e2nodeComponentID = NULL;
    it[i].e2nodeComponentConfigUpdateAck.updateOutcome = 0; // 0 success, 1 failure
    it[i].e2nodeComponentConfigUpdateAck.failureCause = NULL;
  }

  E2AP_PDU_t* pdu = e2ap_node_configuration_update_ack(it, num_func_conf);
  try_encode(pdu);
  free_pdu(pdu);
}

void test_e2_node_configuration_update_failure()  
{
  Cause_t	c;
  memset(&c, 0, sizeof(Cause_t));
  c.present = Cause_PR_ricService;
  c.choice.ricService = CauseRICservice_function_not_required;	

  E2AP_PDU_t* pdu = e2ap_node_configuration_update_failure(c, NULL, NULL); 
  try_encode(pdu);
  free_pdu(pdu);
}

void test_e2_node_connection_update() 
{
  const int num_TNLA_add = 2;
  E2connectionUpdate_Item_t con_add[num_TNLA_add];
  memset(con_add, 0, sizeof(E2connectionUpdate_Item_t)*num_TNLA_add);
  for(int i = 0; i < num_TNLA_add ; ++i){
    Cause_t c;
    memset(&c, 0, sizeof(Cause_t));
    c.present = Cause_PR_ricService;
    c.choice.ricService = CauseRICservice_function_not_required;	
    const char* addr = "192.168.0.1";
    con_add[i].tnlInformation.tnlAddress.buf = malloc(strlen(addr));
    memcpy( con_add[i].tnlInformation.tnlAddress.buf, addr, strlen(addr)); 
    con_add[i].tnlInformation.tnlAddress.size = strlen(addr);
    con_add[i].tnlInformation.tnlAddress.bits_unused = 0;
    con_add[i].tnlUsage = TNLusage_ric_service;
  }
  E2AP_PDU_t* pdu = e2ap_connection_update(con_add, num_TNLA_add, NULL, 0 ); 
  try_encode(pdu);
  free_pdu(pdu);
}

void test_e2_node_connection_update_ack() 
{
  const int num_con_setup = 2;
  E2connectionUpdate_Item_t con_setup[num_con_setup];
  memset(con_setup, 0, sizeof(E2connectionUpdate_Item_t)*num_con_setup);
  for(int i = 0; i < num_con_setup; ++i){
    Cause_t c;
    memset(&c, 0, sizeof(Cause_t));
    c.present = Cause_PR_ricService;
    c.choice.ricService = CauseRICservice_function_not_required;	
    const char* addr = "192.168.0.1";
    con_setup[i].tnlInformation.tnlAddress.buf = malloc(strlen(addr));
    memcpy( con_setup[i].tnlInformation.tnlAddress.buf, addr, strlen(addr)); 
    con_setup[i].tnlInformation.tnlAddress.size = strlen(addr);
    con_setup[i].tnlInformation.tnlAddress.bits_unused = 0;
    con_setup[i].tnlUsage = TNLusage_ric_service;
  }
  E2AP_PDU_t* pdu = e2ap_connection_update_ack(con_setup, num_con_setup, NULL, 0 ); 
  try_encode(pdu);
  free_pdu(pdu);
}

void test_e2_node_connection_update_failure() 
{
  Cause_t c;
  memset(&c, 0, sizeof(Cause_t));
  c.present = Cause_PR_ricService;
  c.choice.ricService = CauseRICservice_function_not_required;	

  E2AP_PDU_t* pdu = e2ap_connection_update_failure(c,NULL, NULL); 
  try_encode(pdu);
  free_pdu(pdu);
}

*/

int main()
{
//  const int repetitioins = 10000;
//  for(int i = 0; i <  repetitioins; ++i){
/*
    test_ric_subscription_request();
    test_ric_subscription_response();
    test_ric_subscription_failure();
    test_ric_subscription_delete_request();
    test_ric_subscription_delete_response();
    test_ric_subscription_delete_failure();
    test_ric_indication();
    test_ric_control_request();
    test_ric_control_ack();
    test_ric_control_failure();

    test_error_indication();
    test_e2_setup_request();
    test_e2_setup_response();
    test_e2_setup_failure();
    test_reset_request();
    test_reset_response(); 
    test_ric_service_update(); 
    test_ric_service_update_ack(); 
    test_ric_service_update_failure(); 
    test_ric_service_query(); 
    test_e2_node_configuration_update();  
    test_e2_node_configuration_update_ack();  
    test_e2_node_configuration_update_failure();  
    test_e2_node_connection_update();
    test_e2_node_connection_update_ack(); 
    test_e2_node_connection_update_failure(); 
  */
//  }
  return 0;
}
