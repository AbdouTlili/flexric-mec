  /*
  * XXX-optimization: understand where do they come these values below for memory allocation of the byte arrays.
  * A tuning of those values would probably benefit this implementation
  * XXX-implementation, cfr: https://gitlab.eurecom.fr/mosaic5g/flexric/-/blob/rrc-sm/src/sm/rrc_sm/enc/rrc_enc_asn.c
  */
#include "../ie/asn/E2SM-KPM-EventTriggerDefinition-Format1.h"
#include "../ie/asn/E2SM-KPM-EventTriggerDefinition.h"
#include "../ie/asn/E2SM-KPM-ActionDefinition-Format1.h"
#include "../ie/asn/E2SM-KPM-ActionDefinition-Format2.h"
#include "../ie/asn/E2SM-KPM-ActionDefinition-Format3.h"
#include "../ie/asn/E2SM-KPM-ActionDefinition-Format4.h"
#include "../ie/asn/E2SM-KPM-ActionDefinition-Format5.h"
#include "../ie/asn/E2SM-KPM-ActionDefinition.h"
#include "../ie/asn/MeasurementInfoList.h"
#include "../ie/asn/MeasurementInfoItem.h"
#include "../ie/asn/LabelInfoItem.h"
#include "../ie/asn/asn_constant.h"
#include "../ie/asn/CGI.h"
#include "../ie/asn/EUTRA-CGI.h"
#include "../ie/asn/NR-CGI.h"
#include "../ie/asn/E2SM-KPM-IndicationHeader.h"
#include "../ie/asn/E2SM-KPM-IndicationHeader-Format1.h"
#include "../ie/asn/E2SM-KPM-IndicationMessage.h"
#include "../ie/asn/E2SM-KPM-IndicationMessage-Format1.h"
#include "../ie/asn/MeasurementDataItem.h"
#include "../ie/asn/MeasurementData.h"
#include "../ie/asn/MeasurementRecord.h"
#include "../ie/asn/MeasurementRecordItem.h"
#include "../ie/asn/E2SM-KPM-RANfunction-Description.h"
#include "../ie/asn/RANfunction-Name.h"

#include "kpm_enc_asn.h"
#include "../../../util/alg_ds/alg/defer.h"
#include "../../../util/conversions.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/** Direction of the message: RIC --> E2 Node */
byte_array_t kpm_enc_event_trigger_asn(kpm_event_trigger_t const* event_trigger) {

  assert(event_trigger != NULL);
  E2SM_KPM_EventTriggerDefinition_t *pdu = calloc(1, sizeof(E2SM_KPM_EventTriggerDefinition_t));
  assert ( pdu != NULL && "Memory exhausted" );
  pdu->eventDefinition_formats.present = E2SM_KPM_EventTriggerDefinition__eventDefinition_formats_PR_eventDefinition_Format1;
  pdu->eventDefinition_formats.choice.eventDefinition_Format1 = calloc (1, sizeof(E2SM_KPM_EventTriggerDefinition_Format1_t));
  assert (pdu->eventDefinition_formats.choice.eventDefinition_Format1 != NULL && "Memory exhausted" );
  pdu->eventDefinition_formats.choice.eventDefinition_Format1->reportingPeriod = event_trigger->ms;
  
  byte_array_t ba = {.buf = malloc(2048), .len = 2048}; 
  const enum asn_transfer_syntax syntax = ATS_ALIGNED_BASIC_PER;
  asn_enc_rval_t er = asn_encode_to_buffer(NULL, syntax, &asn_DEF_E2SM_KPM_EventTriggerDefinition, pdu, ba.buf, ba.len);
  assert(er.encoded > -1 && (size_t)er.encoded <= ba.len);
  ba.len = er.encoded;
  
  ASN_STRUCT_RESET(asn_DEF_E2SM_KPM_EventTriggerDefinition, pdu);
  free(pdu);

  return ba;
}

/* Encoding Action Definition IE
 * Limitation: for now it will just manage action definition of type 1 (RIC style type = 1). 
 * Direction of the message: RIC -> E2 Node
 */
