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


#include <stddef.h>
#include <stdint.h>

#include "e2ap_msg_dec_asn.h"

#include "lib/ap/ie/asn/E2AP-PDU.h"
#include "lib/ap/ie/asn/E2setupRequest.h"
#include "lib/ap/ie/asn/InitiatingMessage.h"
#include "lib/ap/ie/asn/SuccessfulOutcome.h"
#include "lib/ap/ie/asn/UnsuccessfulOutcome.h"
#include "lib/ap/ie/asn/GlobalE2node-gNB-ID.h"
#include "lib/ap/ie/asn/ProtocolIE-Field.h"
#include "lib/ap/ie/asn/RANfunction-Item.h"
#include "lib/ap/ie/asn/E2AP-PDU.h"
#include "lib/ap/ie/asn/E2nodeComponentConfigUpdate-Item.h"
#include "lib/ap/ie/asn/E2nodeComponentID.h"
#include "lib/ap/ie/asn/E2nodeComponentGNB-CU-UP-ID.h"
#include "lib/ap/ie/asn/E2nodeComponentGNB-DU-ID.h"
#include "lib/ap/ie/asn/E2nodeComponentConfigUpdateGNB.h"
#include "lib/ap/ie/asn/E2nodeComponentConfigUpdateENgNB.h"
#include "lib/ap/ie/asn/E2nodeComponentConfigUpdateNGeNB.h"
#include "lib/ap/ie/asn/E2nodeComponentConfigUpdateENB.h"
#include "lib/ap/ie/asn/RICindication.h"
#include "lib/ap/ie/asn/RICsubscriptionRequest.h"
#include "lib/ap/ie/asn/RICsubsequentAction.h"
#include "lib/ap/ie/asn/RICcontrolAckRequest.h"

#include "lib/ap/global_consts.h"
#include "lib/ap/e2ap_ap.h"
#include "util/conversions.h"
#include "util/ngran_types.h"

#include "lib/ap/free/e2ap_msg_free.h"

static inline
byte_array_t copy_ostring_to_ba(OCTET_STRING_t src)
{
  byte_array_t dst = { .len = src.size}; 
  dst.buf = malloc(src.size);
  memcpy(dst.buf, src.buf, src.size);
  return dst;
}

static inline
byte_array_t copy_bs_to_ba(BIT_STRING_t src)
{
  byte_array_t dst = { .len = src.size}; 
  dst.buf = malloc(src.size);
  memcpy(dst.buf, src.buf, src.size);
  assert(src.bits_unused == 0 && "Not implemented otherwise");
  return dst;
}

static inline
ran_function_t copy_ran_function(const RANfunction_Item_t* src)
{
  ran_function_t dst;
  memset(&dst, 0, sizeof(ran_function_t));
  dst.id = src->ranFunctionID;
  dst.rev = src->ranFunctionRevision;
  dst.def = copy_ostring_to_ba(src->ranFunctionDefinition);
  if(src->ranFunctionOID != NULL){
    dst.oid = calloc(1, sizeof(byte_array_t)); 
    *dst.oid = copy_ostring_to_ba(*src->ranFunctionOID);
  }
  return dst;
}

static
int e2ap_asn1c_get_procedureCode(const E2AP_PDU_t* pdu)
{
  int rc = -1;
  switch(pdu->present)
  {
    case E2AP_PDU_PR_initiatingMessage:
      rc = pdu->choice.initiatingMessage->procedureCode;
      break;
    case E2AP_PDU_PR_successfulOutcome:
      rc = pdu->choice.successfulOutcome->procedureCode;
      break;
    case E2AP_PDU_PR_unsuccessfulOutcome:
      rc = pdu->choice.unsuccessfulOutcome->procedureCode;
      break;
    default:
      assert(0 && "Error: Unknown message type");
      break;
  }
  assert(rc != -1 && "Invalid procedure code");
  return rc;
}

static inline
e2_node_component_config_update_t copy_e2_node_component_conf_update(const E2nodeComponentConfigUpdate_Item_t* src)
{
  e2_node_component_config_update_t dst;
  memset(&dst, 0, sizeof(e2_node_component_config_update_t));
  // E2 Node Component Type. Mandatory
  assert(src->e2nodeComponentType < 6); 
  dst.e2_node_component_type = src->e2nodeComponentType; 
  // E2 Node Component Configuration Update. Mandatory
  switch (src->e2nodeComponentConfigUpdate.present) {
    case E2nodeComponentConfigUpdate_PR_gNBconfigUpdate:
      {
        dst.update_present = E2_NODE_COMPONENT_CONFIG_UPDATE_GNB_CONFIG_UPDATE;
        const E2nodeComponentConfigUpdateGNB_t* gNBconfigUpdate = src->e2nodeComponentConfigUpdate.choice.gNBconfigUpdate;
        if (gNBconfigUpdate->ngAPconfigUpdate != NULL){
          dst.gnb.ngap_gnb_cu_cp = malloc(sizeof(byte_array_t)); 
          *dst.gnb.ngap_gnb_cu_cp = copy_ostring_to_ba(*gNBconfigUpdate->ngAPconfigUpdate); 
        }
        if (gNBconfigUpdate->xnAPconfigUpdate != NULL){
          dst.gnb.xnap_gnb_cu_cp = malloc(sizeof(byte_array_t));
          *dst.gnb.xnap_gnb_cu_cp = copy_ostring_to_ba(*gNBconfigUpdate->xnAPconfigUpdate); 
        }
        if (gNBconfigUpdate->e1APconfigUpdate != NULL){
          dst.gnb.e1ap_gnb_cu_cp = malloc(sizeof(byte_array_t) ); 
          *dst.gnb.e1ap_gnb_cu_cp = copy_ostring_to_ba(*gNBconfigUpdate->e1APconfigUpdate); 
        }
        if (gNBconfigUpdate->f1APconfigUpdate != NULL){
          dst.gnb.f1ap_gnb_cu_cp = malloc(sizeof(byte_array_t));
          *dst.gnb.f1ap_gnb_cu_cp = copy_ostring_to_ba(*gNBconfigUpdate->f1APconfigUpdate);
        }
        break;
      }
    case E2nodeComponentConfigUpdate_PR_en_gNBconfigUpdate:
      {
        dst.update_present = E2_NODE_COMPONENT_CONFIG_UPDATE_EN_GNB_CONFIG_UPDATE; 
        const E2nodeComponentConfigUpdateENgNB_t* en_gNBconfigUpdate = src->e2nodeComponentConfigUpdate.choice.en_gNBconfigUpdate;
        if (en_gNBconfigUpdate->x2APconfigUpdate != NULL){
          dst.en_gnb.x2ap_en_gnb = malloc(sizeof(byte_array_t) );
          *dst.en_gnb.x2ap_en_gnb = copy_ostring_to_ba(*en_gNBconfigUpdate->x2APconfigUpdate);
        }
        break;
      }
    case E2nodeComponentConfigUpdate_PR_ng_eNBconfigUpdate:
      {
        dst.update_present = E2_NODE_COMPONENT_CONFIG_UPDATE_NG_ENB_CONFIG_UPDATE;
        const E2nodeComponentConfigUpdateNGeNB_t* ng_eNBconfigUpdate = src->e2nodeComponentConfigUpdate.choice.ng_eNBconfigUpdate;
        if (ng_eNBconfigUpdate->ngAPconfigUpdate != NULL){
          dst.ng_enb.ngap_ng_enb = malloc(sizeof(byte_array_t)); 
          *dst.ng_enb.ngap_ng_enb = copy_ostring_to_ba(*ng_eNBconfigUpdate->ngAPconfigUpdate);
        }
        if (ng_eNBconfigUpdate->xnAPconfigUpdate != NULL){
          dst.ng_enb.xnap_ng_enb = malloc(sizeof(byte_array_t)) ;
          *dst.ng_enb.xnap_ng_enb = copy_ostring_to_ba(*ng_eNBconfigUpdate->xnAPconfigUpdate); 
        }
        break;
      }
    case E2nodeComponentConfigUpdate_PR_eNBconfigUpdate:
      {
        dst.update_present = E2_NODE_COMPONENT_CONFIG_UPDATE_ENB_CONFIG_UPDATE;
        const E2nodeComponentConfigUpdateENB_t* eNBconfigUpdate = src->e2nodeComponentConfigUpdate.choice.eNBconfigUpdate;
        if (eNBconfigUpdate->s1APconfigUpdate != NULL){
          dst.enb.s1ap_enb = malloc(sizeof(byte_array_t));
          *dst.enb.s1ap_enb = copy_ostring_to_ba(*eNBconfigUpdate->s1APconfigUpdate);
        }
        if (eNBconfigUpdate->x2APconfigUpdate != NULL){
          dst.enb.x2ap_enb = malloc(sizeof(byte_array_t) );
          *dst.enb.x2ap_enb = copy_ostring_to_ba(*eNBconfigUpdate->x2APconfigUpdate); 
        }
        break;
      }
    default:
      assert(0!=0 && "Invalid code path");
  }
  return dst;
}


static inline
cause_t copy_cause(Cause_t src)
{
  cause_t dst; 
  memset(&dst, 0, sizeof(cause_t));
  switch(src.present) {
    case Cause_PR_NOTHING: {
                             assert(0 != 0 && "Not Implemented!");
                             break;
                           }
    case Cause_PR_ricRequest: {
                                assert(src.choice.ricRequest < 11);
                                dst.present = CAUSE_RICREQUEST;
                                dst.ricRequest = src.choice.ricRequest; 
                                break;
                              }
    case  Cause_PR_ricService: {
                                 assert(src.choice.ricService < 3);
                                 dst.present = CAUSE_RICSERVICE;
                                 dst.ricService = src.choice.ricService;
                                 break;
                               }
    case Cause_PR_transport:{
                              assert(src.choice.transport < 2);
                              dst.present = CAUSE_TRANSPORT;
                              dst.transport = src.choice.transport;
                              break;
                            }
    case  Cause_PR_protocol: {

                               assert(src.choice.protocol < 7);
                               dst.present = CAUSE_PROTOCOL;
                               dst.protocol = src.choice.protocol;
                               break;
                             }
    case  Cause_PR_misc:{
                          assert(src.choice.misc < 4);
                          dst.present = CAUSE_MISC;
                          dst.misc = src.choice.misc; 
                          break;
                        }
    default: {
               assert(0!= 0 && "Invalid code path. Error caused assigned");
               break;
             }
  }

  return dst;
}



///////////////////////////////////////////////////////////////////////////////////////////////////
// O-RAN E2APv01.01: Messages for Global Procedures ///////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// RIC -> E2
e2ap_msg_t e2ap_dec_subscription_request(const E2AP_PDU_t* pdu)
{
  assert(pdu != NULL);

  e2ap_msg_t ret;
  memset(&ret, 0, sizeof(e2ap_msg_t));

  assert(pdu->present == E2AP_PDU_PR_initiatingMessage); 
  assert(pdu->choice.initiatingMessage->procedureCode == ProcedureCode_id_RICsubscription); 
  // the O-RAN RIC wrongly sets the criticality -- we ignore it for
  // interoperatibility for the moment
  //assert(pdu->choice.initiatingMessage->criticality == Criticality_reject);
  assert(pdu->choice.initiatingMessage->value.present == InitiatingMessage__value_PR_RICsubscriptionRequest);

  ric_subscription_request_t* sr = &ret.u_msgs.ric_sub_req; 


  const RICsubscriptionRequest_t* out = &pdu->choice.initiatingMessage->value.choice.RICsubscriptionRequest;

  assert(out->protocolIEs.list.count == 3); 

  // RIC Request ID. Mandatory.
  RICsubscriptionRequest_IEs_t* sub_req  = out->protocolIEs.list.array[0];

  assert(sub_req->id == ProtocolIE_ID_id_RICrequestID);	
  assert(sub_req->criticality == Criticality_reject);
  assert(sub_req->value.present == RICsubscriptionRequest_IEs__value_PR_RICrequestID); 
  sr->ric_id.ric_inst_id = sub_req->value.choice.RICrequestID.ricInstanceID; 
  sr->ric_id.ric_req_id = sub_req->value.choice.RICrequestID.ricRequestorID;


  RICsubscriptionRequest_IEs_t* ran_id  = out->protocolIEs.list.array[1];

  assert(ran_id->id == ProtocolIE_ID_id_RANfunctionID);
  assert(ran_id->criticality == Criticality_reject);
  assert(ran_id->value.present == RICsubscriptionRequest_IEs__value_PR_RANfunctionID);
  // RAN Function ID. Mandatory
  sr->ric_id.ran_func_id = ran_id->value.choice.RANfunctionID;

  // RIC Subscription Details. Mandatory
  RICsubscriptionRequest_IEs_t* sub_det = out->protocolIEs.list.array[2];
  assert(sub_det->id == ProtocolIE_ID_id_RICsubscriptionDetails);
  assert(sub_det->criticality == Criticality_reject);
  assert(sub_det->value.present == RICsubscriptionRequest_IEs__value_PR_RICsubscriptionDetails); 

  //RIC Event Trigger Definition. Mandatory
  const int size_event_trigger = sub_det->value.choice.RICsubscriptionDetails.ricEventTriggerDefinition.size; 
  sr->event_trigger.buf = calloc(1,size_event_trigger); 
  memcpy(sr->event_trigger.buf, sub_det->value.choice.RICsubscriptionDetails.ricEventTriggerDefinition.buf, size_event_trigger);
  sr->event_trigger.len = size_event_trigger; 

  const int len_ra = sub_det->value.choice.RICsubscriptionDetails.ricAction_ToBeSetup_List.list.count;
  assert(len_ra > 0 && len_ra <= MAX_NUM_ACTION_DEF );

  sr->len_action = len_ra;
  sr->action = calloc(len_ra, sizeof(ric_action_t));

  RICaction_ToBeSetup_ItemIEs_t** arr = (RICaction_ToBeSetup_ItemIEs_t**) sub_det->value.choice.RICsubscriptionDetails.ricAction_ToBeSetup_List.list.array;
  //Sequence of Actions. Mandatory
  for(int i = 0; i < len_ra ; ++i){
    assert(arr[i]->id == ProtocolIE_ID_id_RICaction_ToBeSetup_Item);	 
    // the O-RAN RIC wrongly sets the criticality -- we ignore it for
    // interoperatibility for the moment
    //assert(arr[i]->criticality == Criticality_reject);
    assert(arr[i]->value.present == RICaction_ToBeSetup_ItemIEs__value_PR_RICaction_ToBeSetup_Item);

    const RICaction_ToBeSetup_Item_t* src = &arr[i]->value.choice.RICaction_ToBeSetup_Item; 
    ric_action_t * dst = &sr->action[i];
    //Sequence of Actions. Mandatory
    dst->id = src->ricActionID;
    dst->type = src->ricActionType;

    // RIC Action Definition. Optional 
    if(src->ricActionDefinition != NULL){
      dst->definition = calloc(1, sizeof(byte_array_t));
      *dst->definition = copy_ostring_to_ba(*src->ricActionDefinition);
    } 
    // RIC Subsequent Action. Optional 
    if(src->ricSubsequentAction != NULL){
      dst->subseq_action = calloc(1, sizeof(ric_subsequent_action_t));
      dst->subseq_action->type = src->ricSubsequentAction->ricSubsequentActionType;
      if(src->ricSubsequentAction->ricTimeToWait != 0){ // Optional but it is defined as a uint32_t .....
        dst->subseq_action->time_to_wait_ms = malloc(sizeof(uint32_t)); 
        *dst->subseq_action->time_to_wait_ms = src->ricSubsequentAction->ricTimeToWait ; 
      }
    }
  }
  return ret;
}


