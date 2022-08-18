#include "../ie/asn/E2SM-KPM-EventTriggerDefinition-Format1.h"
#include "../ie/asn/E2SM-KPM-EventTriggerDefinition.h"
#include "../ie/asn/E2SM-KPM-IndicationHeader.h"
#include "../ie/asn/E2SM-KPM-IndicationHeader-Format1.h"
#include "../ie/asn/E2SM-KPM-IndicationMessage.h"
#include "../ie/asn/E2SM-KPM-IndicationMessage-Format1.h"
#include "../ie/asn/MeasurementDataItem.h"
#include "../ie/asn/MeasurementData.h"
#include "../ie/asn/MeasurementRecord.h"
#include "../ie/asn/MeasurementRecordItem.h"


#include "kpm_dec_asn.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>


kpm_event_trigger_t kpm_dec_event_trigger_asn(size_t len, uint8_t const ev_tr[len])
{
  assert(len>0);
  assert(ev_tr != NULL);

  E2SM_KPM_EventTriggerDefinition_t *pdu = calloc(1, sizeof(E2SM_KPM_EventTriggerDefinition_t));
  assert( pdu !=NULL && "Memory exhausted" );

  const enum asn_transfer_syntax syntax = ATS_ALIGNED_BASIC_PER;
  const asn_dec_rval_t rval = asn_decode(NULL, syntax, &asn_DEF_E2SM_KPM_EventTriggerDefinition, (void**)&pdu, ev_tr, len);
  assert(rval.code == RC_OK && "Are you sending data in ATS_ALIGNED_BASIC_PER syntax?");

  kpm_event_trigger_t ret = {0};

  assert ( pdu->eventDefinition_formats.present == E2SM_KPM_EventTriggerDefinition__eventDefinition_formats_PR_eventDefinition_Format1 && "Invalid Choice"  );

  ret.ms = pdu->eventDefinition_formats.choice.eventDefinition_Format1->reportingPeriod;

  ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2SM_KPM_EventTriggerDefinition,pdu);
  free(pdu);

  return ret;
}


kpm_action_def_t kpm_dec_action_def_asn(size_t len, uint8_t const action_def[len]) 
{
  //assert (0!=0 &&  "Not implemented");
  printf ("[IE Action definition]: decoding not implemented\n");
  kpm_action_def_t avoid_warning = {0};
  return avoid_warning;
}

kpm_ind_hdr_t kpm_dec_ind_hdr_asn(size_t len, uint8_t const ind_hdr[len])
{
  assert(len>0);
  assert(ind_hdr != NULL);

  kpm_ind_hdr_t ret = {0};

  E2SM_KPM_IndicationHeader_t *pdu = calloc(1, sizeof(E2SM_KPM_IndicationHeader_t));
  assert( pdu !=NULL && "Memory exhausted" );

  const enum asn_transfer_syntax syntax = ATS_ALIGNED_BASIC_PER;
  const asn_dec_rval_t rval = asn_decode(NULL, syntax, &asn_DEF_E2SM_KPM_IndicationHeader, (void**)&pdu, ind_hdr, len);
  assert(rval.code == RC_OK && "Are you sending data in ATS_ALIGNED_BASIC_PER syntax?");
  E2SM_KPM_IndicationHeader_Format1_t *hdr = pdu->indicationHeader_formats.choice.indicationHeader_Format1;

  ret.collectStartTime.buf = calloc (1, hdr->colletStartTime.size);
  assert( ret.collectStartTime.buf !=NULL && "Memory exhausted" );
  memcpy(ret.collectStartTime.buf, hdr->colletStartTime.buf, hdr->colletStartTime.size);
  ret.collectStartTime.len = hdr->colletStartTime.size;

  if (hdr->fileFormatversion){
    ret.fileFormatversion = calloc(1, sizeof(ret.fileFormatversion));
    assert(ret.fileFormatversion !=NULL && "Memory exhausted" );
    BYTE_ARRAY_HEAP_CP_FROM_OCTET_STRING(ret.fileFormatversion, hdr->fileFormatversion);
  }

  if (hdr->senderName){
    ret.senderName = calloc(1, sizeof(ret.senderName));
    assert(ret.senderName !=NULL && "Memory exhausted" );
    BYTE_ARRAY_HEAP_CP_FROM_OCTET_STRING(ret.senderName, hdr->senderName);
  }

  if (hdr->senderType){
    ret.senderType = calloc(1, sizeof(ret.senderType));
    assert(ret.senderType !=NULL && "Memory exhausted" );
    BYTE_ARRAY_HEAP_CP_FROM_OCTET_STRING(ret.senderType, hdr->senderType);
  }

	if (hdr->vendorName){
    ret.vendorName = calloc(1, sizeof(ret.vendorName));
    assert(ret.vendorName !=NULL && "Memory exhausted" );
    BYTE_ARRAY_HEAP_CP_FROM_OCTET_STRING(ret.vendorName, hdr->vendorName);
  }

	ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2SM_KPM_IndicationHeader, pdu);
  free(pdu);

  return ret;
}

kpm_ind_msg_t kpm_dec_ind_msg_asn(size_t len, uint8_t const ind_msg[len])
{
  assert(ind_msg != NULL);
  assert(len>0);

  kpm_ind_msg_t ret = {0};

  E2SM_KPM_IndicationMessage_t *pdu = calloc(1, sizeof(E2SM_KPM_IndicationMessage_t));
  assert( pdu !=NULL && "Memory exhausted" );

  const enum asn_transfer_syntax syntax = ATS_ALIGNED_BASIC_PER;
  const asn_dec_rval_t rval = asn_decode(NULL, syntax, &asn_DEF_E2SM_KPM_IndicationMessage, (void**)&pdu, ind_msg, len);
  assert(rval.code == RC_OK && "Are you sending data in ATS_ALIGNED_BASIC_PER syntax?");
  E2SM_KPM_IndicationMessage_Format1_t *msg = pdu->indicationMessage_formats.choice.indicationMessage_Format1;

  // 1. MeasData
  adapter_MeasDataItem_t *mData = calloc(msg->measData.list.count, sizeof(adapter_MeasDataItem_t));
  ret.MeasData = mData;
  ret.MeasData_len = msg->measData.list.count;
  for (size_t i = 0; i< (size_t)msg->measData.list.count; i++)
  {
    MeasurementDataItem_t *item = msg->measData.list.array[i]; 
    if (item->incompleteFlag)
      mData[i].incompleteFlag = 0;
    adapter_MeasRecord_t * rec = calloc(item->measRecord.list.count, sizeof(adapter_MeasRecord_t));
    for (size_t j = 0; j < (size_t) item->measRecord.list.count; j++){
      rec[j].type = item->measRecord.list.array[j]->present;
      switch (rec[j].type){
      case MeasRecord_int:
        rec[j].int_val = item->measRecord.list.array[j]->choice.integer;
        break;
      case MeasRecord_real: 
        rec[j].real_val = item->measRecord.list.array[j]->choice.real;
        break;
      default:
        break;
      } 
    }
    mData[i].measRecord = rec;
  }
  // 2. measInfoList (OPTIONAL)
  assert(msg->measInfoList == NULL && "Decoding of measInfoList not implemented yet");

  // 3. granulPeriod (OPTIONAL)
  assert(msg->granulPeriod == NULL && "Decoding of granulPeriod not implemented yet");

  ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2SM_KPM_IndicationMessage, pdu);
  free(pdu); 

  return ret;
}


kpm_func_def_t kpm_dec_func_def_asn(size_t len, uint8_t const ind_msg[len])
{

}