byte_array_t kpm_enc_action_def_asn(kpm_action_def_t const* action_def) 
{
  assert(action_def != NULL);
  E2SM_KPM_ActionDefinition_t *pdu = calloc(1, sizeof(E2SM_KPM_ActionDefinition_t));
  assert (pdu != NULL && "Memory exhausted");
  
  assert ((action_def->ric_style_type > 5 || action_def->ric_style_type < 1) == false && 
          "Not valid RIC style Type"); // range defined in $7.4.1
  pdu->ric_Style_Type = action_def->ric_style_type;
  
  pdu->actionDefinition_formats.present = action_def->ric_style_type;
  switch (pdu->actionDefinition_formats.present) 
  {
    case E2SM_KPM_ActionDefinition__actionDefinition_formats_PR_actionDefinition_Format1:
    
      // 1. measInfoList: it has to have at list one element
      pdu->actionDefinition_formats.choice.actionDefinition_Format1 = calloc (1, sizeof(E2SM_KPM_ActionDefinition_Format1_t));
      E2SM_KPM_ActionDefinition_Format1_t * adf_p = pdu->actionDefinition_formats.choice.actionDefinition_Format1;
      assert (adf_p != NULL && "Memory exhausted" );
      
      int ret;
      assert((action_def->MeasInfo_len <= maxnoofMeasurementInfo && action_def->MeasInfo_len > 0) 
              && "Number of measures not allowed");
      for (size_t i = 0; i<action_def->MeasInfo_len; i++)
      {
        MeasurementInfoItem_t *mInfo = calloc(1, sizeof(MeasurementInfoItem_t));
        assert (mInfo != NULL && "Memory exhausted");
        mInfo->measType.present = action_def->MeasInfo[i].measType;
        if (mInfo->measType.present == MeasurementType_PR_measName){
          ret = OCTET_STRING_fromBuf( &(mInfo->measType.choice.measName), 
                                      (char *)action_def->MeasInfo[i].measName.buf, 
                                      (int)action_def->MeasInfo[i].measName.len);
          assert(ret == 0);
        } else {
          mInfo->measType.choice.measID = action_def->MeasInfo[i].measID;
        }
        
        assert((action_def->MeasInfo[i].labelInfo_len <= maxnoofLabelInfo && action_def->MeasInfo[i].labelInfo_len > 0) 
                && "Number of labels not allowed");
        
        for (size_t j = 0; j<action_def->MeasInfo[i].labelInfo_len; j++)
        {
          LabelInfoItem_t *labels = calloc(1, sizeof(LabelInfoItem_t));
          assert (labels != NULL && "Memory exhausted");
          
          if (action_def->MeasInfo[i].labelInfo[j].noLabel != NULL) {
            labels->measLabel.noLabel = malloc (sizeof(*(labels->measLabel.noLabel)));
            assert (labels->measLabel.noLabel != NULL && "Memory exhausted");
            *(labels->measLabel.noLabel) = *(action_def->MeasInfo[i].labelInfo[j].noLabel); 
          
            int rc1 = ASN_SEQUENCE_ADD(&mInfo->labelInfoList.list, labels);
            assert(rc1 == 0);
            /* 
             * specification mentions that if 'noLabel' is included, other elements in the same datastructure 
             * 'LabelInfoItem_t' shall not be included.
             */
            continue; 
          }
          if (action_def->MeasInfo[i].labelInfo[j].plmnID != NULL){
            labels->measLabel.plmnID = OCTET_STRING_new_fromBuf (&asn_DEF_PLMNIdentity, 
                                                                (const char *)action_def->MeasInfo[i].labelInfo[j].plmnID->buf, 
                                                                action_def->MeasInfo[i].labelInfo[j].plmnID->len);
            assert(labels->measLabel.plmnID != NULL);
          }
          #if 0
          // To complete with below fields.
          // internal_S_NSSAI_t	          *sliceID;	/* OPTIONAL */
          // internal_FiveQI_t	          *fiveQI;	/* OPTIONAL */
          // internal_QosFlowIdentifier_t	*qFI;	    /* OPTIONAL */
          // internal_QCI_t	              *qCI;	    /* OPTIONAL */
          // internal_QCI_t	              *qCImax;	/* OPTIONAL */
          // internal_QCI_t	              *qCImin;	/* OPTIONAL */
          // long	              *aRPmax;	/* OPTIONAL */
          // long	              *aRPmin;	/* OPTIONAL */
          // long	              *bitrateRange;/* OPTIONAL */
          // long	              *layerMU_MIMO;/* OPTIONAL */
          // long	              *sUM;	    /* OPTIONAL */
          // long	              *distBinX;/* OPTIONAL */
          // long	              *distBinY;/* OPTIONAL */
          // long	              *distBinZ;/* OPTIONAL */
          // long	              *preLabelOverride;/* OPTIONAL */
          // long	              *startEndInd;	/* OPTIONAL */
          // long	              *min;	    /* OPTIONAL */
          // long	              *max;	    /* OPTIONAL */
          // long	              *avg;	    /* OPTIONAL */
          #endif
          int rc1 = ASN_SEQUENCE_ADD(&mInfo->labelInfoList.list, labels);
          assert(rc1 == 0);
        }
        int rc2 = ASN_SEQUENCE_ADD(&adf_p->measInfoList.list, mInfo);
        assert(rc2 == 0);
      }

      // 2. granular period
      adf_p->granulPeriod = action_def->granularity_period;
      
      // 3. cell Global ID (optional)
      if (action_def->cellGlobalIDtype == choice_NOTHING)
        break;

      adf_p->cellGlobalID->present = action_def->cellGlobalIDtype;
      if (action_def->cellGlobalIDtype == choice_nR_CGI) {
          ret = OCTET_STRING_fromBuf( &adf_p->cellGlobalID->choice.nR_CGI->pLMNIdentity, 
                                      (const char *)action_def->pLMNIdentity.buf, 
                                      action_def->pLMNIdentity.len);
          assert(ret == 0);
          defer({OCTET_STRING_free(&asn_DEF_PLMNIdentity, &adf_p->cellGlobalID->choice.nR_CGI->pLMNIdentity, ASFM_FREE_UNDERLYING);});

          NR_CELL_ID_TO_BIT_STRING(action_def->nRCellIdentity, &adf_p->cellGlobalID->choice.nR_CGI->nRCellIdentity);
	        defer({BIT_STRING_free(&asn_DEF_NRCellIdentity, &adf_p->cellGlobalID->choice.nR_CGI->nRCellIdentity, ASFM_FREE_UNDERLYING);});
      } else if (action_def->cellGlobalIDtype == choice_eUTRA_CGI){
        ret = OCTET_STRING_fromBuf(&adf_p->cellGlobalID->choice.eUTRA_CGI->pLMNIdentity, 
                                   (const char *)action_def->pLMNIdentity.buf, 
                                   action_def->pLMNIdentity.len);
        assert(ret == 0);
        defer({OCTET_STRING_free(&asn_DEF_PLMNIdentity, &adf_p->cellGlobalID->choice.eUTRA_CGI->pLMNIdentity, ASFM_FREE_UNDERLYING);});

        NR_CELL_ID_TO_BIT_STRING(action_def->eUTRACellIdentity, &adf_p->cellGlobalID->choice.eUTRA_CGI->eUTRACellIdentity);
	      defer({BIT_STRING_free(&asn_DEF_EUTRACellIdentity, &adf_p->cellGlobalID->choice.eUTRA_CGI->eUTRACellIdentity, ASFM_FREE_UNDERLYING);});
      }
      break;
    default: 
      assert(0!=0 && "Unexpected action definition format. Only type 1 is suppoorted");
  }

  /* XXX-tuning: 
   * below bytearray sizing needs to be reviewed and made dynamic. It looks too small for the general case of action definition.
   */
  byte_array_t ba = {.buf = malloc(2048), .len = 2048}; 
  const enum asn_transfer_syntax syntax = ATS_ALIGNED_BASIC_PER;
  asn_enc_rval_t er = asn_encode_to_buffer(NULL, syntax, &asn_DEF_E2SM_KPM_ActionDefinition, pdu, ba.buf, ba.len);
  assert(er.encoded > -1 && (size_t)er.encoded <= ba.len);
  ba.len = er.encoded;

  ASN_STRUCT_RESET(asn_DEF_E2SM_KPM_ActionDefinition, pdu);
  free(pdu);

  return ba;
}