// xApp -> iApp
e2ap_msg_t e2ap_dec_e42_subscription_request(const struct E2AP_PDU* pdu)
{
  assert(pdu != NULL);

  e2ap_msg_t ret = {.type = E42_RIC_SUBSCRIPTION_REQUEST};

  assert(pdu->present == E2AP_PDU_PR_initiatingMessage); 
  assert(pdu->choice.initiatingMessage->procedureCode == ProcedureCode_id_E42RICsubscription); 
  // the O-RAN RIC wrongly sets the criticality -- we ignore it for
  // interoperatibility for the moment
  //assert(pdu->choice.initiatingMessage->criticality == Criticality_reject);
  assert(pdu->choice.initiatingMessage->value.present == InitiatingMessage__value_PR_E42RICsubscriptionRequest);

  e42_ric_subscription_request_t* e42_sr = &ret.u_msgs.e42_ric_sub_req; 

  const E42RICsubscriptionRequest_t* out = &pdu->choice.initiatingMessage->value.choice.E42RICsubscriptionRequest;


  ric_subscription_request_t* sr = &e42_sr->sr;

  assert(out->protocolIEs.list.count == 5); 

  // XAPP ID. Mandatory
  E42RICsubscriptionRequest_IEs_t* xapp_id = out->protocolIEs.list.array[0];
  assert(xapp_id->id == ProtocolIE_ID_id_XAPP_ID);
  assert(xapp_id->criticality == Criticality_reject);
  assert(xapp_id->value.present == E42RICsubscriptionRequest_IEs__value_PR_XAPP_ID);
  e42_sr->xapp_id = xapp_id->value.choice.XAPP_ID;

  // Global E2 Node ID. Mandatory
  E42RICsubscriptionRequest_IEs_t* node_src = out->protocolIEs.list.array[1];

  global_e2_node_id_t* id = &e42_sr->id;
  // Only gNodeB supported
  assert(node_src->id == ProtocolIE_ID_id_GlobalE2node_ID);
  assert(node_src->criticality == Criticality_reject);
  assert(node_src->value.present == E42RICsubscriptionRequest_IEs__value_PR_GlobalE2node_ID);
  assert(node_src->value.choice.GlobalE2node_ID.present == GlobalE2node_ID_PR_gNB);
  id->type = ngran_gNB;  

  GlobalE2node_gNB_ID_t *e2gnb = node_src->value.choice.GlobalE2node_ID.choice.gNB;
  assert(e2gnb->global_gNB_ID.gnb_id.present == GNB_ID_Choice_PR_gnb_ID);
  PLMNID_TO_MCC_MNC(&e2gnb->global_gNB_ID.plmn_id, id->plmn.mcc, id->plmn.mnc, id->plmn.mnc_digit_len);
  BIT_STRING_TO_MACRO_GNB_ID(&e2gnb->global_gNB_ID.gnb_id.choice.gnb_ID, id->nb_id);

  // RIC Request ID. Mandatory.
  E42RICsubscriptionRequest_IEs_t* sub_req  = out->protocolIEs.list.array[2];

  assert(sub_req->id == ProtocolIE_ID_id_RICrequestID);	
  assert(sub_req->criticality == Criticality_reject);
  assert(sub_req->value.present == E42RICsubscriptionRequest_IEs__value_PR_RICrequestID); 
  sr->ric_id.ric_inst_id = sub_req->value.choice.RICrequestID.ricInstanceID; 
  sr->ric_id.ric_req_id = sub_req->value.choice.RICrequestID.ricRequestorID;


  E42RICsubscriptionRequest_IEs_t* ran_id  = out->protocolIEs.list.array[3];

  assert(ran_id->id == ProtocolIE_ID_id_RANfunctionID);
  assert(ran_id->criticality == Criticality_reject);
  assert(ran_id->value.present == E42RICsubscriptionRequest_IEs__value_PR_RANfunctionID);
  // RAN Function ID. Mandatory
  sr->ric_id.ran_func_id = ran_id->value.choice.RANfunctionID;

  // RIC Subscription Details. Mandatory
  E42RICsubscriptionRequest_IEs_t* sub_det = out->protocolIEs.list.array[4];
  assert(sub_det->id == ProtocolIE_ID_id_RICsubscriptionDetails);
  assert(sub_det->criticality == Criticality_reject);
  assert(sub_det->value.present == E42RICsubscriptionRequest_IEs__value_PR_RICsubscriptionDetails); 

  //RIC Event Trigger Definition. Mandatory
  const int size_event_trigger = sub_det->value.choice.RICsubscriptionDetails.ricEventTriggerDefinition.size; 
  sr->event_trigger.buf = calloc(1,size_event_trigger); 
  memcpy(sr->event_trigger.buf, sub_det->value.choice.RICsubscriptionDetails.ricEventTriggerDefinition.buf, size_event_trigger);
  sr->event_trigger.len = size_event_trigger; 

  const int len_ra = sub_det->value.choice.RICsubscriptionDetails.ricAction_ToBeSetup_List.list.count;
  assert(len_ra > 0 && len_ra <= MAX_NUM_ACTION_DEF );

  sr->len_action = len_ra;
  sr->action = calloc(len_ra, sizeof(ric_action_t));

  RICaction_ToBeSetup_ItemIEs_t** arr = (RICaction_ToBeSetup_ItemIEs_t**) sub_det->value.choice.RICsubscriptionDetails.ricAction_ToBeSetup_List.list.array;
  //Sequence of Actions. Mandatory
  for(int i = 0; i < len_ra ; ++i){
    assert(arr[i]->id == ProtocolIE_ID_id_RICaction_ToBeSetup_Item);	 
    // the O-RAN RIC wrongly sets the criticality -- we ignore it for
    // interoperatibility for the moment
    //assert(arr[i]->criticality == Criticality_reject);
    assert(arr[i]->value.present == RICaction_ToBeSetup_ItemIEs__value_PR_RICaction_ToBeSetup_Item);

    const RICaction_ToBeSetup_Item_t* src = &arr[i]->value.choice.RICaction_ToBeSetup_Item; 
    ric_action_t * dst = &sr->action[i];
    //Sequence of Actions. Mandatory
    dst->id = src->ricActionID;
    dst->type = src->ricActionType;

    // RIC Action Definition. Optional 
    if(src->ricActionDefinition != NULL){
      dst->definition = calloc(1, sizeof(byte_array_t));
      *dst->definition = copy_ostring_to_ba(*src->ricActionDefinition);
    } 
    // RIC Subsequent Action. Optional 
    if(src->ricSubsequentAction != NULL){
      dst->subseq_action = calloc(1, sizeof(ric_subsequent_action_t));
      dst->subseq_action->type = src->ricSubsequentAction->ricSubsequentActionType;
      if(src->ricSubsequentAction->ricTimeToWait != 0){ // Optional but it is defined as a uint32_t .....
        dst->subseq_action->time_to_wait_ms = malloc(sizeof(uint32_t)); 
        *dst->subseq_action->time_to_wait_ms = src->ricSubsequentAction->ricTimeToWait ; 
      }
    }
  }
  return ret;
}

// E2 -> RIC 
e2ap_msg_t e2ap_dec_subscription_response(const E2AP_PDU_t* pdu)
{
  //printf("[E2AP] SUBSCRIPTION RESPONSE received\n");
  assert(pdu != NULL);

  e2ap_msg_t ret = {.type = RIC_SUBSCRIPTION_RESPONSE};
  ric_subscription_response_t* sr = &ret.u_msgs.ric_sub_resp;

  assert(pdu->present == E2AP_PDU_PR_successfulOutcome);
  assert(pdu->choice.successfulOutcome->procedureCode == ProcedureCode_id_RICsubscription);
  assert(pdu->choice.successfulOutcome->criticality == Criticality_reject);
  assert(pdu->choice.successfulOutcome->value.present == SuccessfulOutcome__value_PR_RICsubscriptionResponse);

  const RICsubscriptionResponse_t* out = &pdu->choice.successfulOutcome->value.choice.RICsubscriptionResponse;

  // RIC Request ID. Mandatory
  const RICsubscriptionResponse_IEs_t* req_id = out->protocolIEs.list.array[0];
  assert(req_id->id == ProtocolIE_ID_id_RICrequestID);
  assert(req_id->criticality == Criticality_reject);
  assert(req_id->value.present == RICsubscriptionResponse_IEs__value_PR_RICrequestID);
  assert(req_id->value.choice.RICrequestID.ricRequestorID < MAX_RIC_REQUEST_ID);
  assert(req_id->value.choice.RICrequestID.ricInstanceID <  MAX_RIC_INSTANCE_ID);
  sr->ric_id.ric_req_id = req_id->value.choice.RICrequestID.ricRequestorID;
  sr->ric_id.ric_inst_id = req_id->value.choice.RICrequestID.ricInstanceID; 


  // RAN Function ID. Mandatory
  const RICsubscriptionResponse_IEs_t* ran_func = out->protocolIEs.list.array[1]; 
  assert(ran_func->id == ProtocolIE_ID_id_RANfunctionID);
  assert(ran_func->criticality == Criticality_reject);
  assert(ran_func->value.present == RICsubscriptionResponse_IEs__value_PR_RANfunctionID);
  assert(ran_func->value.choice.RANfunctionID < MAX_NUM_RAN_FUNC_ID);
  sr->ric_id.ran_func_id = ran_func->value.choice.RANfunctionID;

  // RIC Action Admitted List
  const RICsubscriptionResponse_IEs_t* act_adm_list = out->protocolIEs.list.array[2];

  assert(act_adm_list->id == ProtocolIE_ID_id_RICactions_Admitted);
  assert(act_adm_list->criticality == Criticality_reject);
  assert(act_adm_list->value.present == RICsubscriptionResponse_IEs__value_PR_RICaction_Admitted_List);

  const size_t sz = act_adm_list->value.choice.RICaction_Admitted_List.list.count;
  sr->len_admitted = sz;
  sr->admitted = calloc(sz, sizeof(ric_action_admitted_t));

  for(size_t i = 0; i < sz; ++i){
    // RIC Action ID. Mandatory
    const RICaction_Admitted_ItemIEs_t *ai = (const RICaction_Admitted_ItemIEs_t *)act_adm_list->value.choice.RICaction_Admitted_List.list.array[i];

    assert(ai->id == ProtocolIE_ID_id_RICaction_Admitted_Item);
    assert(ai->criticality == Criticality_reject);
    assert(ai->value.present == RICaction_Admitted_ItemIEs__value_PR_RICaction_Admitted_Item);

    ric_action_admitted_t* dst = &sr->admitted[i];
    const RICaction_Admitted_Item_t* src = &ai->value.choice.RICaction_Admitted_Item;
    assert(src->ricActionID < MAX_RIC_ACTION_ID );
    dst->ric_act_id =  src->ricActionID;
  }

  // RIC Actions Not Admitted Lists
  const RICsubscriptionResponse_IEs_t* act_not_adm_list = out->protocolIEs.list.array[3];
  assert(act_not_adm_list->id == ProtocolIE_ID_id_RICactions_NotAdmitted);
  assert(act_not_adm_list->criticality == Criticality_reject);
  assert(act_not_adm_list->value.present == RICsubscriptionResponse_IEs__value_PR_RICaction_NotAdmitted_List);

  const size_t sz_not_ad = act_not_adm_list->value.choice.RICaction_NotAdmitted_List.list.count;
  sr->not_admitted = calloc(sz_not_ad, sizeof(ric_action_not_admitted_t));

  for(size_t i = 0; i < sz_not_ad; ++i){
    // RIC Action ID. Mandatory
    const RICaction_NotAdmitted_ItemIEs_t* nai = (const RICaction_NotAdmitted_ItemIEs_t*)act_not_adm_list->value.choice.RICaction_NotAdmitted_List.list.array[i];

    assert(nai->id == ProtocolIE_ID_id_RICaction_NotAdmitted_Item);	
    assert(nai->criticality == Criticality_reject);
    assert(nai->value.present == RICaction_NotAdmitted_ItemIEs__value_PR_RICaction_NotAdmitted_Item);
    ric_action_not_admitted_t* dst = &sr->not_admitted[i];  
    const RICaction_NotAdmitted_Item_t* src = &nai->value.choice.RICaction_NotAdmitted_Item;
    dst->ric_act_id = src->ricActionID;
    // Cause. Mandatory 
    switch( src->cause.present ) {
      case Cause_PR_NOTHING:{
                              assert(0 != 0 && "Not Implemented!");
                              break;
                            }
      case Cause_PR_ricRequest: {
                                  assert(src->cause.choice.ricRequest < 11);

                                  dst->cause.ricRequest = src->cause.choice.ricRequest; 
                                  dst->cause.present = CAUSE_RICREQUEST;
                                  break;
                                }
      case Cause_PR_ricService: {
                                  assert(src->cause.choice.ricService < 3);
                                  dst->cause.ricService = src->cause.choice.ricService;
                                  dst->cause.present = CAUSE_RICSERVICE;
                                  break;
                                }
      case Cause_PR_transport:{
                                assert(src->cause.choice.transport < 2);
                                dst->cause.transport = src->cause.choice.transport;
                                dst->cause.present = CAUSE_TRANSPORT;
                                break;
                              }
      case Cause_PR_protocol:{
                               assert(src->cause.choice.protocol < 7);
                               dst->cause.protocol = src->cause.choice.protocol;
                               dst->cause.present = CAUSE_PROTOCOL;
                               break;
                             }
      case Cause_PR_misc:{
                           assert(src->cause.choice.misc < 4);
                           dst->cause.misc = src->cause.choice.misc;
                           dst->cause.present = CAUSE_MISC;
                           break;
                         }
      default: {
                 assert(0!= 0 && "Invalid code path. Error caused assigned");
                 break;
               }
    }
  }

  return ret;
}

//E2 -> RIC 
e2ap_msg_t e2ap_dec_subscription_failure(const E2AP_PDU_t* pdu)
{
  assert(pdu != NULL);
  e2ap_msg_t ret = {.type = RIC_SUBSCRIPTION_FAILURE};
  ric_subscription_failure_t* sf = &ret.u_msgs.ric_sub_fail;
  assert(pdu->present == E2AP_PDU_PR_unsuccessfulOutcome);
  assert(pdu->choice.unsuccessfulOutcome->procedureCode == ProcedureCode_id_RICsubscription);
  assert(pdu->choice.unsuccessfulOutcome->criticality == Criticality_reject);
  assert(pdu->choice.unsuccessfulOutcome->value.present == UnsuccessfulOutcome__value_PR_RICsubscriptionFailure);

  const RICsubscriptionFailure_t *out = &pdu->choice.unsuccessfulOutcome->value.choice.RICsubscriptionFailure;

  // RIC Request ID. Mandatory
  const RICsubscriptionFailure_IEs_t* req_id = out->protocolIEs.list.array[0];
  assert(req_id->id == ProtocolIE_ID_id_RICrequestID);
  assert(req_id->criticality == Criticality_reject);
  assert(req_id->value.present == RICsubscriptionFailure_IEs__value_PR_RICrequestID);
  sf->ric_id.ric_req_id = req_id->value.choice.RICrequestID.ricRequestorID;
  sf->ric_id.ric_inst_id = req_id->value.choice.RICrequestID.ricInstanceID;

  // RAN Function ID. Mandatory 
  const RICsubscriptionFailure_IEs_t* ran_func = out->protocolIEs.list.array[1];
  assert(ran_func->id == ProtocolIE_ID_id_RANfunctionID);
  assert(ran_func->criticality == Criticality_reject);
  assert(ran_func->value.present == RICsubscriptionFailure_IEs__value_PR_RANfunctionID);
  assert(ran_func->value.choice.RANfunctionID < MAX_RAN_FUNC_ID);
  sf->ric_id.ran_func_id = ran_func->value.choice.RANfunctionID;

  // RIC Actions Not Admitted List
  const RICsubscriptionFailure_IEs_t* act_not_adm_list = out->protocolIEs.list.array[2];
  assert(act_not_adm_list->id == ProtocolIE_ID_id_RICactions_NotAdmitted);
  assert(act_not_adm_list->criticality == Criticality_reject);
  assert(act_not_adm_list->value.present == RICsubscriptionFailure_IEs__value_PR_RICaction_NotAdmitted_List);

  sf->len_na = act_not_adm_list->value.choice.RICaction_NotAdmitted_List.list.count;
  sf->not_admitted = calloc(sf->len_na,sizeof(ric_action_not_admitted_t));
  for(size_t i = 0; i < sf->len_na; ++i){
    // RIC Action ID. Mandatory  
    const RICaction_NotAdmitted_ItemIEs_t* nai = (const RICaction_NotAdmitted_ItemIEs_t*)act_not_adm_list->value.choice.RICaction_NotAdmitted_List.list.array[i];
    assert(nai->id == ProtocolIE_ID_id_RICaction_NotAdmitted_Item);	
    assert(nai->criticality == Criticality_reject);
    assert(nai->value.present == RICaction_NotAdmitted_ItemIEs__value_PR_RICaction_NotAdmitted_Item);
    const RICaction_NotAdmitted_Item_t* src =  &nai->value.choice.RICaction_NotAdmitted_Item;
    ric_action_not_admitted_t* dst = &sf->not_admitted[i];
    dst->ric_act_id = src->ricActionID;
    dst->cause = copy_cause(src->cause);
  }

 // Criticality Diagnosis. Optional
  if(out->protocolIEs.list.count > 3){
    const RICsubscriptionFailure_IEs_t* crit_diag = out->protocolIEs.list.array[3];
    assert(crit_diag->id == ProtocolIE_ID_id_CriticalityDiagnostics);
    assert(crit_diag->criticality == Criticality_reject);
    assert(crit_diag->value.present == RICsubscriptionFailure_IEs__value_PR_CriticalityDiagnostics); 
    assert(0!=0 && "Not Implemented");
  }

  return ret;
}

//RIC -> E2
e2ap_msg_t e2ap_dec_subscription_delete_request(const E2AP_PDU_t* pdu)
{
  assert(pdu != NULL);
  e2ap_msg_t ret = {.type = RIC_SUBSCRIPTION_DELETE_REQUEST};
  ric_subscription_delete_request_t* dr = &ret.u_msgs.ric_sub_del_req;

  // Message Type. Mandatory
  assert(pdu->present == E2AP_PDU_PR_initiatingMessage);
  assert(pdu->choice.initiatingMessage->procedureCode == ProcedureCode_id_RICsubscriptionDelete); 
  assert(pdu->choice.initiatingMessage->criticality == Criticality_reject);
  assert(pdu->choice.initiatingMessage->value.present == InitiatingMessage__value_PR_RICsubscriptionDeleteRequest); 

  const RICsubscriptionDeleteRequest_t* out = &pdu->choice.initiatingMessage->value.choice.RICsubscriptionDeleteRequest;

  // RIC Request ID. Mandatory
  const RICsubscriptionDeleteRequest_IEs_t* sub_req = out->protocolIEs.list.array[0];
  assert(sub_req->id == ProtocolIE_ID_id_RICrequestID);	
  assert(sub_req->criticality == Criticality_reject);
  assert(sub_req->value.present == RICsubscriptionDeleteRequest_IEs__value_PR_RICrequestID); 
  dr->ric_id.ric_inst_id = sub_req->value.choice.RICrequestID.ricInstanceID;
  dr->ric_id.ric_req_id = sub_req->value.choice.RICrequestID.ricRequestorID;

  // RIC Function ID. Mandatory
  const RICsubscriptionDeleteRequest_IEs_t* ran_id = out->protocolIEs.list.array[1];
  assert(ran_id->id == ProtocolIE_ID_id_RANfunctionID);
  assert(ran_id->criticality == Criticality_reject);
  assert(ran_id->value.present == RICsubscriptionDeleteRequest_IEs__value_PR_RANfunctionID);

  assert(ran_id->value.choice.RANfunctionID < MAX_RAN_FUNC_ID);
  dr->ric_id.ran_func_id = ran_id->value.choice.RANfunctionID;
  return ret;
}

// E2 -> RIC
e2ap_msg_t e2ap_dec_subscription_delete_response(const E2AP_PDU_t* pdu)
{
  assert(pdu != NULL);
  e2ap_msg_t ret = {.type = RIC_SUBSCRIPTION_DELETE_RESPONSE};
  ric_subscription_delete_response_t* dr = &ret.u_msgs.ric_sub_del_resp;

  assert(pdu->present == E2AP_PDU_PR_successfulOutcome);
  assert(pdu->choice.successfulOutcome->procedureCode == ProcedureCode_id_RICsubscriptionDelete);
  assert(pdu->choice.successfulOutcome->criticality == Criticality_reject);
  assert(pdu->choice.successfulOutcome->value.present == SuccessfulOutcome__value_PR_RICsubscriptionDeleteResponse);

  const RICsubscriptionDeleteResponse_t* out = &pdu->choice.successfulOutcome->value.choice.RICsubscriptionDeleteResponse;

  // RIC request ID. Mandatory
  const RICsubscriptionDeleteResponse_IEs_t* req_id = out->protocolIEs.list.array[0];
  assert(req_id->id == ProtocolIE_ID_id_RICrequestID);
  assert(req_id->criticality == Criticality_reject);
  assert(req_id->value.present == RICsubscriptionDeleteResponse_IEs__value_PR_RICrequestID);

  dr->ric_id.ric_req_id = req_id->value.choice.RICrequestID.ricRequestorID;  
  dr->ric_id.ric_inst_id = req_id->value.choice.RICrequestID.ricInstanceID;   

  // RAN Function ID. Mandatory
  const RICsubscriptionDeleteResponse_IEs_t* ran_func = out->protocolIEs.list.array[1]; 
  assert(ran_func->id == ProtocolIE_ID_id_RANfunctionID);
  assert(ran_func->criticality == Criticality_reject);
  assert(ran_func->value.present == RICsubscriptionDeleteResponse_IEs__value_PR_RANfunctionID);

  assert(ran_func->value.choice.RANfunctionID < MAX_RAN_FUNC_ID);
  dr->ric_id.ran_func_id = ran_func->value.choice.RANfunctionID; 
  return ret;
}

//E2 -> RIC
e2ap_msg_t e2ap_dec_subscription_delete_failure(const E2AP_PDU_t* pdu)
{
  assert(pdu != NULL);
  e2ap_msg_t ret = {.type = RIC_SUBSCRIPTION_DELETE_FAILURE};
  ric_subscription_delete_failure_t* df = &ret.u_msgs.ric_sub_del_fail;

  // Message Type. Mandatory
  assert(pdu->present == E2AP_PDU_PR_unsuccessfulOutcome);
  assert(pdu->choice.unsuccessfulOutcome->procedureCode == ProcedureCode_id_RICsubscriptionDelete);
  assert(pdu->choice.unsuccessfulOutcome->criticality == Criticality_reject);
  assert(pdu->choice.unsuccessfulOutcome->value.present == UnsuccessfulOutcome__value_PR_RICsubscriptionDeleteFailure);

  const RICsubscriptionDeleteFailure_t* out = &pdu->choice.unsuccessfulOutcome->value.choice.RICsubscriptionDeleteFailure;

  // RIC Request ID. Mandatory
  RICsubscriptionDeleteFailure_IEs_t* req_id = out->protocolIEs.list.array[0];
  assert(req_id->id == ProtocolIE_ID_id_RICrequestID);
  assert(req_id->criticality == Criticality_reject);
  assert(req_id->value.present ==  RICsubscriptionDeleteFailure_IEs__value_PR_RICrequestID);
 df->ric_id.ric_req_id = req_id->value.choice.RICrequestID.ricRequestorID; 
 df->ric_id.ric_inst_id = req_id->value.choice.RICrequestID.ricInstanceID;  
 
  // RAN Function ID. Mandatory
  RICsubscriptionDeleteFailure_IEs_t* ran_func = out->protocolIEs.list.array[1];
  assert(ran_func->id == ProtocolIE_ID_id_RANfunctionID);
  assert(ran_func->criticality == Criticality_reject);
  assert(ran_func->value.present == RICsubscriptionDeleteFailure_IEs__value_PR_RANfunctionID);
  assert(ran_func->value.choice.RANfunctionID < MAX_RAN_FUNC_ID);
 df->ric_id.ran_func_id = ran_func->value.choice.RANfunctionID; 
 
 // Cause. Mandatory 
  RICsubscriptionDeleteFailure_IEs_t* act_not_adm = out->protocolIEs.list.array[2];
  assert(act_not_adm->id == ProtocolIE_ID_id_RICactions_NotAdmitted);
  assert(act_not_adm->criticality == Criticality_reject);
  assert(act_not_adm->value.present == RICsubscriptionDeleteFailure_IEs__value_PR_Cause);
 df->cause = copy_cause(act_not_adm->value.choice.Cause);
 
 if(out->protocolIEs.list.count > 3){
 // Criticality Diagnosis. Optional
 RICsubscriptionDeleteFailure_IEs_t* crit_diag = out->protocolIEs.list.array[3];
  assert(crit_diag->id == ProtocolIE_ID_id_CriticalityDiagnostics);
  assert(crit_diag->criticality == Criticality_reject);
  assert(crit_diag->value.present == RICsubscriptionDeleteFailure_IEs__value_PR_CriticalityDiagnostics); 
  assert(0!=0 && "Not implemented");
//  crit_diag->value.choice.CriticalityDiagnostics = *df->crit_diagnose; 
 }
  return ret;
}


// E2 -> RIC
e2ap_msg_t e2ap_dec_indication(const E2AP_PDU_t* pdu)
{
  assert(pdu != NULL);

  e2ap_msg_t ret = {.type = RIC_INDICATION};
  ric_indication_t* ind = &ret.u_msgs.ric_ind;

  assert(pdu->present == E2AP_PDU_PR_initiatingMessage);
  assert(pdu->choice.initiatingMessage->procedureCode == ProcedureCode_id_RICindication);
  assert(pdu->choice.initiatingMessage->criticality == Criticality_reject);
  assert(pdu->choice.initiatingMessage->value.present == InitiatingMessage__value_PR_RICindication); 

  const RICindication_t* out = &pdu->choice.initiatingMessage->value.choice.RICindication;

  // RIC Request ID. Mandatory
  const RICindication_IEs_t* sub_req = out->protocolIEs.list.array[0];
  assert(sub_req->id == ProtocolIE_ID_id_RICrequestID);
  assert(sub_req->criticality == Criticality_reject);
  assert(sub_req->value.present == RICindication_IEs__value_PR_RICrequestID);

  assert(sub_req->value.choice.RICrequestID.ricInstanceID < MAX_RIC_INSTANCE_ID);
  ind->ric_id.ric_inst_id = sub_req->value.choice.RICrequestID.ricInstanceID;
  assert(sub_req->value.choice.RICrequestID.ricRequestorID < MAX_RIC_REQUEST_ID);
  ind->ric_id.ric_req_id = sub_req->value.choice.RICrequestID.ricRequestorID; 

  //RAN Function ID. Mandatory
  const RICindication_IEs_t* ran_id = out->protocolIEs.list.array[1];
  assert(ran_id->id == ProtocolIE_ID_id_RANfunctionID);
  assert(ran_id->criticality == Criticality_reject);
  assert(ran_id->value.present == RICindication_IEs__value_PR_RANfunctionID); 
  assert(ran_id->value.choice.RANfunctionID < MAX_RAN_FUNC_ID);
  ind->ric_id.ran_func_id = ran_id->value.choice.RANfunctionID;

  // RIC Action ID. Mandatory
  const RICindication_IEs_t* ric_act = out->protocolIEs.list.array[2];
  assert(ric_act->id == ProtocolIE_ID_id_RICactionID);
  assert(ric_act->criticality == Criticality_reject);
  assert(ric_act->value.present == RICindication_IEs__value_PR_RICactionID); 
  assert(ric_act->value.choice.RICactionID < MAX_RIC_ACTION_ID);
  ind->action_id = ric_act->value.choice.RICactionID; 


  int elm = 3;
  while(elm < out->protocolIEs.list.count){
    const RICindication_IEs_t* ric_ind = out->protocolIEs.list.array[elm];  
    const ProtocolIE_ID_t	id = ric_ind->id; 
    assert(id == ProtocolIE_ID_id_RICindicationSN ||
        id == ProtocolIE_ID_id_RICindicationType ||
        id ==  ProtocolIE_ID_id_RICindicationHeader ||
        id ==  ProtocolIE_ID_id_RICindicationMessage ||
        id ==  ProtocolIE_ID_id_RICcallProcessID );

    if(id == ProtocolIE_ID_id_RICindicationSN){
      //RIC indication SN. Optional
      assert(ric_ind->criticality == Criticality_reject);
      assert(ric_ind->value.present == RICindication_IEs__value_PR_RICindicationSN); 
      ind->sn =  malloc(sizeof(uint16_t));
      assert(ric_ind->value.choice.RICindicationSN < MAX_RIC_INDICATION_SN );
      *ind->sn = ric_ind->value.choice.RICindicationSN; 
    } else if (id == ProtocolIE_ID_id_RICindicationType){
      // RIC indication Type. Mandatory
      assert(ric_ind->criticality == Criticality_reject);
      assert(ric_ind->value.present == RICindication_IEs__value_PR_RICindicationType);
      assert(ric_ind->value.choice.RICindicationType < MAX_RIC_INDICATION_TYPE);
      ind->type = ric_ind->value.choice.RICindicationType; 
    } else if (id == ProtocolIE_ID_id_RICindicationHeader){
      //RIC indication header. Mandatory
      assert(ric_ind->id == ProtocolIE_ID_id_RICindicationHeader);
      assert(ric_ind->criticality == Criticality_reject);
      assert(ric_ind->value.present == RICindication_IEs__value_PR_RICindicationHeader);
      ind->hdr = copy_ostring_to_ba(ric_ind->value.choice.RICindicationHeader);
    } else if (id == ProtocolIE_ID_id_RICindicationMessage){
      // RIC indication message. Mandatory
      assert(ric_ind->criticality == Criticality_reject);
      assert(ric_ind->value.present == RICindication_IEs__value_PR_RICindicationMessage); 
      ind->msg = copy_ostring_to_ba(ric_ind->value.choice.RICindicationMessage);
    } else { // if (id == ProtocolIE_ID_id_RICcallProcessID)
      // RIC call process id. Optional
      assert(ric_ind->criticality == Criticality_reject);
      assert(ric_ind->value.present == RICindication_IEs__value_PR_RICcallProcessID);
      ind->call_process_id = malloc(sizeof(byte_array_t)); 
      *ind->call_process_id = copy_ostring_to_ba(ric_ind->value.choice.RICcallProcessID);  
    } 
    elm += 1;
  }

  return ret;
}


// RIC -> E2
e2ap_msg_t e2ap_dec_control_request(const E2AP_PDU_t* pdu)
{
  assert(pdu != NULL);

  e2ap_msg_t ret = {.type = RIC_CONTROL_REQUEST };
  ric_control_request_t* ctrl = &ret.u_msgs.ric_ctrl_req;

  assert(pdu->present == E2AP_PDU_PR_initiatingMessage);
  assert(pdu->choice.initiatingMessage->procedureCode == ProcedureCode_id_RICcontrol);
  // the O-RAN RIC wrongly sets the criticality -- we ignore it for
  // interoperatibility for the moment
  //assert(pdu->choice.initiatingMessage->criticality == Criticality_reject);
  assert(pdu->choice.initiatingMessage->value.present == InitiatingMessage__value_PR_RICcontrolRequest);

  const RICcontrolRequest_t *out = &pdu->choice.initiatingMessage->value.choice.RICcontrolRequest;

  // RIC Request ID. Mandatory
  const RICcontrolRequest_IEs_t *ric_req_id = out->protocolIEs.list.array[0];
  assert(ric_req_id->id == ProtocolIE_ID_id_RICrequestID);
  // the O-RAN RIC wrongly sets the criticality -- we ignore it for
  // interoperatibility for the moment
  //assert(ric_req_id->criticality == Criticality_reject);
  assert(ric_req_id->value.present == RICcontrolRequest_IEs__value_PR_RICrequestID);
  assert(ric_req_id->value.choice.RICrequestID.ricInstanceID < MAX_RIC_INSTANCE_ID);
  ctrl->ric_id.ric_inst_id = ric_req_id->value.choice.RICrequestID.ricInstanceID;
  assert(ric_req_id->value.choice.RICrequestID.ricRequestorID < MAX_RIC_REQUEST_ID);
  ctrl->ric_id.ric_req_id = ric_req_id->value.choice.RICrequestID.ricRequestorID;

  //RAN Function ID. Mandatory
  const RICcontrolRequest_IEs_t* ran_id = out->protocolIEs.list.array[1];
  assert(ran_id->id == ProtocolIE_ID_id_RANfunctionID);
  // the O-RAN RIC wrongly sets the criticality -- we ignore it for
  // interoperatibility for the moment
  //assert(ran_id->criticality == Criticality_reject);
  assert(ran_id->value.present == RICcontrolRequest_IEs__value_PR_RANfunctionID);
  assert(ran_id->value.choice.RANfunctionID < MAX_RAN_FUNC_ID);
  ctrl->ric_id.ran_func_id = ran_id->value.choice.RANfunctionID;

  int elm = 2;
  while (elm < out->protocolIEs.list.count) {
    const RICcontrolRequest_IEs_t* ric_ctrl = out->protocolIEs.list.array[elm];
    const ProtocolIE_ID_t	id = ric_ctrl->id;
    assert(id == ProtocolIE_ID_id_RICcallProcessID
        || id == ProtocolIE_ID_id_RICcontrolHeader
        || id == ProtocolIE_ID_id_RICcontrolMessage
        || id == ProtocolIE_ID_id_RICcontrolAckRequest);

    if (id == ProtocolIE_ID_id_RICcallProcessID) {
      // RIC Call Process ID. Optional
      // the O-RAN RIC wrongly sets the criticality -- we ignore it for
      // interoperatibility for the moment
      //assert(ric_ctrl->criticality == Criticality_reject);
      assert(ric_ctrl->value.present == RICcontrolRequest_IEs__value_PR_RICcallProcessID);
      ctrl->call_process_id = malloc(sizeof(*ctrl->call_process_id));
      assert(ctrl->call_process_id);
      *ctrl->call_process_id = copy_ostring_to_ba(ric_ctrl->value.choice.RICcallProcessID);
    } else if (id == ProtocolIE_ID_id_RICcontrolHeader) {
      // RIC Control Header. Mandatory
      // the O-RAN RIC wrongly sets the criticality -- we ignore it for
      // interoperatibility for the moment
      //assert(ric_ctrl->criticality == Criticality_reject);
      assert(ric_ctrl->value.present == RICcontrolRequest_IEs__value_PR_RICcontrolHeader);
      ctrl->hdr = copy_ostring_to_ba(ric_ctrl->value.choice.RICcontrolHeader);
    } else if (id == ProtocolIE_ID_id_RICcontrolMessage) {
      // RIC Control Message. Mandatory
      // the O-RAN RIC wrongly sets the criticality -- we ignore it for
      // interoperatibility for the moment
      //assert(ric_ctrl->criticality == Criticality_reject);
      assert(ric_ctrl->value.present == RICcontrolRequest_IEs__value_PR_RICcontrolMessage);
      ctrl->msg = copy_ostring_to_ba(ric_ctrl->value.choice.RICcontrolMessage);
    } else { // if (id == ProtocolIE_ID_id_RICcontrolAckRequest)
      // RIC Control Ack Request. Optional
      // the O-RAN RIC wrongly sets the criticality -- we ignore it for
      // interoperatibility for the moment
      //assert(ric_ctrl->criticality == Criticality_reject);
      assert(ric_ctrl->value.present == RICcontrolRequest_IEs__value_PR_RICcontrolAckRequest);
      ctrl->ack_req = malloc(sizeof(*ctrl->ack_req));
      assert(ctrl->ack_req);
      assert(ric_ctrl->value.choice.RICcontrolAckRequest < 3);
      *ctrl->ack_req = ric_ctrl->value.choice.RICcontrolAckRequest;
    }
    elm += 1;
  }

  return ret;
}