/* 
 * Encoding of Indication Header IE
 * Direction: NodeE2 -> RIC 
 */
byte_array_t kpm_enc_ind_hdr_asn(kpm_ind_hdr_t const* ind_hdr)
{
  assert(ind_hdr != NULL);

  E2SM_KPM_IndicationHeader_t *pdu = calloc(1,sizeof(E2SM_KPM_IndicationHeader_t));
  assert( pdu !=NULL && "Memory exhausted" );

  pdu->indicationHeader_formats.present = E2SM_KPM_IndicationHeader__indicationHeader_formats_PR_indicationHeader_Format1;
  pdu->indicationHeader_formats.choice.indicationHeader_Format1 = calloc(1, sizeof(E2SM_KPM_IndicationHeader_Format1_t));
  E2SM_KPM_IndicationHeader_Format1_t *ih_p = pdu->indicationHeader_formats.choice.indicationHeader_Format1;
  int ret = OCTET_STRING_fromBuf(&ih_p->colletStartTime, 
                                  (const char *)ind_hdr->collectStartTime.buf, 
                                  ind_hdr->collectStartTime.len);
  assert(ret == 0);
  
  if (ind_hdr->fileFormatversion != NULL){
    ret = OCTET_STRING_fromBuf(ih_p->fileFormatversion, 
                              (const char *)ind_hdr->fileFormatversion->buf, 
                              ind_hdr->fileFormatversion->len);
    assert(ret == 0);
    defer({OCTET_STRING_free(&asn_DEF_PrintableString, ih_p->fileFormatversion, ASFM_FREE_EVERYTHING);});
  }

  if (ind_hdr->senderName != NULL){
    ret = OCTET_STRING_fromBuf(ih_p->senderName, 
                              (const char *)ind_hdr->senderName->buf, 
                              ind_hdr->senderName->len);
    assert(ret == 0);
    defer({OCTET_STRING_free(&asn_DEF_PrintableString, ih_p->senderName, ASFM_FREE_EVERYTHING);});
  }

  if (ind_hdr->senderType != NULL){
    ret = OCTET_STRING_fromBuf(ih_p->senderType, 
                              (const char *)ind_hdr->senderType->buf, 
                              ind_hdr->senderType->len);
    assert(ret == 0);
    defer({OCTET_STRING_free(&asn_DEF_PrintableString, ih_p->senderType, ASFM_FREE_EVERYTHING);});
  }

	if (ind_hdr->vendorName != NULL) {
    ret = OCTET_STRING_fromBuf(ih_p->vendorName, 
                              (const char *)ind_hdr->vendorName->buf, 
                              ind_hdr->vendorName->len);
    assert(ret == 0);
    defer({OCTET_STRING_free(&asn_DEF_PrintableString, ih_p->vendorName, ASFM_FREE_EVERYTHING);});
  }

  byte_array_t  ba = {.buf = malloc(2048), .len = 2048};
  const enum asn_transfer_syntax syntax = ATS_ALIGNED_BASIC_PER;
  asn_enc_rval_t er = asn_encode_to_buffer(NULL, syntax, &asn_DEF_E2SM_KPM_IndicationHeader, pdu, ba.buf, ba.len);
  assert(er.encoded > -1 && (size_t)er.encoded <= ba.len);
  ba.len = er.encoded;


  ASN_STRUCT_RESET(asn_DEF_E2SM_KPM_IndicationHeader, pdu);
  free(pdu);

  return ba;
}
/* 
 * Encoding of Indication Message IE. 
 * Limitation: just format 1 is implemented further to Action Definition IE type 1 constraint
 * Direction: NodeE2 -> RIC 
 */