// RIC -> E2
e2ap_msg_t e2ap_dec_e42_control_request(const struct E2AP_PDU* pdu)
{
  assert(pdu != NULL);

  e2ap_msg_t ret = {.type = E42_RIC_CONTROL_REQUEST };

  e42_ric_control_request_t* e42_ctrl = &ret.u_msgs.e42_ric_ctrl_req;

  ric_control_request_t* ctrl = &e42_ctrl->ctrl_req;

  assert(pdu->present == E2AP_PDU_PR_initiatingMessage);
  assert(pdu->choice.initiatingMessage->procedureCode ==   ProcedureCode_id_E42RICcontrol);
  // the O-RAN RIC wrongly sets the criticality -- we ignore it for
  // interoperatibility for the moment
  //assert(pdu->choice.initiatingMessage->criticality == Criticality_reject);
  assert(pdu->choice.initiatingMessage->value.present == InitiatingMessage__value_PR_E42RICcontrolRequest);

  const E42RICcontrolRequest_t *out = &pdu->choice.initiatingMessage->value.choice.E42RICcontrolRequest;

  // XAPP ID. Mandatory
  E42RICcontrolRequest_IEs_t* xapp_id = out->protocolIEs.list.array[0];
  assert(xapp_id->id == ProtocolIE_ID_id_XAPP_ID);
  assert(xapp_id->criticality == Criticality_reject);
  assert(xapp_id->value.present == E42RICcontrolRequest_IEs__value_PR_XAPP_ID);
  e42_ctrl->xapp_id = xapp_id->value.choice.XAPP_ID;

  // Global E2 Node ID. Mandatory
  E42RICcontrolRequest_IEs_t* node_src = out->protocolIEs.list.array[1];

  global_e2_node_id_t* id = &e42_ctrl->id;
  // Only gNodeB supported
  assert(node_src->id == ProtocolIE_ID_id_GlobalE2node_ID);
  assert(node_src->criticality == Criticality_reject);
  assert(node_src->value.present == E42RICcontrolRequest_IEs__value_PR_GlobalE2node_ID);
  assert(node_src->value.choice.GlobalE2node_ID.present == GlobalE2node_ID_PR_gNB);
  id->type = ngran_gNB;  

  GlobalE2node_gNB_ID_t *e2gnb = node_src->value.choice.GlobalE2node_ID.choice.gNB;
  assert(e2gnb->global_gNB_ID.gnb_id.present == GNB_ID_Choice_PR_gnb_ID);
  PLMNID_TO_MCC_MNC(&e2gnb->global_gNB_ID.plmn_id, id->plmn.mcc, id->plmn.mnc, id->plmn.mnc_digit_len);
  BIT_STRING_TO_MACRO_GNB_ID(&e2gnb->global_gNB_ID.gnb_id.choice.gnb_ID, id->nb_id);

  // RIC Request ID. Mandatory
  const E42RICcontrolRequest_IEs_t *ric_req_id = out->protocolIEs.list.array[2];
  assert(ric_req_id->id == ProtocolIE_ID_id_RICrequestID);
  // the O-RAN RIC wrongly sets the criticality -- we ignore it for
  // interoperatibility for the moment
  //assert(ric_req_id->criticality == Criticality_reject);
  assert(ric_req_id->value.present == E42RICcontrolRequest_IEs__value_PR_RICrequestID);
  assert(ric_req_id->value.choice.RICrequestID.ricInstanceID < MAX_RIC_INSTANCE_ID);
  ctrl->ric_id.ric_inst_id = ric_req_id->value.choice.RICrequestID.ricInstanceID;
  assert(ric_req_id->value.choice.RICrequestID.ricRequestorID < MAX_RIC_REQUEST_ID);
  ctrl->ric_id.ric_req_id = ric_req_id->value.choice.RICrequestID.ricRequestorID;

  //RAN Function ID. Mandatory
  const E42RICcontrolRequest_IEs_t* ran_id = out->protocolIEs.list.array[3];
  assert(ran_id->id == ProtocolIE_ID_id_RANfunctionID);
  // the O-RAN RIC wrongly sets the criticality -- we ignore it for
  // interoperatibility for the moment
  //assert(ran_id->criticality == Criticality_reject);
  assert(ran_id->value.present == E42RICcontrolRequest_IEs__value_PR_RANfunctionID);
  assert(ran_id->value.choice.RANfunctionID < MAX_RAN_FUNC_ID);
  ctrl->ric_id.ran_func_id = ran_id->value.choice.RANfunctionID;

  int elm = 4;
  while (elm < out->protocolIEs.list.count) {
    const E42RICcontrolRequest_IEs_t* ric_ctrl = out->protocolIEs.list.array[elm];
    const ProtocolIE_ID_t	id = ric_ctrl->id;
    assert(id == ProtocolIE_ID_id_RICcallProcessID
        || id == ProtocolIE_ID_id_RICcontrolHeader
        || id == ProtocolIE_ID_id_RICcontrolMessage
        || id == ProtocolIE_ID_id_RICcontrolAckRequest);

    if (id == ProtocolIE_ID_id_RICcallProcessID) {
      // RIC Call Process ID. Optional
      // the O-RAN RIC wrongly sets the criticality -- we ignore it for
      // interoperatibility for the moment
      //assert(ric_ctrl->criticality == Criticality_reject);
      assert(ric_ctrl->value.present == E42RICcontrolRequest_IEs__value_PR_RICcallProcessID);
      ctrl->call_process_id = malloc(sizeof(*ctrl->call_process_id));
      assert(ctrl->call_process_id);
      *ctrl->call_process_id = copy_ostring_to_ba(ric_ctrl->value.choice.RICcallProcessID);
    } else if (id == ProtocolIE_ID_id_RICcontrolHeader) {
      // RIC Control Header. Mandatory
      // the O-RAN RIC wrongly sets the criticality -- we ignore it for
      // interoperatibility for the moment
      //assert(ric_ctrl->criticality == Criticality_reject);
      assert(ric_ctrl->value.present == E42RICcontrolRequest_IEs__value_PR_RICcontrolHeader);
      ctrl->hdr = copy_ostring_to_ba(ric_ctrl->value.choice.RICcontrolHeader);
    } else if (id == ProtocolIE_ID_id_RICcontrolMessage) {
      // RIC Control Message. Mandatory
      // the O-RAN RIC wrongly sets the criticality -- we ignore it for
      // interoperatibility for the moment
      //assert(ric_ctrl->criticality == Criticality_reject);
      assert(ric_ctrl->value.present == E42RICcontrolRequest_IEs__value_PR_RICcontrolMessage);
      ctrl->msg = copy_ostring_to_ba(ric_ctrl->value.choice.RICcontrolMessage);
    } else { // if (id == ProtocolIE_ID_id_RICcontrolAckRequest)
      // RIC Control Ack Request. Optional
      // the O-RAN RIC wrongly sets the criticality -- we ignore it for
      // interoperatibility for the moment
      //assert(ric_ctrl->criticality == Criticality_reject);
      assert(ric_ctrl->value.present == E42RICcontrolRequest_IEs__value_PR_RICcontrolAckRequest);
      ctrl->ack_req = malloc(sizeof(*ctrl->ack_req));
      assert(ctrl->ack_req);
      assert(ric_ctrl->value.choice.RICcontrolAckRequest < 3);
      *ctrl->ack_req = ric_ctrl->value.choice.RICcontrolAckRequest;
    }
    elm += 1;
  }

  return ret;
}

// E2 -> RIC
e2ap_msg_t e2ap_dec_control_ack(const E2AP_PDU_t* pdu)
{
  assert(pdu != NULL);

  e2ap_msg_t ret = {.type = RIC_CONTROL_ACKNOWLEDGE};
  ric_control_acknowledge_t* ctrl = &ret.u_msgs.ric_ctrl_ack;

  assert(pdu->present == E2AP_PDU_PR_successfulOutcome);
  assert(pdu->choice.successfulOutcome->procedureCode == ProcedureCode_id_RICcontrol);
  assert(pdu->choice.successfulOutcome->criticality == Criticality_reject);
  assert(pdu->choice.successfulOutcome->value.present ==  SuccessfulOutcome__value_PR_RICcontrolAcknowledge);

  const RICcontrolAcknowledge_t* out = &pdu->choice.successfulOutcome->value.choice.RICcontrolAcknowledge;

  //RIC Request ID. Mandatory
  const RICcontrolAcknowledge_IEs_t* ric_req_id = out->protocolIEs.list.array[0];
  assert(ric_req_id->id == ProtocolIE_ID_id_RICrequestID);
  assert(ric_req_id->criticality == Criticality_reject);
  assert(ric_req_id->value.present ==  RICcontrolAcknowledge_IEs__value_PR_RICrequestID);
  assert(ric_req_id->value.choice.RICrequestID.ricInstanceID < MAX_RIC_INSTANCE_ID);
  ctrl->ric_id.ric_inst_id = ric_req_id->value.choice.RICrequestID.ricInstanceID;
  assert(ric_req_id->value.choice.RICrequestID.ricRequestorID < MAX_RIC_REQUEST_ID);
  ctrl->ric_id.ric_req_id = ric_req_id->value.choice.RICrequestID.ricRequestorID;

  //RAN Function ID. Mandatory
  const RICcontrolAcknowledge_IEs_t* ran_id = out->protocolIEs.list.array[1];
  assert(ran_id->id == ProtocolIE_ID_id_RANfunctionID);
  assert(ran_id->criticality == Criticality_reject);
  assert(ran_id->value.present == RICcontrolAcknowledge_IEs__value_PR_RANfunctionID);
  assert(ran_id->value.choice.RANfunctionID < MAX_RAN_FUNC_ID);
  ctrl->ric_id.ran_func_id = ran_id->value.choice.RANfunctionID;

  int elm = 2;
  while(elm < out->protocolIEs.list.count){
    const RICcontrolAcknowledge_IEs_t* ric_ctrl = out->protocolIEs.list.array[elm];
    const ProtocolIE_ID_t	id = ric_ctrl->id;
    assert(id == ProtocolIE_ID_id_RICcallProcessID
        || id ==  ProtocolIE_ID_id_RICcontrolStatus
        || id ==  ProtocolIE_ID_id_RICcontrolOutcome);
  //RIC Call process ID. Optional
    if(id == ProtocolIE_ID_id_RICcallProcessID){
     assert(ric_ctrl->criticality == Criticality_reject);
     assert(ric_ctrl->value.present == RICcontrolAcknowledge_IEs__value_PR_RICcallProcessID);
      ctrl->call_process_id = malloc(sizeof(byte_array_t));
      *ctrl->call_process_id = copy_ostring_to_ba(ric_ctrl->value.choice.RICcallProcessID); 
  //RIC Control Status. Mandatory
    } else if (id == ProtocolIE_ID_id_RICcontrolStatus){
      assert(ric_ctrl->criticality == Criticality_reject);
     assert(ric_ctrl->value.present ==  RICcontrolAcknowledge_IEs__value_PR_RICcontrolStatus);
     ctrl->status =ric_ctrl->value.choice.RICcontrolStatus;   
  //RIC Control Outcome. Optional
    } else { // id == ProtocolIE_ID_id_RICcontrolOutcome
      assert(ric_ctrl->criticality == Criticality_reject);
      assert(ric_ctrl->value.present == RICcontrolAcknowledge_IEs__value_PR_RICcontrolOutcome);
      ctrl->control_outcome = malloc(sizeof(byte_array_t));
      *ctrl->control_outcome = copy_ostring_to_ba(ric_ctrl->value.choice.RICcontrolOutcome); 
    }
    elm +=1;
  }
  return ret;
}


// E2 -> RIC
e2ap_msg_t e2ap_dec_control_failure(const E2AP_PDU_t* pdu)
{
  assert(pdu != NULL);
  e2ap_msg_t ret = {.type = RIC_CONTROL_FAILURE};
  ric_control_failure_t* cf = &ret.u_msgs.ric_ctrl_fail;

  //Message Type. Mandatory
  assert(pdu->present == E2AP_PDU_PR_unsuccessfulOutcome);
  assert(pdu->choice.unsuccessfulOutcome->procedureCode == ProcedureCode_id_RICcontrol);
  assert(pdu->choice.unsuccessfulOutcome->criticality == Criticality_reject);
  assert(pdu->choice.unsuccessfulOutcome->value.present == UnsuccessfulOutcome__value_PR_RICcontrolFailure); 

  const RICcontrolFailure_t* out = &pdu->choice.unsuccessfulOutcome->value.choice.RICcontrolFailure;

  //RIC Request ID. Mandatory
  const RICcontrolFailure_IEs_t* sub_req = out->protocolIEs.list.array[0];
  assert(sub_req->id == ProtocolIE_ID_id_RICrequestID);
  assert(sub_req->criticality == Criticality_reject);
  assert(sub_req->value.present == RICcontrolFailure_IEs__value_PR_RICrequestID);
  cf->ric_id.ric_inst_id = sub_req->value.choice.RICrequestID.ricInstanceID;
  cf->ric_id.ric_req_id = sub_req->value.choice.RICrequestID.ricRequestorID;
 
  //RAN Function ID. Mandatory
  RICcontrolFailure_IEs_t* ran_id = out->protocolIEs.list.array[1];
  assert(ran_id->id == ProtocolIE_ID_id_RANfunctionID);
  assert(ran_id->criticality ==  Criticality_reject);
  assert(ran_id->value.present == RICcontrolFailure_IEs__value_PR_RANfunctionID);
  assert(ran_id->value.choice.RANfunctionID < MAX_NUM_RAN_FUNC_ID);
  cf->ric_id.ran_func_id = ran_id->value.choice.RANfunctionID ;

  int elm = 2;
  while(elm < out->protocolIEs.list.count){
    RICcontrolFailure_IEs_t * cf_ie = out->protocolIEs.list.array[elm];
    assert(cf_ie->id == ProtocolIE_ID_id_RICcallProcessID
            || cf_ie->id == ProtocolIE_ID_id_Cause
            || cf_ie->id ==  ProtocolIE_ID_id_RICcontrolOutcome); 

    if(cf_ie->id == ProtocolIE_ID_id_RICcallProcessID){
      //RIC Call process ID. Optional
      assert(cf_ie->criticality == Criticality_reject);
      assert(cf_ie->value.present == RICcontrolFailure_IEs__value_PR_RICcallProcessID);
      cf->call_process_id = calloc(1, sizeof(byte_array_t));
      *cf->call_process_id = copy_ostring_to_ba(cf_ie->value.choice.RICcallProcessID);
    } else if (cf_ie->id == ProtocolIE_ID_id_Cause){
      //Cause. Mandatory
      assert(cf_ie->criticality == Criticality_reject);
      assert(cf_ie->value.present == RICcontrolFailure_IEs__value_PR_Cause); 
      cf->cause = copy_cause(cf_ie->value.choice.Cause); 
    } else { //if (cf_ie->id ==  ProtocolIE_ID_id_RICcontrolOutcome)
      //RIC Control Outcome. Optional
      assert(cf_ie->criticality == Criticality_reject);
      assert(cf_ie->value.present == RICcontrolFailure_IEs__value_PR_RICcontrolOutcome);
      cf->control_outcome = calloc(1, sizeof(byte_array_t));
      *cf->control_outcome = copy_ostring_to_ba(cf_ie->value.choice.RICcontrolOutcome);
    }
    elm += 1;
  }
  return ret;
}
  

///////////////////////////////////////////////////////////////////////////////////////////////////
// O-RAN E2APv01.01: Messages for Global Procedures ///////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// RIC <-> E2 
e2ap_msg_t e2ap_dec_error_indication(const E2AP_PDU_t* pdu)
{
  assert(pdu != NULL);
  e2ap_msg_t ret = {.type = E2AP_ERROR_INDICATION};
  e2ap_error_indication_t* dst = &ret.u_msgs.err_ind;
  // Message Type. Mandatory
  assert(pdu->present == E2AP_PDU_PR_initiatingMessage);
  assert(pdu->choice.initiatingMessage->procedureCode == ProcedureCode_id_ErrorIndication); 
  assert(pdu->choice.initiatingMessage->criticality == Criticality_reject);
  assert(pdu->choice.initiatingMessage->value.present == InitiatingMessage__value_PR_ErrorIndication); 

  const ErrorIndication_t* out = &pdu->choice.initiatingMessage->value.choice.ErrorIndication;

  int elm = 0;
  while (elm < out->protocolIEs.list.count){
    const ErrorIndication_IEs_t* err_ind = out->protocolIEs.list.array[elm];
    assert(err_ind->id == ProtocolIE_ID_id_RICrequestID
          || err_ind->id == ProtocolIE_ID_id_RANfunctionID
          || err_ind->id == ProtocolIE_ID_id_Cause
          || err_ind->id == ProtocolIE_ID_id_CriticalityDiagnostics
        );
      if(err_ind->id == ProtocolIE_ID_id_RICrequestID){
        //RIC Request ID. Mandatory
        assert(err_ind->criticality == Criticality_reject);
        assert(err_ind->value.present == ErrorIndication_IEs__value_PR_RICrequestID); 
        dst->ric_id = calloc(1, sizeof(ric_gen_id_t));
        dst->ric_id->ric_inst_id = err_ind->value.choice.RICrequestID.ricInstanceID; 
        dst->ric_id->ric_req_id = err_ind->value.choice.RICrequestID.ricRequestorID; 
      } else if (err_ind->id == ProtocolIE_ID_id_RANfunctionID){
        // RAN Function ID. Optional
        assert(err_ind->id == ProtocolIE_ID_id_RANfunctionID);
        assert(err_ind->criticality == Criticality_reject);
        assert(err_ind->value.present == ErrorIndication_IEs__value_PR_RANfunctionID); 
        assert(err_ind->value.choice.RANfunctionID < MAX_NUM_RAN_FUNC_ID);
        dst->ric_id->ran_func_id = err_ind->value.choice.RANfunctionID; 
      } else if (err_ind->id == ProtocolIE_ID_id_Cause){
        // Cause. Optional
        assert(err_ind->criticality == Criticality_reject);
        assert(err_ind->value.present == ErrorIndication_IEs__value_PR_Cause); 
        dst->cause = calloc(1, sizeof(cause_t));
        *dst->cause = copy_cause(err_ind->value.choice.Cause);
      } else {// if (err_ind->id == ProtocolIE_ID_id_CriticalityDiagnostics)
        assert(err_ind->criticality == Criticality_reject);
        assert(err_ind->value.present == ErrorIndication_IEs__value_PR_CriticalityDiagnostics); 
        assert(0!=0 && "Not implemented");
      }
    elm += 1;
  }
  return ret;
}


// E2 -> RIC
e2ap_msg_t e2ap_dec_setup_request(const E2AP_PDU_t* pdu)
{
  assert(pdu != NULL);
  assert(pdu->present == E2AP_PDU_PR_initiatingMessage);
  assert(pdu->choice.initiatingMessage->procedureCode == ProcedureCode_id_E2setup);
  assert(pdu->choice.initiatingMessage->criticality == Criticality_reject);
  assert(pdu->choice.initiatingMessage->value.present == InitiatingMessage__value_PR_E2setupRequest);

  e2ap_msg_t ret = {.type = E2_SETUP_REQUEST};
  e2_setup_request_t* sr = &ret.u_msgs.e2_stp_req;

  const E2setupRequest_t *out = &pdu->choice.initiatingMessage->value.choice.E2setupRequest;
  assert(out->protocolIEs.list.count > 0 && out->protocolIEs.list.count < 4);

  E2setupRequestIEs_t* setup_rid = out->protocolIEs.list.array[0];

  // Only gNodeB supported
  assert(setup_rid->id == ProtocolIE_ID_id_GlobalE2node_ID);
  assert(setup_rid->criticality == Criticality_reject);
  assert(setup_rid->value.present == E2setupRequestIEs__value_PR_GlobalE2node_ID);
  assert(setup_rid->value.choice.GlobalE2node_ID.present == GlobalE2node_ID_PR_gNB);
  sr->id.type = ngran_gNB;  

  GlobalE2node_gNB_ID_t *e2gnb = setup_rid->value.choice.GlobalE2node_ID.choice.gNB;
  assert(e2gnb->global_gNB_ID.gnb_id.present == GNB_ID_Choice_PR_gnb_ID);
  PLMNID_TO_MCC_MNC(&e2gnb->global_gNB_ID.plmn_id, sr->id.plmn.mcc, sr->id.plmn.mnc, sr->id.plmn.mnc_digit_len);
  BIT_STRING_TO_MACRO_GNB_ID(&e2gnb->global_gNB_ID.gnb_id.choice.gnb_ID,sr->id.nb_id);

  int elm_id = out->protocolIEs.list.count - 1;
  assert(elm_id > -1 && elm_id < 3);

  while(elm_id != 0){
    const ProtocolIE_ID_t proto_id = out->protocolIEs.list.array[elm_id]->id;  
    assert(proto_id == ProtocolIE_ID_id_RANfunctionsAdded || proto_id == ProtocolIE_ID_id_E2nodeComponentConfigUpdate);
    // List of RAN Functions Added
    if(proto_id == ProtocolIE_ID_id_RANfunctionsAdded){
      E2setupRequestIEs_t* ran_list = out->protocolIEs.list.array[elm_id];
      assert(ran_list->id == ProtocolIE_ID_id_RANfunctionsAdded);
      assert(ran_list->criticality == Criticality_reject);
      assert(ran_list->value.present == E2setupRequestIEs__value_PR_RANfunctions_List);

      assert(ran_list->value.choice.RANfunctions_List.list.count < MAX_NUM_RAN_FUNC_ID);
      sr->len_rf = ran_list->value.choice.RANfunctions_List.list.count;

      sr->ran_func_item = calloc(sr->len_rf, sizeof(ran_function_t));
      RANfunction_ItemIEs_t** arr = (RANfunction_ItemIEs_t**)ran_list->value.choice.RANfunctions_List.list.array;
      for(size_t i =0; i < sr->len_rf; ++i){
        ran_function_t* dst = &sr->ran_func_item[i];
        const RANfunction_Item_t* src = &arr[i]->value.choice.RANfunction_Item;

        assert(src->ranFunctionID <= MAX_RAN_FUNC_ID);
        dst->id = src->ranFunctionID; 

        assert(src->ranFunctionRevision > -1 && src->ranFunctionRevision <= MAX_RAN_FUNC_REV);
        dst->rev = src->ranFunctionRevision;

        dst->def = copy_ostring_to_ba(src->ranFunctionDefinition); 

        if(src->ranFunctionOID != NULL){
          dst->oid = malloc(sizeof(byte_array_t));
          *dst->oid = copy_ostring_to_ba(*src->ranFunctionOID);
        }
      }
    } else { //if(proto_id == ProtocolIE_ID_id_E2nodeComponentConfigUpdate)
      const E2setupRequestIEs_t* comp_update = out->protocolIEs.list.array[elm_id];
      assert(comp_update->criticality == Criticality_reject);
      assert(comp_update->value.present == E2setupRequestIEs__value_PR_E2nodeComponentConfigUpdate_List); 

      assert(comp_update->value.choice.E2nodeComponentConfigUpdate_List.list.count < MAX_NUM_E2_NODE_COMPONENTS);
      const size_t sz = comp_update->value.choice.E2nodeComponentConfigUpdate_List.list.count; 
      sr->len_ccu = sz;
      sr->comp_conf_update = calloc(sz, sizeof(e2_node_component_config_update_t));

      E2nodeComponentConfigUpdate_ItemIEs_t** arr = (E2nodeComponentConfigUpdate_ItemIEs_t**)comp_update->value.choice.E2nodeComponentConfigUpdate_List.list.array;
      for(size_t i = 0; i < sz; ++i){
        assert(arr[i]->id == ProtocolIE_ID_id_E2nodeComponentConfigUpdate_Item);
        assert(arr[i]->criticality == Criticality_reject); 
        assert(arr[i]->value.present == E2nodeComponentConfigUpdate_ItemIEs__value_PR_E2nodeComponentConfigUpdate_Item);
        const E2nodeComponentConfigUpdate_Item_t* src = &arr[i]->value.choice.E2nodeComponentConfigUpdate_Item;
        e2_node_component_config_update_t* dst = &sr->comp_conf_update[i]; 
        sr->comp_conf_update[i] = copy_e2_node_component_conf_update(src);
        /* 
        // E2 Node Component Type. Mandatory
        assert(src->e2nodeComponentType < 6); 
        dst->e2_node_component_type = src->e2nodeComponentType; 
        // E2 Node Component Configuration Update. Mandatory
        switch (src->e2nodeComponentConfigUpdate.present) {
        case E2nodeComponentConfigUpdate_PR_gNBconfigUpdate:
        {
        dst->update_present = E2_NODE_COMPONENT_CONFIG_UPDATE_GNB_CONFIG_UPDATE;
        const E2nodeComponentConfigUpdateGNB_t* gNBconfigUpdate = src->e2nodeComponentConfigUpdate.choice.gNBconfigUpdate;
        if (gNBconfigUpdate->ngAPconfigUpdate != NULL){
        dst->gnb.ngap_gnb_cu_cp = malloc(sizeof(byte_array_t)); 
         *dst->gnb.ngap_gnb_cu_cp = copy_ostring_to_ba(*gNBconfigUpdate->ngAPconfigUpdate); 
         }
         if (gNBconfigUpdate->xnAPconfigUpdate != NULL){
         dst->gnb.xnap_gnb_cu_cp = malloc(sizeof(byte_array_t));
         *dst->gnb.xnap_gnb_cu_cp = copy_ostring_to_ba(*gNBconfigUpdate->xnAPconfigUpdate); 
         }
         if (gNBconfigUpdate->e1APconfigUpdate != NULL){
         dst->gnb.e1ap_gnb_cu_cp = malloc(sizeof(byte_array_t) ); 
         *dst->gnb.e1ap_gnb_cu_cp = copy_ostring_to_ba(*gNBconfigUpdate->e1APconfigUpdate); 
         }
         if (gNBconfigUpdate->f1APconfigUpdate != NULL){
         dst->gnb.f1ap_gnb_cu_cp = malloc(sizeof(byte_array_t));
         *dst->gnb.f1ap_gnb_cu_cp = copy_ostring_to_ba(*gNBconfigUpdate->f1APconfigUpdate);
         }
         break;
         }
         case E2nodeComponentConfigUpdate_PR_en_gNBconfigUpdate:
         {
         dst->update_present = E2_NODE_COMPONENT_CONFIG_UPDATE_EN_GNB_CONFIG_UPDATE; 
         const E2nodeComponentConfigUpdateENgNB_t* en_gNBconfigUpdate = src->e2nodeComponentConfigUpdate.choice.en_gNBconfigUpdate;
         if (en_gNBconfigUpdate->x2APconfigUpdate != NULL){
         dst->en_gnb.x2ap_en_gnb = malloc(sizeof(byte_array_t) );
         *dst->en_gnb.x2ap_en_gnb = copy_ostring_to_ba(*en_gNBconfigUpdate->x2APconfigUpdate);
         }
         break;
         }
         case E2nodeComponentConfigUpdate_PR_ng_eNBconfigUpdate:
         {
         dst->update_present = E2_NODE_COMPONENT_CONFIG_UPDATE_NG_ENB_CONFIG_UPDATE;
         const E2nodeComponentConfigUpdateNGeNB_t* ng_eNBconfigUpdate = src->e2nodeComponentConfigUpdate.choice.ng_eNBconfigUpdate;
         if (ng_eNBconfigUpdate->ngAPconfigUpdate != NULL){
         dst->ng_enb.ngap_ng_enb = malloc(sizeof(byte_array_t)); 
         *dst->ng_enb.ngap_ng_enb = copy_ostring_to_ba(*ng_eNBconfigUpdate->ngAPconfigUpdate);
         }
         if (ng_eNBconfigUpdate->xnAPconfigUpdate != NULL){
         dst->ng_enb.xnap_ng_enb = malloc(sizeof(byte_array_t)) ;
         *dst->ng_enb.xnap_ng_enb = copy_ostring_to_ba(*ng_eNBconfigUpdate->xnAPconfigUpdate); 
         }
         break;
         }
         case E2nodeComponentConfigUpdate_PR_eNBconfigUpdate:
         {
         dst->update_present = E2_NODE_COMPONENT_CONFIG_UPDATE_ENB_CONFIG_UPDATE;
         const E2nodeComponentConfigUpdateENB_t* eNBconfigUpdate = src->e2nodeComponentConfigUpdate.choice.eNBconfigUpdate;
         if (eNBconfigUpdate->s1APconfigUpdate != NULL){
         dst->enb.s1ap_enb = malloc(sizeof(byte_array_t));
         *dst->enb.s1ap_enb = copy_ostring_to_ba(*eNBconfigUpdate->s1APconfigUpdate);
         }
         if (eNBconfigUpdate->x2APconfigUpdate != NULL){
         dst->enb.x2ap_enb = malloc(sizeof(byte_array_t) );
         *dst->enb.x2ap_enb = copy_ostring_to_ba(*eNBconfigUpdate->x2APconfigUpdate); 
         }
         break;
         }
         default:
         assert(0!=0 && "Invalid code path");
         }
         */

        // E2 Node Component ID. Optional
        if (src->e2nodeComponentID != NULL){
          dst->id_present = malloc(sizeof(*dst->id_present));
          switch (src->e2nodeComponentID->present) {
            case E2nodeComponentID_PR_e2nodeComponentTypeGNB_CU_UP:
              *dst->id_present = E2_NODE_COMPONENT_ID_E2_NODE_COMPONENT_TYPE_GNB_CU_UP;
              // Is this what we want?
              asn_INTEGER2ulong(&src->e2nodeComponentID->choice.e2nodeComponentTypeGNB_CU_UP->	 gNB_CU_UP_ID, &dst->gnb_cu_up_id);
              break;
            case E2nodeComponentID_PR_e2nodeComponentTypeGNB_DU:
              *dst->id_present = E2_NODE_COMPONENT_ID_E2_NODE_COMPONENT_TYPE_GNB_DU; 
              asn_INTEGER2ulong(&src->e2nodeComponentID->choice.e2nodeComponentTypeGNB_DU->gNB_DU_ID, &dst->gnb_du_id);
              break;
            default:
              assert(0!=0 && "Invalid code path");
          }
        }
      } 
    }     
    elm_id -=1; 
  }
  //printf("e2ap_dec_setup_request_asn called \n");
  return ret;
}

static
e2ap_msg_t e2ap_dec_setup_response_unsuccess(const E2AP_PDU_t* pdu)
{
 assert(pdu != NULL);
 e2ap_msg_t ret;
 memset(&ret,0,sizeof( e2ap_msg_t ) );
 
 assert(0!=0 && "Not implemented");

 return ret;
}