byte_array_t kpm_enc_ind_msg_asn(kpm_ind_msg_t const* ind_msg) 
{
  assert(ind_msg != NULL);
  
  E2SM_KPM_IndicationMessage_t *pdu = calloc(1, sizeof(E2SM_KPM_IndicationMessage_t));
  assert( pdu !=NULL && "Memory exhausted" );

  pdu->indicationMessage_formats.present = E2SM_KPM_IndicationMessage__indicationMessage_formats_PR_indicationMessage_Format1;
  pdu->indicationMessage_formats.choice.indicationMessage_Format1 = calloc(1, sizeof(E2SM_KPM_IndicationMessage_Format1_t));
  E2SM_KPM_IndicationMessage_Format1_t *msg = pdu->indicationMessage_formats.choice.indicationMessage_Format1;
  assert( msg !=NULL && "Memory exhausted" );

  // 1. measData
  assert((ind_msg->MeasData_len <= maxnoofMeasurementRecord && ind_msg->MeasData_len > 0) 
          && "Number of records not allowed");
  for (size_t i = 0; i<ind_msg->MeasData_len; i++)
  {
    MeasurementDataItem_t *mData = calloc(1, sizeof(MeasurementDataItem_t));
    assert (mData != NULL && "Memory exhausted");
    assert (ind_msg->MeasData[i].measRecord_len <= maxnoofMeasurementValue && ind_msg->MeasData[i].measRecord_len >0
            && "Number of Values not allowed");
    if (ind_msg->MeasData[i].incompleteFlag == MeasurementDataItem__incompleteFlag_true){
      mData->incompleteFlag = malloc (sizeof( *mData->incompleteFlag));
      assert(mData->incompleteFlag != NULL && "Memory exhausted");
      *(mData->incompleteFlag) = MeasurementDataItem__incompleteFlag_true; 
    }
    for (size_t j=0; j<ind_msg->MeasData[i].measRecord_len; j++){
      MeasurementRecordItem_t *mRecord = calloc(1, sizeof(MeasurementRecordItem_t));
      assert (mRecord != NULL && "Memory exhausted");
      switch (ind_msg->MeasData[i].measRecord[j].type){
        case MeasRecord_int:
          mRecord->choice.integer = ind_msg->MeasData[i].measRecord[j].int_val;
          break;
        case MeasRecord_real:
          mRecord->choice.real = ind_msg->MeasData[i].measRecord[j].real_val;
          break;
        case MeasRecord_noval:
          mRecord->choice.noValue = 0;
          break;
        default:
          assert(0!= 0 && "unexpected Record type");
      }
      mRecord->present = ind_msg->MeasData[i].measRecord[j].type;
      int rc1 = ASN_SEQUENCE_ADD(&mData->measRecord.list, mRecord);
      assert(rc1 == 0);
    }
    int rc2 = ASN_SEQUENCE_ADD(&msg->measData.list, mData);
    assert(rc2 == 0);
  }

  // 2. measInfoList (OPTIONAL)
  // TODO: for now let's put it NULL
  msg->measInfoList = NULL;

  // 3. granulPeriod (OPTIONAL)
  // TODO
  msg->granulPeriod = NULL;

  byte_array_t  ba = {.buf = malloc(2048), .len = 2048};
  const enum asn_transfer_syntax syntax = ATS_ALIGNED_BASIC_PER;
  asn_enc_rval_t er = asn_encode_to_buffer(NULL, syntax, &asn_DEF_E2SM_KPM_IndicationMessage, pdu, ba.buf, ba.len);
  assert(er.encoded > -1 && (size_t)er.encoded <= ba.len);
  ba.len = er.encoded;


  ASN_STRUCT_RESET(asn_DEF_E2SM_KPM_IndicationMessage, pdu);
  free(pdu);

  return ba;
}