static
e2ap_msg_t e2ap_dec_setup_response_success(const E2AP_PDU_t* pdu)
{
  assert(pdu->choice.successfulOutcome->procedureCode == ProcedureCode_id_E2setup);
  assert(pdu->choice.successfulOutcome->criticality == Criticality_reject);
  assert(pdu->choice.successfulOutcome->value.present == SuccessfulOutcome__value_PR_E2setupResponse);

  e2ap_msg_t ret = {.type = E2_SETUP_RESPONSE };

  e2_setup_response_t* sr = &ret.u_msgs.e2_stp_resp;

  const E2setupResponse_t* out = &pdu->choice.successfulOutcome->value.choice.E2setupResponse;

  assert(out->protocolIEs.list.count > 0 && out->protocolIEs.list.count < 4); 

  const E2setupResponseIEs_t* setup_rid = out->protocolIEs.list.array[0];

  assert(setup_rid->id == ProtocolIE_ID_id_GlobalRIC_ID);
  assert(setup_rid->criticality == Criticality_reject);
  assert(setup_rid->value.present == E2setupResponseIEs__value_PR_GlobalRIC_ID);

  PLMNID_TO_MCC_MNC(&setup_rid->value.choice.GlobalRIC_ID.pLMN_Identity, sr->id.plmn.mcc, sr->id.plmn.mnc, sr->id.plmn.mnc_digit_len);
  BIT_STRING_TO_MACRO_ENB_ID(&setup_rid->value.choice.GlobalRIC_ID.ric_ID, sr->id.near_ric_id.double_word);
  // Remind that only the leftmost bits are send, and therefore, the orignial nb_id cannot be recovered


//  memcpy(&sr->id.near_ric_id, &setup_rid->value.choice.GlobalRIC_ID.ric_ID, sizeof(uint8_t)*3); 
  int elm_id = out->protocolIEs.list.count - 1;
  assert(elm_id > -1 && elm_id < 4);

  while(elm_id != 0){
    const ProtocolIE_ID_t proto_id = out->protocolIEs.list.array[elm_id]->id;  
    assert(proto_id == ProtocolIE_ID_id_RANfunctionsAccepted || proto_id == ProtocolIE_ID_id_RANfunctionsRejected || proto_id == ProtocolIE_ID_id_E2nodeComponentConfigUpdateAck);
    if(proto_id == ProtocolIE_ID_id_RANfunctionsAccepted){
      // List of RAN Functions Accepted
      E2setupResponseIEs_t* ran_func = out->protocolIEs.list.array[elm_id];
      assert(ran_func->id == ProtocolIE_ID_id_RANfunctionsAccepted);
      assert(ran_func->criticality == Criticality_reject); 
      assert(ran_func->value.present == E2setupResponseIEs__value_PR_RANfunctionsID_List); 

      assert(ran_func->value.choice.RANfunctionsID_List.list.count < MAX_NUM_RAN_FUNC_ID);
      sr->len_acc = ran_func->value.choice.RANfunctionsID_List.list.count; 

      sr->accepted = calloc(sr->len_acc, sizeof(accepted_ran_function_t));
      const RANfunctionID_ItemIEs_t** arr = (const RANfunctionID_ItemIEs_t**)ran_func->value.choice.RANfunctionsID_List.list.array; 
      for(size_t i =0; i < sr->len_acc; ++i){
        assert(arr[i]->id == ProtocolIE_ID_id_RANfunctionID_Item); 
        assert(arr[i]->criticality == Criticality_ignore);
        assert(arr[i]->value.present == RANfunctionID_ItemIEs__value_PR_RANfunctionID_Item); 
        assert(arr[i]->value.choice.RANfunctionID_Item.ranFunctionID < MAX_RAN_FUNC_ID);
        sr->accepted[i] = arr[i]->value.choice.RANfunctionID_Item.ranFunctionID; 
      }
      elm_id -=1; 
    } else if (proto_id == ProtocolIE_ID_id_RANfunctionsRejected) {
      // List of RAN Functions Rejected
      const E2setupResponseIEs_t* ran_rej = out->protocolIEs.list.array[elm_id];
      assert(ran_rej->id == ProtocolIE_ID_id_RANfunctionsRejected);	
      assert(ran_rej->criticality == Criticality_reject); 
      assert(ran_rej->value.present == E2setupResponseIEs__value_PR_RANfunctionsIDcause_List); 

      sr->len_rej = ran_rej->value.choice.RANfunctionsID_List.list.count; 
      sr->rejected = calloc( sr->len_rej, sizeof(rejected_ran_function_t));
      RANfunctionIDcause_ItemIEs_t** arr = (RANfunctionIDcause_ItemIEs_t**)ran_rej->value.choice.RANfunctionsID_List.list.array; 
      for(size_t i =0; i < sr->len_rej; ++i){
        assert(arr[i]->id == ProtocolIE_ID_id_RANfunctionID_Item); 
        assert(arr[i]->criticality == Criticality_ignore);
        assert(arr[i]->value.present ==  RANfunctionIDcause_ItemIEs__value_PR_RANfunctionIDcause_Item);
        const RANfunctionIDcause_Item_t* src = &arr[i]->value.choice.RANfunctionIDcause_Item;
        rejected_ran_function_t* dst = &sr->rejected[i];
       
        // RAN Function ID. Mandatory
        assert(src->ranFunctionID < MAX_RAN_FUNC_ID);
        dst->id = src->ranFunctionID;
        // Cause. Mandatory
        switch(src->cause.present){
          case Cause_PR_ricRequest:
            {
              dst->cause.present = CAUSE_RICREQUEST;
              assert(src->cause.choice.ricRequest < 11);
              dst->cause.ricRequest = src->cause.choice.ricRequest;
              break;
            }
          case Cause_PR_ricService:
            {
              dst->cause.present = CAUSE_RICSERVICE;
              assert(src->cause.choice.ricService < 3);
              dst->cause.ricService = src->cause.choice.ricService;
              break;
            }
          case Cause_PR_transport:
            {
              dst->cause.present = CAUSE_TRANSPORT;
              assert( src->cause.choice.transport < 2);
               dst->cause.transport = src->cause.choice.transport;
              break;
            }
          case Cause_PR_protocol:
            {
              dst->cause.present =CAUSE_PROTOCOL;
              assert(src->cause.choice.protocol < 7);
              dst->cause.protocol = src->cause.choice.protocol;
              break;
            }
          case Cause_PR_misc:
            {
              dst->cause.present =CAUSE_MISC;
              assert(src->cause.choice.protocol < 4);
              dst->cause.misc = src->cause.choice.misc;
              break;
            }
          case Cause_PR_NOTHING:	/* No components present */
          default:
            assert(0!=0 && "not valid code path");
        }
      }
      elm_id -=1; 
    } else { // ProtocolIE_ID_id_E2nodeComponentConfigUpdateAck

      const E2setupResponseIEs_t* comp_conf = out->protocolIEs.list.array[elm_id];
      assert(comp_conf->id == ProtocolIE_ID_id_E2nodeComponentConfigUpdateAck); 
      assert(comp_conf->criticality == Criticality_reject); 
      assert(comp_conf->value.present == E2setupResponseIEs__value_PR_E2nodeComponentConfigUpdateAck_List); 

      sr->len_ccual = comp_conf->value.choice.E2nodeComponentConfigUpdateAck_List.list.count; 
      sr->comp_conf_update_ack_list = calloc(sr->len_ccual, sizeof(e2_node_component_config_update_t));
      E2nodeComponentConfigUpdateAck_ItemIEs_t** arr = (E2nodeComponentConfigUpdateAck_ItemIEs_t**)comp_conf->value.choice.E2nodeComponentConfigUpdateAck_List.list.array; 
      for (size_t i = 0; i < sr->len_ccual; ++i) {
        assert(0!=0 && "Not implemented!");

        assert(arr[i]->id == ProtocolIE_ID_id_RANfunctionID_Item); 
        assert(arr[i]->criticality == Criticality_ignore);
        assert(arr[i]->value.present == E2nodeComponentConfigUpdateAck_ItemIEs__value_PR_E2nodeComponentConfigUpdateAck_Item); 

      }
      elm_id -=1; 
    }
  }
  return ret;
}

// RIC -> E2
e2ap_msg_t e2ap_dec_setup_response(const E2AP_PDU_t* pdu)
{
  assert(pdu != NULL);
  assert(pdu->present == E2AP_PDU_PR_successfulOutcome || pdu->present == E2AP_PDU_PR_unsuccessfulOutcome);
  if(pdu->present == E2AP_PDU_PR_successfulOutcome)
    return e2ap_dec_setup_response_success(pdu);

  return e2ap_dec_setup_response_unsuccess(pdu);
}

// RIC -> E2
e2ap_msg_t e2ap_dec_setup_failure(const E2AP_PDU_t* pdu)
{
  assert(pdu != NULL);
  e2ap_msg_t ret = {.type = E2_SETUP_FAILURE};
  e2_setup_failure_t* sf = &ret.u_msgs.e2_stp_fail;

  //Message Type. Mandatory
  assert(pdu->present  == E2AP_PDU_PR_unsuccessfulOutcome);
  assert(pdu->choice.unsuccessfulOutcome->procedureCode == ProcedureCode_id_E2setup); 
  assert(pdu->choice.unsuccessfulOutcome->criticality == Criticality_reject);
  assert(pdu->choice.unsuccessfulOutcome->value.present == UnsuccessfulOutcome__value_PR_E2setupFailure); 

  const E2setupFailure_t* out = &pdu->choice.unsuccessfulOutcome->value.choice.E2setupFailure;

  // Cause. Mandatory
  const E2setupFailureIEs_t* cause = out->protocolIEs.list.array[0]; 
  assert(cause->id  == ProtocolIE_ID_id_E2connectionSetupFailed);
  assert(cause->criticality == Criticality_ignore);
  assert(cause->value.present == E2setupFailureIEs__value_PR_Cause);
  sf->cause = copy_cause(cause->value.choice.Cause);

  int elm_id = out->protocolIEs.list.count - 1;
  assert(elm_id > -1 && elm_id < 4);

  while(elm_id != 0){
    const E2setupFailureIEs_t* src = out->protocolIEs.list.array[elm_id];
    assert(src->value.present == E2setupFailureIEs__value_PR_TimeToWait
        || src->value.present == E2setupFailureIEs__value_PR_CriticalityDiagnostics
        || src->value.present == E2setupFailureIEs__value_PR_TNLinformation);

    if(src->value.present == E2setupFailureIEs__value_PR_TimeToWait){
      //Time To Wait. Optional 
      assert(src->id == ProtocolIE_ID_id_TimeToWait); 
      assert(src->criticality == Criticality_ignore);
      sf->time_to_wait_ms = calloc(1,sizeof(uint32_t));
      *sf->time_to_wait_ms = src->value.choice.TimeToWait;
    } else if (src->value.present == E2setupFailureIEs__value_PR_CriticalityDiagnostics){
      //Criticality Diagnostics. Optional
      assert(src->id == ProtocolIE_ID_id_CriticalityDiagnostics); 
      assert(src->criticality == Criticality_ignore);
      assert(0!=0 && "Not implemented");
    } else { //if (src->value.present ==  E2setupFailureIEs__value_PR_TNLinformation) 
      //Transport Layer Information. Optional
      assert(src->id == ProtocolIE_ID_id_TNLinformation); 
      assert(src->criticality == Criticality_ignore);
      sf->tl_info = calloc(1, sizeof(transport_layer_information_t));
      sf->tl_info->address = copy_bs_to_ba(src->value.choice.TNLinformation.tnlAddress);
      if(src->value.choice.TNLinformation.tnlPort != NULL){
        sf->tl_info->port = calloc(1, sizeof(byte_array_t));
        *sf->tl_info->port = copy_bs_to_ba(*src->value.choice.TNLinformation.tnlPort );
      }
    }
    elm_id -= 1;
  }
  return ret;
}

// RIC <-> E2
e2ap_msg_t e2ap_dec_reset_request(const E2AP_PDU_t* pdu)
{
  assert(pdu != NULL);
  e2ap_msg_t ret = {.type = E2AP_RESET_REQUEST};
  e2ap_reset_request_t* rr = &ret.u_msgs.rst_req;

  // Message Type. Mandatory
  assert(pdu->present == E2AP_PDU_PR_initiatingMessage); 
  assert(pdu->choice.initiatingMessage->procedureCode == ProcedureCode_id_Reset); 
  assert(pdu->choice.initiatingMessage->criticality == Criticality_reject);
  assert(pdu->choice.initiatingMessage->value.present == InitiatingMessage__value_PR_ResetRequest); 

  const ResetRequest_t* out = &pdu->choice.initiatingMessage->value.choice.ResetRequest;
  // Cause. Mandatory
  const ResetRequestIEs_t * cause = out->protocolIEs.list.array[0];
  assert(cause->criticality  == Criticality_ignore);
  assert(cause->id == ProtocolIE_ID_id_Cause);	
  assert(cause->value.present == ResetRequestIEs__value_PR_Cause);

  rr->cause = copy_cause(cause->value.choice.Cause);
  return ret;
}

// RIC <-> E2
e2ap_msg_t e2ap_dec_reset_response(const E2AP_PDU_t* pdu)
{
  assert(pdu != NULL);
  e2ap_msg_t ret = {.type = E2AP_RESET_RESPONSE};

  // Message Type. Mandatory
  assert(pdu->present == E2AP_PDU_PR_successfulOutcome);
  assert(pdu->choice.successfulOutcome->procedureCode == ProcedureCode_id_Reset);
  assert(pdu->choice.successfulOutcome->criticality == Criticality_reject);
  assert(pdu->choice.successfulOutcome->value.present == SuccessfulOutcome__value_PR_ResetResponse);

 const ResetResponse_t* out = &pdu->choice.successfulOutcome->value.choice.ResetResponse;
  // Criticality Diagnostics. Optional
  if(out->protocolIEs.list.count > 0){
    const ResetResponseIEs_t* res = out->protocolIEs.list.array[0]; 
    assert(res->id  == ProtocolIE_ID_id_CriticalityDiagnostics);
    assert(res->criticality == Criticality_ignore);
    assert(res->value.present == ResetResponseIEs__value_PR_CriticalityDiagnostics);
    assert(0!=0 && "Not implemented");
  } 
  return ret;
}

// E2 -> RIC
e2ap_msg_t e2ap_dec_service_update(const E2AP_PDU_t* pdu)
{
  assert(pdu != NULL);
  e2ap_msg_t ret = {.type = RIC_SERVICE_UPDATE};
  ric_service_update_t* su = &ret.u_msgs.ric_serv_updt;

  // Message Type
  assert(pdu->present == E2AP_PDU_PR_initiatingMessage);
  assert(pdu->choice.initiatingMessage->procedureCode == ProcedureCode_id_RICserviceUpdate);
  assert(pdu->choice.initiatingMessage->criticality == Criticality_reject);
  assert(pdu->choice.initiatingMessage->value.present == InitiatingMessage__value_PR_RICserviceUpdate);

  const RICserviceUpdate_t *out = &pdu->choice.initiatingMessage->value.choice.RICserviceUpdate;

  // List of RAN Functions Added
  const RICserviceUpdate_IEs_t* ran_add = out->protocolIEs.list.array[0];
  assert(ran_add->id == ProtocolIE_ID_id_RANfunctionsAdded);
  assert(ran_add->criticality == Criticality_reject);
  assert(ran_add->value.present == RICserviceUpdate_IEs__value_PR_RANfunctions_List);
  const int sz_add = ran_add->value.choice.RANfunctions_List.list.count;
  su->added = calloc(sz_add, sizeof(ran_function_t));
  su->len_added = sz_add;
  for(int i = 0; i < sz_add; ++i){
    const RANfunction_ItemIEs_t* r = (const RANfunction_ItemIEs_t*)ran_add->value.choice.RANfunctions_List.list.array[i];
    assert(r->id == ProtocolIE_ID_id_RANfunction_Item);
    assert(r->criticality == Criticality_reject);
    assert(r->value.present == RANfunction_ItemIEs__value_PR_RANfunction_Item);
    su->added[i] = copy_ran_function(&r->value.choice.RANfunction_Item);
  }

  // List of RAN Functions Modified
  const RICserviceUpdate_IEs_t* ran_mod = out->protocolIEs.list.array[1];
  assert(ran_mod->id == ProtocolIE_ID_id_RANfunctionsModified);
  assert(ran_mod->criticality == Criticality_reject);
  assert(ran_mod->value.present == RICserviceUpdate_IEs__value_PR_RANfunctions_List);

  const int sz_mod = ran_mod->value.choice.RANfunctions_List.list.count;
  su->modified = calloc(sz_mod, sizeof(ran_function_t));
  su->len_modified = sz_mod;
  for(int i = 0; i < sz_mod; ++i){
    const RANfunction_ItemIEs_t* r = (const RANfunction_ItemIEs_t*)ran_mod->value.choice.RANfunctions_List.list.array[i];
    assert(r->id == ProtocolIE_ID_id_RANfunction_Item);
    assert(r->criticality == Criticality_reject);
    assert(r->value.present == RANfunction_ItemIEs__value_PR_RANfunction_Item);
    su->modified[i] = copy_ran_function(&r->value.choice.RANfunction_Item);
  }

  // List of RAN Functions Deleted
  const RICserviceUpdate_IEs_t* ran_del = out->protocolIEs.list.array[2];
  assert(ran_del->id == ProtocolIE_ID_id_RANfunctionsDeleted);
  assert(ran_del->criticality == Criticality_reject);
  assert(ran_del->value.present == RICserviceUpdate_IEs__value_PR_RANfunctionsID_List);
  const int sz_del = ran_del->value.choice.RANfunctions_List.list.count;
  su->deleted  = calloc(sz_del, sizeof(ran_function_t));
  su->len_deleted = sz_del;
  for(int i = 0; i < sz_del; ++i){
    const RANfunction_ItemIEs_t* r = (const RANfunction_ItemIEs_t*)ran_del->value.choice.RANfunctions_List.list.array[i];
    assert(r->id == ProtocolIE_ID_id_RANfunction_Item);
    assert(r->criticality == Criticality_reject);
    assert(r->value.present == RANfunction_ItemIEs__value_PR_RANfunction_Item);
    su->deleted[i].id = r->value.choice.RANfunction_Item.ranFunctionID;
    su->deleted[i].rev = r->value.choice.RANfunction_Item.ranFunctionRevision;
  }
  return ret;
}


// RIC -> E2
e2ap_msg_t e2ap_dec_service_update_ack(const E2AP_PDU_t* pdu)
{
  assert(pdu != NULL);
  e2ap_msg_t ret = {.type = RIC_SERVICE_UPDATE_ACKNOWLEDGE};
  ric_service_update_ack_t* su = &ret.u_msgs.ric_serv_updt_ack;
  // Message Type. Mandatory
  assert(pdu->present == E2AP_PDU_PR_successfulOutcome); 
  assert(pdu->choice.successfulOutcome->procedureCode == ProcedureCode_id_RICserviceUpdate);
  assert(pdu->choice.successfulOutcome->criticality == Criticality_reject);
  assert(pdu->choice.successfulOutcome->value.present == SuccessfulOutcome__value_PR_RICserviceUpdateAcknowledge); 

  const RICserviceUpdateAcknowledge_t* out = &pdu->choice.successfulOutcome->value.choice.RICserviceUpdateAcknowledge; 

  int elm = 0;
  while(elm < out->protocolIEs.list.count){
    RICserviceUpdateAcknowledge_IEs_t* up_ack = out->protocolIEs.list.array[elm]; 
    assert(up_ack->id == ProtocolIE_ID_id_RANfunctionsAccepted
        || up_ack->id == ProtocolIE_ID_id_RANfunctionsRejected);

    if(up_ack->id == ProtocolIE_ID_id_RANfunctionsAccepted){
      // List of RAN Functions Accepted 
      RICserviceUpdateAcknowledge_IEs_t* update_ack = up_ack;
      assert(update_ack->criticality == Criticality_ignore);
      assert(update_ack->value.present == RICserviceUpdateAcknowledge_IEs__value_PR_RANfunctionsID_List); 
      const int sz_acc = update_ack->value.choice.RANfunctionsID_List.list.count;
      su->accepted = calloc(sz_acc, sizeof(ran_function_id_t)); 
      su->len_accepted = sz_acc;
      for(int i = 0; i < sz_acc; ++i){
        const RANfunctionID_ItemIEs_t* r = (const RANfunctionID_ItemIEs_t*)update_ack->value.choice.RANfunctionsID_List.list.array[i];
        assert(r->id == ProtocolIE_ID_id_RANfunction_Item);
        assert(r->criticality == Criticality_reject);
        assert(r->value.present == RANfunctionID_ItemIEs__value_PR_RANfunctionID_Item);
        su->accepted[i].id = r->value.choice.RANfunctionID_Item.ranFunctionID;
        su->accepted[i].rev = r->value.choice.RANfunctionID_Item.ranFunctionRevision ;
      }
    } else { // up_ack->id == ProtocolIE_ID_id_RANfunctionsRejected
      // List of RAN Functions Rejected
      RICserviceUpdateAcknowledge_IEs_t* func_reject_ie = up_ack;
      assert(func_reject_ie->id == ProtocolIE_ID_id_RANfunctionsRejected); 
      assert(func_reject_ie->criticality == Criticality_ignore);
      assert(func_reject_ie->value.present == RICserviceUpdateAcknowledge_IEs__value_PR_RANfunctionsID_List); 
      const int sz_rej = func_reject_ie->value.choice.RANfunctionsIDcause_List.list.count;
      su->rejected = calloc(sz_rej, sizeof( rejected_ran_function_t));
      for(int i = 0; i < sz_rej; ++i){
        const RANfunctionIDcause_ItemIEs_t * r = (const RANfunctionIDcause_ItemIEs_t * )func_reject_ie->value.choice.RANfunctionsIDcause_List.list.array[i];
        assert(r->criticality == Criticality_ignore);
        assert(r->id == ProtocolIE_ID_id_RANfunctionIEcause_Item);
        assert(r->value.present == RANfunctionIDcause_ItemIEs__value_PR_RANfunctionIDcause_Item);

        const RANfunctionIDcause_Item_t* src = &r->value.choice.RANfunctionIDcause_Item; 
        rejected_ran_function_t* dst = &su->rejected[i];
        dst->id = src->ranFunctionID;
        dst->cause = copy_cause(src->cause);
      }
    }
    elm += 1;
  }
  return ret;
}

// RIC -> E2
 e2ap_msg_t e2ap_dec_service_update_failure(const E2AP_PDU_t* pdu)
{
  assert(pdu != NULL);
  e2ap_msg_t ret = {.type = RIC_SERVICE_UPDATE_FAILURE};
 ric_service_update_failure_t* uf = &ret.u_msgs.ric_serv_updt_fail;
  // Message Type. Mandatory
  assert(pdu->present == E2AP_PDU_PR_unsuccessfulOutcome); 
  assert(pdu->choice.unsuccessfulOutcome->procedureCode == ProcedureCode_id_RICserviceUpdate);
  assert(pdu->choice.unsuccessfulOutcome->criticality == Criticality_reject);
  assert(pdu->choice.unsuccessfulOutcome->value.present == UnsuccessfulOutcome__value_PR_RICserviceUpdateFailure); 

  const RICserviceUpdateFailure_t* out = &pdu->choice.unsuccessfulOutcome->value.choice.RICserviceUpdateFailure; 

  // List of RAN Functions Rejected. Mandatory
  RICserviceUpdateFailure_IEs_t* update_failure = out->protocolIEs.list.array[0];
  assert(update_failure->id == ProtocolIE_ID_id_RANfunctionsRejected);
  assert(update_failure->criticality == Criticality_reject);
  assert(update_failure->value.present == RICserviceUpdateFailure_IEs__value_PR_RANfunctionsIDcause_List); 

 const int sz_rej = update_failure->value.choice.RANfunctionsIDcause_List.list.count;
  uf->rejected = calloc(sz_rej, sizeof( rejected_ran_function_t) );
  uf->len_rej = sz_rej;
  for(int i = 0; i < sz_rej; ++i){
    const RANfunctionIDcause_ItemIEs_t * r = (const RANfunctionIDcause_ItemIEs_t * )update_failure->value.choice.RANfunctionsIDcause_List.list.array[i]; 

    assert(r->criticality == Criticality_ignore);
    assert(r->id == ProtocolIE_ID_id_RANfunctionIEcause_Item);
    assert(r->value.present == RANfunctionIDcause_ItemIEs__value_PR_RANfunctionIDcause_Item);

    uf->rejected[i].id = r->value.choice.RANfunctionIDcause_Item.ranFunctionID;
    uf->rejected[i].cause = copy_cause(r->value.choice.RANfunctionIDcause_Item.cause);
  }

  int elm = out->protocolIEs.list.count - 1;
  while(elm > 0){
    const RICserviceUpdateFailure_IEs_t* su = out->protocolIEs.list.array[elm]; 
    assert(su->value.present ==  RICserviceUpdateFailure_IEs__value_PR_TimeToWait
            || su->value.present == RICserviceUpdateFailure_IEs__value_PR_CriticalityDiagnostics);
    if(su->value.present == RICserviceUpdateFailure_IEs__value_PR_TimeToWait){
        assert(su->id == ProtocolIE_ID_id_RANfunctionsRejected);
        assert(su->criticality == Criticality_reject);
        uf->time_to_wait =  calloc(1, sizeof(e2ap_time_to_wait_e));
        *uf->time_to_wait = su->value.choice.TimeToWait;
    } else { // su->value.present == RICserviceUpdateFailure_IEs__value_PR_CriticalityDiagnostics
    assert(su->id == ProtocolIE_ID_id_RANfunctionsRejected);
    assert(su->criticality == Criticality_reject);
    assert(0!=0 && "Not implememnted");
    }
    elm -= 1;
  }
  return ret;
}

// RIC -> E2
e2ap_msg_t e2ap_dec_service_query(const E2AP_PDU_t* pdu)
{
  assert(pdu != NULL);
  e2ap_msg_t ret = {.type = RIC_SERVICE_QUERY};
  ric_service_query_t* sq = &ret.u_msgs.ric_serv_query;
  // Message Type. Mandatory
  assert(pdu->present == E2AP_PDU_PR_initiatingMessage); 
  assert(pdu->choice.initiatingMessage->procedureCode == ProcedureCode_id_RICserviceQuery); 
  assert(pdu->choice.initiatingMessage->criticality == Criticality_reject);
  assert(pdu->choice.initiatingMessage->value.present == InitiatingMessage__value_PR_RICserviceQuery); 

  const RICserviceQuery_t* out = &pdu->choice.initiatingMessage->value.choice.RICserviceQuery; 

  // List of RAN Functions Accepted. Mandatory
  const RICserviceQuery_IEs_t* serv_query_ie = out->protocolIEs.list.array[0];
  assert(serv_query_ie->id == ProtocolIE_ID_id_RANfunctionsAccepted); 
  assert(serv_query_ie->criticality == Criticality_ignore);
  assert(serv_query_ie->value.present == RICserviceQuery_IEs__value_PR_RANfunctionsID_List); 

  const int sz = serv_query_ie->value.choice.RANfunctionsID_List.list.count;
  sq->accepted = calloc(sz, sizeof(e2ap_ran_function_id_rev_t)); 
  sq->len_accepted = sz;
  for(int i = 0; i < sz; ++i){
    const RANfunction_ItemIEs_t* r = (const RANfunction_ItemIEs_t*)serv_query_ie->value.choice.RANfunctionsID_List.list.array[i];
    assert(r->id == ProtocolIE_ID_id_RANfunction_Item);
    assert(r->criticality == Criticality_reject);
    assert(r->value.present == RANfunction_ItemIEs__value_PR_RANfunction_Item);
    sq->accepted[i].id = r->value.choice.RANfunction_Item.ranFunctionID;
    sq->accepted[i].rev = r->value.choice.RANfunction_Item.ranFunctionRevision;
  }
  return ret;
}

// E2 -> RIC
e2ap_msg_t e2ap_dec_node_configuration_update(const E2AP_PDU_t* pdu)
{
  assert(pdu != NULL);
  e2ap_msg_t ret = {.type = E2_NODE_CONFIGURATION_UPDATE};
  e2_node_configuration_update_t* cu = &ret.u_msgs.e2_node_conf_updt;

  // Message Type. Mandatory
  assert(pdu->present == E2AP_PDU_PR_initiatingMessage); 
  assert(pdu->choice.initiatingMessage->procedureCode == ProcedureCode_id_E2nodeConfigurationUpdate); 
  assert(pdu->choice.initiatingMessage->criticality == Criticality_reject);
  assert(pdu->choice.initiatingMessage->value.present == InitiatingMessage__value_PR_E2nodeConfigurationUpdate); 

  const E2nodeConfigurationUpdate_t* out = &pdu->choice.initiatingMessage->value.choice.E2nodeConfigurationUpdate; 

  // E2 Node Component Configuration Update List
  const E2nodeConfigurationUpdate_IEs_t* conf_update_ie = out->protocolIEs.list.array[0]; 
  assert(conf_update_ie->id == ProtocolIE_ID_id_E2nodeComponentConfigUpdate);	
  assert(conf_update_ie->criticality == Criticality_ignore);
  assert(conf_update_ie->value.present == E2nodeConfigurationUpdate_IEs__value_PR_E2nodeComponentConfigUpdate_List);
 

  const int sz = conf_update_ie->value.choice.E2nodeComponentConfigUpdate_List.list.count;
  cu->comp_conf_update_list = calloc(sz, sizeof(e2_node_component_config_update_t)); 
  cu->len_ccul = sz;

  for(int i = 0; i < sz; ++i){
    const E2nodeComponentConfigUpdate_ItemIEs_t* n = (const E2nodeComponentConfigUpdate_ItemIEs_t*) conf_update_ie->value.choice.E2nodeComponentConfigUpdate_List.list.array[i];
     const E2nodeComponentConfigUpdate_Item_t* src = &n->value.choice.E2nodeComponentConfigUpdate_Item;
    cu->comp_conf_update_list[i] = copy_e2_node_component_conf_update(src);
  }
  return ret;
}


// RIC -> E2
e2ap_msg_t e2ap_dec_node_configuration_update_ack(const E2AP_PDU_t* pdu)
{
  assert(pdu != NULL);
 assert(0!=0 && "Not implemented");
  e2ap_msg_t ret;
  return ret;
}


// RIC -> E2
e2ap_msg_t e2ap_dec_node_configuration_update_failure(const E2AP_PDU_t* pdu)\
{
  assert(pdu != NULL);
 assert(0!=0 && "Not implemented");
  e2ap_msg_t ret;
  return ret;
}


// RIC -> E2
 e2ap_msg_t e2ap_dec_connection_update(const E2AP_PDU_t* pdu)
{
  assert(pdu != NULL);
 assert(0!=0 && "Not implemented");
  e2ap_msg_t ret;
  return ret;
}


// E2 -> RIC
 e2ap_msg_t e2ap_dec_connection_update_ack(const E2AP_PDU_t* pdu)
{
  assert(pdu != NULL);
 assert(0!=0 && "Not implemented");
  e2ap_msg_t ret;
  return ret;
}


// E2 -> RIC
e2ap_msg_t e2ap_dec_connection_update_failure(const E2AP_PDU_t* pdu)
{
  assert(pdu != NULL);
 assert(0!=0 && "Not implemented");
  e2ap_msg_t ret;
  return ret;
}


// xApp -> iApp
e2ap_msg_t e2ap_dec_e42_setup_request(const struct E2AP_PDU* pdu)
{
  assert(pdu != NULL);
  assert(pdu->present == E2AP_PDU_PR_initiatingMessage);

  assert(pdu->choice.initiatingMessage->procedureCode == ProcedureCode_id_E42setup);
  assert(pdu->choice.initiatingMessage->criticality == Criticality_reject);
  assert(pdu->choice.initiatingMessage->value.present == InitiatingMessage__value_PR_E42setupRequest);

  e2ap_msg_t ret = {.type = E42_SETUP_REQUEST};
  e42_setup_request_t* sr = &ret.u_msgs.e42_stp_req;

  const E42setupRequest_t *out = &pdu->choice.initiatingMessage->value.choice.E42setupRequest;
  assert(out->protocolIEs.list.count > 0 && "Need to have some functionality");

  E42setupRequestIEs_t* ran_list = out->protocolIEs.list.array[0];

  assert(ran_list->id == ProtocolIE_ID_id_RANfunctionsAdded);
  assert(ran_list->criticality == Criticality_reject);
  assert(ran_list->value.present == E42setupRequestIEs__value_PR_RANfunctions_List);

  size_t const sz = ran_list->value.choice.RANfunctions_List.list.count;
  sr->len_rf = sz;

  if(sr->len_rf > 0){
    sr->ran_func_item = calloc(sr->len_rf, sizeof(ran_function_t));
  }
  RANfunction_ItemIEs_t** arr = (RANfunction_ItemIEs_t**)ran_list->value.choice.RANfunctions_List.list.array;
  for(size_t i =0; i < sr->len_rf; ++i){
    ran_function_t* dst = &sr->ran_func_item[i];
    const RANfunction_Item_t* src = &arr[i]->value.choice.RANfunction_Item;

    assert(src->ranFunctionID <= MAX_RAN_FUNC_ID);
    dst->id = src->ranFunctionID; 

    assert(src->ranFunctionRevision > -1 && src->ranFunctionRevision <= MAX_RAN_FUNC_REV);
    dst->rev = src->ranFunctionRevision;

    dst->def = copy_ostring_to_ba(src->ranFunctionDefinition); 

    if(src->ranFunctionOID != NULL){
      dst->oid = malloc(sizeof(byte_array_t));
      *dst->oid = copy_ostring_to_ba(*src->ranFunctionOID);
    }
  }

  return ret;
}