/*
 * Encoding of Function Definition IE. 
 * Direction: NodeE2 -> RIC 
 */
byte_array_t kpm_enc_func_def_asn(kpm_func_def_t const* func_def)  
{
  assert(func_def != NULL);
  
  E2SM_KPM_RANfunction_Description_t *pdu = calloc(1, sizeof(E2SM_KPM_RANfunction_Description_t));
  assert( pdu !=NULL && "Memory exhausted" );
  
// Let's encode the minimum compulsory arguments 
  int ret = OCTET_STRING_fromBuf(&pdu->ranFunction_Name.ranFunction_Description, 
                              (const char *)func_def->ranFunction_Name.Description.buf, 
                              func_def->ranFunction_Name.Description.len);
  assert(ret == 0);
  
  ret = OCTET_STRING_fromBuf(&pdu->ranFunction_Name.ranFunction_E2SM_OID, 
                              (const char *)func_def->ranFunction_Name.E2SM_OID.buf, 
                              func_def->ranFunction_Name.E2SM_OID.len);
  assert(ret == 0);
  
  ret = OCTET_STRING_fromBuf(&pdu->ranFunction_Name.ranFunction_ShortName, 
                              (const char *)func_def->ranFunction_Name.ShortName.buf, 
                              func_def->ranFunction_Name.ShortName.len);
  assert(ret == 0);
  
  pdu->ranFunction_Name.ranFunction_Instance = NULL;

  byte_array_t  ba = {.buf = malloc(2048), .len = 2048};
  const enum asn_transfer_syntax syntax = ATS_ALIGNED_BASIC_PER;
  asn_enc_rval_t er = asn_encode_to_buffer(NULL, syntax, &asn_DEF_E2SM_KPM_RANfunction_Description, pdu, ba.buf, ba.len);
  assert(er.encoded > -1 && (size_t)er.encoded <= ba.len);
  ba.len = er.encoded;


  ASN_STRUCT_RESET(asn_DEF_E2SM_KPM_RANfunction_Description, pdu);
  free(pdu);

  return ba;
}