// iApp -> xApp
e2ap_msg_t e2ap_dec_e42_setup_response(const struct E2AP_PDU* pdu)
{
  assert(pdu != NULL);
  assert(pdu->present == E2AP_PDU_PR_successfulOutcome);

  assert(pdu->choice.successfulOutcome->procedureCode == ProcedureCode_id_E42setup);
  assert(pdu->choice.successfulOutcome->criticality == Criticality_reject);
  assert(pdu->choice.successfulOutcome->value.present == SuccessfulOutcome__value_PR_E42setupResponse);

  e2ap_msg_t ret = {.type = E42_SETUP_RESPONSE};
  e42_setup_response_t* sr = &ret.u_msgs.e42_stp_resp;

  E42setupResponse_t const* out = &pdu->choice.successfulOutcome->value.choice.E42setupResponse;

  size_t elm = out->protocolIEs.list.count; 
  assert(elm > 0 && "At least the xApp-id should be here"); 

  E42setupResponseIEs_t* setup_rid = out->protocolIEs.list.array[0];

  assert(setup_rid->id == ProtocolIE_ID_id_XAPP_ID);
  assert(setup_rid->criticality == Criticality_reject);
  assert(setup_rid->value.present == E42setupResponseIEs__value_PR_XAPP_ID);

  sr->xapp_id = setup_rid->value.choice.XAPP_ID;
  elm -= 1;

  sr->len_e2_nodes_conn = elm;
  if(elm > 0){
    sr->nodes = calloc(elm, sizeof( e2_node_connected_t) );
    assert(sr->nodes != NULL && "Memory exhausted");
  }

  for(size_t i = 0; i < elm; ++i){
    E42setupResponseIEs_t const* conn_list = out->protocolIEs.list.array[i+1];

    assert(conn_list->id == ProtocolIE_ID_id_E2nodesConnected);
    assert(conn_list->criticality == Criticality_reject);
    assert(conn_list->value.present == E42setupResponseIEs__value_PR_E2nodeConnected_List);
    assert(conn_list->value.choice.E2nodeConnected_List.protocolIEs.list.count == 2 && "Need the Global E2 Node ID and the RAN functions" );

    // Global E2 Node ID. Mandatory
    E2nodeConnected_ItemIEs_t const* src = conn_list->value.choice.E2nodeConnected_List.protocolIEs.list.array[0];

    e2_node_connected_t* dst = &sr->nodes[i];
    // Only gNodeB supported
    assert(src->id == ProtocolIE_ID_id_GlobalE2node_ID);
    assert(src->criticality == Criticality_reject);
    assert(src->value.present == E2nodeConnected_ItemIEs__value_PR_GlobalE2node_ID);
    assert(src->value.choice.GlobalE2node_ID.present == GlobalE2node_ID_PR_gNB);
    dst->id.type = ngran_gNB;  

    GlobalE2node_gNB_ID_t *e2gnb = src->value.choice.GlobalE2node_ID.choice.gNB;
    assert(e2gnb->global_gNB_ID.gnb_id.present == GNB_ID_Choice_PR_gnb_ID);
    PLMNID_TO_MCC_MNC(&e2gnb->global_gNB_ID.plmn_id, dst->id.plmn.mcc, dst->id.plmn.mnc, dst->id.plmn.mnc_digit_len);
    BIT_STRING_TO_MACRO_GNB_ID(&e2gnb->global_gNB_ID.gnb_id.choice.gnb_ID, dst->id.nb_id);


    E2nodeConnected_ItemIEs_t const* src_ran = conn_list->value.choice.E2nodeConnected_List.protocolIEs.list.array[1];

    assert(src_ran->id ==   ProtocolIE_ID_id_RANfunctionsAdded);
    assert(src_ran->criticality == Criticality_reject);
    assert(src_ran->value.present == E2nodeConnected_ItemIEs__value_PR_RANfunctions_List);

    dst->len_rf = src_ran->value.choice.RANfunctions_List.list.count;
    if(dst->len_rf > 0){
      dst->ack_rf = calloc(dst->len_rf, sizeof(ran_function_t));
      assert(dst->ack_rf != NULL && "memory exhausted");
    }


    RANfunction_ItemIEs_t** arr = (RANfunction_ItemIEs_t**)src_ran->value.choice.RANfunctions_List.list.array;
    for(size_t i =0; i < dst->len_rf; ++i){
      ran_function_t* dst_ie = &dst->ack_rf[i];
      const RANfunction_Item_t* src = &arr[i]->value.choice.RANfunction_Item;

      assert(src->ranFunctionID <= MAX_RAN_FUNC_ID);
      dst_ie->id = src->ranFunctionID; 

      assert(src->ranFunctionRevision > -1 && src->ranFunctionRevision <= MAX_RAN_FUNC_REV);
      dst_ie->rev = src->ranFunctionRevision;

      dst_ie->def = copy_ostring_to_ba(src->ranFunctionDefinition); 

      if(src->ranFunctionOID != NULL){
        dst_ie->oid = malloc(sizeof(byte_array_t));
        *dst_ie->oid = copy_ostring_to_ba(*src->ranFunctionOID);
      }
    }
  }
  return ret;
}


e2ap_msg_t e2ap_dec_e42_subscription_delete_request(const struct E2AP_PDU* pdu)
{
  assert(pdu != NULL);
  e2ap_msg_t ret = {.type = E42_RIC_SUBSCRIPTION_DELETE_REQUEST};
  e42_ric_subscription_delete_request_t* e42_sdr = &ret.u_msgs.e42_ric_sub_del_req;

  // Message Type. Mandatory
  assert(pdu->present == E2AP_PDU_PR_initiatingMessage);
  assert(pdu->choice.initiatingMessage->procedureCode == ProcedureCode_id_E42RICsubscriptionDelete); 
  assert(pdu->choice.initiatingMessage->criticality == Criticality_reject);
  assert(pdu->choice.initiatingMessage->value.present == InitiatingMessage__value_PR_E42RICsubscriptionDeleteRequest); 

  const E42RICsubscriptionDeleteRequest_t* out = &pdu->choice.initiatingMessage->value.choice.E42RICsubscriptionDeleteRequest;

  // XAPP ID. Mandatory
  E42RICsubscriptionDeleteRequest_IEs_t* xapp_id = out->protocolIEs.list.array[0];
  assert(xapp_id->id == ProtocolIE_ID_id_XAPP_ID);
  assert(xapp_id->criticality == Criticality_reject);
  assert(xapp_id->value.present == E42RICsubscriptionDeleteRequest_IEs__value_PR_XAPP_ID);
  e42_sdr->xapp_id = xapp_id->value.choice.XAPP_ID;


  ric_subscription_delete_request_t* dr = &e42_sdr->sdr;
  // RIC Request ID. Mandatory
  const E42RICsubscriptionDeleteRequest_IEs_t* sub_req = out->protocolIEs.list.array[1];
  assert(sub_req->id == ProtocolIE_ID_id_RICrequestID);	
  assert(sub_req->criticality == Criticality_reject);
  assert(sub_req->value.present == E42RICsubscriptionDeleteRequest_IEs__value_PR_RICrequestID); 
  dr->ric_id.ric_inst_id = sub_req->value.choice.RICrequestID.ricInstanceID;
  dr->ric_id.ric_req_id = sub_req->value.choice.RICrequestID.ricRequestorID;

  // RIC Function ID. Mandatory
  const E42RICsubscriptionDeleteRequest_IEs_t* ran_id = out->protocolIEs.list.array[2];
  assert(ran_id->id == ProtocolIE_ID_id_RANfunctionID);
  assert(ran_id->criticality == Criticality_reject);
  assert(ran_id->value.present == E42RICsubscriptionDeleteRequest_IEs__value_PR_RANfunctionID);

  assert(ran_id->value.choice.RANfunctionID < MAX_RAN_FUNC_ID);
  dr->ric_id.ran_func_id = ran_id->value.choice.RANfunctionID;
  return ret;
}


///////////////////////////////////////
///////////////////////////////////////
//////////////////////////////////////

static
e2_msg_type_t e2ap_get_msg_type(const E2AP_PDU_t* pdu)
{
  const int index = pdu->present;
  const int procedureCode = e2ap_asn1c_get_procedureCode(pdu);
  assert(index != E2AP_PDU_PR_NOTHING);
  switch (procedureCode) {
    case ProcedureCode_id_E2setup:
      switch (index) {
        case E2AP_PDU_PR_initiatingMessage:   return E2_SETUP_REQUEST;
        case E2AP_PDU_PR_successfulOutcome:   return E2_SETUP_RESPONSE;
        case E2AP_PDU_PR_unsuccessfulOutcome: return E2_SETUP_FAILURE;
      }
      break;

    case ProcedureCode_id_ErrorIndication:
                                              return E2AP_ERROR_INDICATION;
    case ProcedureCode_id_Reset:
      switch (index) {
        case E2AP_PDU_PR_initiatingMessage:   return E2AP_RESET_REQUEST;
        case E2AP_PDU_PR_successfulOutcome:   return E2AP_RESET_RESPONSE;
        case E2AP_PDU_PR_unsuccessfulOutcome: return E2AP_RESET_RESPONSE;
      }
      break;

    case ProcedureCode_id_RICcontrol:
      switch (index) {
        case E2AP_PDU_PR_initiatingMessage:   return RIC_CONTROL_REQUEST;
        case E2AP_PDU_PR_successfulOutcome:   return RIC_CONTROL_ACKNOWLEDGE;
        case E2AP_PDU_PR_unsuccessfulOutcome: return RIC_CONTROL_FAILURE;
      }
      break;

    case ProcedureCode_id_RICindication:
                                              return RIC_INDICATION;
      break;

    case ProcedureCode_id_RICserviceQuery:
                                              return RIC_SERVICE_QUERY;
      break;

    case ProcedureCode_id_RICserviceUpdate:
      switch (index) {
        case E2AP_PDU_PR_initiatingMessage:   return RIC_SERVICE_UPDATE;
        case E2AP_PDU_PR_successfulOutcome:   return RIC_SERVICE_UPDATE_ACKNOWLEDGE;
        case E2AP_PDU_PR_unsuccessfulOutcome: return RIC_SERVICE_UPDATE_FAILURE;
      }
      break;

    case ProcedureCode_id_RICsubscription:
      switch (index) {
        case E2AP_PDU_PR_initiatingMessage:   return RIC_SUBSCRIPTION_REQUEST;
        case E2AP_PDU_PR_successfulOutcome:   return RIC_SUBSCRIPTION_RESPONSE;
        case E2AP_PDU_PR_unsuccessfulOutcome: return RIC_SUBSCRIPTION_FAILURE;
      }
      break;

    case ProcedureCode_id_RICsubscriptionDelete:
      switch (index) {
        case E2AP_PDU_PR_initiatingMessage:   return RIC_SUBSCRIPTION_DELETE_REQUEST;
        case E2AP_PDU_PR_successfulOutcome:   return RIC_SUBSCRIPTION_DELETE_RESPONSE;
        case E2AP_PDU_PR_unsuccessfulOutcome: return RIC_SUBSCRIPTION_DELETE_FAILURE;
      }
      break;

    case ProcedureCode_id_E2nodeConfigurationUpdate:
      switch (index) {
        case E2AP_PDU_PR_initiatingMessage:   return E2_NODE_CONFIGURATION_UPDATE;
        case E2AP_PDU_PR_successfulOutcome:   return E2_NODE_CONFIGURATION_UPDATE_ACKNOWLEDGE;
        case E2AP_PDU_PR_unsuccessfulOutcome: return E2_NODE_CONFIGURATION_UPDATE_FAILURE;
      }
      break;

    case ProcedureCode_id_E2connectionUpdate:
      switch (index) {
        case E2AP_PDU_PR_initiatingMessage:   return E2_CONNECTION_UPDATE;
        case E2AP_PDU_PR_successfulOutcome:   return E2_CONNECTION_UPDATE_ACKNOWLEDGE;
        case E2AP_PDU_PR_unsuccessfulOutcome: return E2_CONNECTION_UPDATE_FAILURE;
      }
      break;

    case ProcedureCode_id_E42setup:
      switch (index) {
        case E2AP_PDU_PR_initiatingMessage:   return E42_SETUP_REQUEST;
        case E2AP_PDU_PR_successfulOutcome:   return E42_SETUP_RESPONSE;
        case E2AP_PDU_PR_unsuccessfulOutcome: return E2_SETUP_FAILURE;
      }
      break;

    case ProcedureCode_id_E42RICsubscription:
      switch (index) {
        case E2AP_PDU_PR_initiatingMessage:   return E42_RIC_SUBSCRIPTION_REQUEST;
        case E2AP_PDU_PR_successfulOutcome:   return RIC_SUBSCRIPTION_RESPONSE;
        case E2AP_PDU_PR_unsuccessfulOutcome: return RIC_SUBSCRIPTION_FAILURE;
      }
      break;

    case ProcedureCode_id_E42RICsubscriptionDelete:
      switch (index) {
        case E2AP_PDU_PR_initiatingMessage:   return E42_RIC_SUBSCRIPTION_DELETE_REQUEST;
        case E2AP_PDU_PR_successfulOutcome:   return RIC_SUBSCRIPTION_DELETE_RESPONSE;
        case E2AP_PDU_PR_unsuccessfulOutcome: return RIC_SUBSCRIPTION_DELETE_FAILURE;
      }
      break;

    case ProcedureCode_id_E42RICcontrol:
      switch (index) {
        case E2AP_PDU_PR_initiatingMessage:   return E42_RIC_CONTROL_REQUEST;
        case E2AP_PDU_PR_successfulOutcome:   return RIC_CONTROL_ACKNOWLEDGE;
        case E2AP_PDU_PR_unsuccessfulOutcome: return RIC_CONTROL_FAILURE;
      }
      break;



    default:
      assert(0 != 0  && "unhandled message");
  }
  assert(0!=0 && "Invalid Path");
  return NONE_E2_MSG_TYPE;
}

static
E2AP_PDU_t* e2ap_create_pdu(const uint8_t* buffer, int buffer_len)
{
  assert(buffer != NULL);
  assert(buffer_len > -1);

  E2AP_PDU_t* pdu = calloc(1, sizeof(E2AP_PDU_t));
  const enum asn_transfer_syntax syntax = ATS_ALIGNED_BASIC_PER;
  const asn_dec_rval_t rval = asn_decode(NULL, syntax, &asn_DEF_E2AP_PDU, (void**)&pdu, buffer, buffer_len);
  //printf("rval.code = %d\n", rval.code);
  //fprintf(stdout, "length of data %ld\n", rval.consumed);
  assert(rval.code == RC_OK && "Are you sending data in ATS_ALIGEND_BASIC_PER syntax?");
  return pdu;
}

e2ap_msg_t e2ap_msg_dec_asn(e2ap_asn_t* asn, byte_array_t ba)
{
  assert(ba.buf != NULL && ba.len > 0);
  E2AP_PDU_t* pdu = e2ap_create_pdu(ba.buf, ba.len);
  assert(pdu != NULL);
  const e2_msg_type_t msg_type = e2ap_get_msg_type(pdu);  
  //printf("Decoding message type = %d \n", msg_type);
  assert(asn->dec_msg[msg_type] != NULL);
  e2ap_msg_t msg = asn->dec_msg[msg_type](pdu);
//  xer_fprint(stdout, &asn_DEF_E2AP_PDU, pdu);
//  fflush(stdout);
  ASN_STRUCT_FREE(asn_DEF_E2AP_PDU,pdu);
  return msg; 
}



