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


#include "mac_data_ie.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "../../../util/alg_ds/alg/eq_float.h"


//////////////////////////////////////
// RIC Event Trigger Definition
/////////////////////////////////////

void free_mac_event_trigger(mac_event_trigger_t* src)
{
  assert(src != NULL);
  assert(0!=0 && "Not implemented" ); 
}

mac_event_trigger_t cp_mac_event_trigger( mac_event_trigger_t* src)
{
  assert(src != NULL);
  assert(0!=0 && "Not implemented" ); 

  mac_event_trigger_t et = {0};
  return et;
}

bool eq_mac_event_trigger(mac_event_trigger_t* m0, mac_event_trigger_t* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);

  assert(0!=0 && "Not implemented" ); 

  return true;
}


//////////////////////////////////////
// RIC Action Definition 
/////////////////////////////////////

void free_mac_action_def(mac_action_def_t* src)
{
  assert(src != NULL);

  assert(0!=0 && "Not implemented" ); 
}

mac_action_def_t cp_mac_action_def(mac_action_def_t* src)
{
  assert(src != NULL);

  assert(0!=0 && "Not implemented" ); 
  mac_action_def_t ad = {0};
  return ad;
}

bool eq_mac_action_def(mac_event_trigger_t* m0,  mac_event_trigger_t* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);

  assert(0!=0 && "Not implemented" ); 

  return true;
}


//////////////////////////////////////
// RIC Indication Header 
/////////////////////////////////////


void free_mac_ind_hdr(mac_ind_hdr_t* src)
{
  assert(src != NULL);
  (void)src;
}

mac_ind_hdr_t cp_mac_ind_hdr(mac_ind_hdr_t const* src)
{
  assert(src != NULL);
  mac_ind_hdr_t dst = {0}; 
  dst.dummy = src->dummy;
  return dst;
}

bool eq_mac_ind_hdr(mac_ind_hdr_t* m0, mac_ind_hdr_t* m1)
{
  assert(m0 != 0);
  assert(m1 != 0);

  if(m0->dummy != m1->dummy)
    return false;
  return true;
}


//////////////////////////////////////
// RIC Indication Message 
/////////////////////////////////////

void free_mac_ind_msg(mac_ind_msg_t* src)
{
  assert(src != NULL);
  if(src->len_ue_stats > 0){
    assert(src->ue_stats != NULL);
    free(src->ue_stats);
  }
}

mac_ue_stats_impl_t cp_mac_ue_stats_impl(mac_ue_stats_impl_t const* src)
{
  assert(src != NULL);

  mac_ue_stats_impl_t dst = { .dl_aggr_tbs = src->dl_aggr_tbs,
                              .ul_aggr_tbs = src->ul_aggr_tbs,
                              .dl_aggr_bytes_sdus = src->dl_aggr_bytes_sdus,
                              .ul_aggr_bytes_sdus = src->ul_aggr_bytes_sdus,

                              .pusch_snr = src->pusch_snr, //: float = -64;
                              .pucch_snr = src->pucch_snr, //: float = -64;

                              .rnti = src->rnti,
                              .dl_aggr_prb = src->dl_aggr_prb, 
                              .ul_aggr_prb = src->ul_aggr_prb,
                              .dl_aggr_sdus = src->dl_aggr_sdus,
                              .ul_aggr_sdus = src->ul_aggr_sdus,
                              .dl_aggr_retx_prb = src->dl_aggr_retx_prb,

                              .wb_cqi = src->wb_cqi, 
                              .dl_mcs1 = src->dl_mcs1,
                              .ul_mcs1 = src->ul_mcs1,
                              .dl_mcs2 = src->dl_mcs2, 
                              .ul_mcs2 = src->ul_mcs2, 
                              .phr = src->phr 
                            }; 

  return dst;
}

mac_ind_msg_t cp_mac_ind_msg( mac_ind_msg_t const* src)
{
  assert(src != NULL);

  mac_ind_msg_t dst = {0};

  dst.len_ue_stats = src->len_ue_stats;
  if(dst.len_ue_stats > 0){
    dst.ue_stats = calloc(dst.len_ue_stats, sizeof( mac_ue_stats_impl_t) );
    assert(dst.ue_stats != NULL && "Memory exhausted");
  }

  for(size_t i = 0; i < dst.len_ue_stats; ++i){
    dst.ue_stats[i] = cp_mac_ue_stats_impl(&src->ue_stats[i]); 
  }

  //memcpy(ret.ue_stats, src->ue_stats, sizeof( mac_ue_stats_impl_t )*ret.len_ue_stats);

  dst.tstamp = src->tstamp; 

  return dst;
}

bool eq_mac_ind_msg(mac_ind_msg_t* m0, mac_ind_msg_t* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);

  if(m0->len_ue_stats != m1->len_ue_stats || m0->tstamp != m1->tstamp)
    return false;

  for(uint32_t i = 0 ; i < m0->len_ue_stats; ++i){
    mac_ue_stats_impl_t* ue0 = &m0->ue_stats[i]; 
    mac_ue_stats_impl_t* ue1 = &m1->ue_stats[i]; 

    if(
        ue0->dl_aggr_tbs != ue1->dl_aggr_tbs ||
        ue0->ul_aggr_tbs != ue1->ul_aggr_tbs ||
        ue0->dl_aggr_bytes_sdus != ue1->dl_aggr_bytes_sdus ||
        ue0->ul_aggr_bytes_sdus != ue1->ul_aggr_bytes_sdus ||
        ue0->rnti != ue1->rnti ||
        ue0->dl_aggr_prb != ue1->dl_aggr_prb ||  
        ue0->ul_aggr_prb != ue1->ul_aggr_prb ||
        ue0->dl_aggr_sdus != ue1-> dl_aggr_sdus ||
        ue0->ul_aggr_sdus != ue1->ul_aggr_sdus ||
        ue0->dl_aggr_retx_prb != ue1->dl_aggr_retx_prb ||
        ue0->wb_cqi != ue1->wb_cqi || 
        ue0->dl_mcs1 != ue1->dl_mcs1 ||
        ue0->ul_mcs1 != ue1->ul_mcs1 ||
        ue0->dl_mcs2 != ue1->dl_mcs2 || 
        ue0->ul_mcs2 != ue1->ul_mcs2 || 
        ue0->phr != ue1->phr || 
        eq_float(ue0->pusch_snr, ue1->pusch_snr, 0.0000001) == false ||
        eq_float(ue0->pucch_snr, ue0->pucch_snr, 0.0000001) == false 
      )
      return false;
  }
  return true;
}

//////////////////////////////////////
// RIC Call Process ID 
/////////////////////////////////////

void free_mac_call_proc_id(mac_call_proc_id_t* src)
{
  // Note that the src could be NULL
  free(src);
}

mac_call_proc_id_t cp_mac_call_proc_id( mac_call_proc_id_t* src)
{
  assert(src != NULL); 
  mac_call_proc_id_t dst = {0};

  dst.dummy = src->dummy;

  return dst;
}

bool eq_mac_call_proc_id(mac_call_proc_id_t* m0, mac_call_proc_id_t* m1)
{
  if(m0 == NULL && m1 == NULL)
    return true;
  if(m0 == NULL)
    return false;
  if(m1 == NULL)
    return false;

  if(m0->dummy != m1->dummy)
    return false;

  return true;
}


//////////////////////////////////////
// RIC Control Header 
/////////////////////////////////////

void free_mac_ctrl_hdr( mac_ctrl_hdr_t* src)
{

  assert(src != NULL);
  assert(0!=0 && "Not implemented" ); 
}

mac_ctrl_hdr_t cp_mac_ctrl_hdr(mac_ctrl_hdr_t* src)
{
  assert(src != NULL);
  assert(0!=0 && "Not implemented" ); 
  mac_ctrl_hdr_t ret = {0};
  return ret;
}

bool eq_mac_ctrl_hdr(mac_ctrl_hdr_t* m0, mac_ctrl_hdr_t* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);

  assert(0!=0 && "Not implemented" ); 

  return true;
}


//////////////////////////////////////
// RIC Control Message 
/////////////////////////////////////


void free_mac_ctrl_msg( mac_ctrl_msg_t* src)
{
  assert(src != NULL);

  assert(0!=0 && "Not implemented" ); 
}

mac_ctrl_msg_t cp_mac_ctrl_msg(mac_ctrl_msg_t* src)
{
  assert(src != NULL);

  assert(0!=0 && "Not implemented" ); 
  mac_ctrl_msg_t ret = {0};
  return ret;
}

bool eq_mac_ctrl_msg(mac_ctrl_msg_t* m0, mac_ctrl_msg_t* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);

  assert(0!=0 && "Not implemented" ); 

  return true;
}


//////////////////////////////////////
// RIC Control Outcome 
/////////////////////////////////////

void free_mac_ctrl_out(mac_ctrl_out_t* src)
{
  assert(src != NULL);

  assert(0!=0 && "Not implemented" ); 
}

mac_ctrl_out_t cp_mac_ctrl_out(mac_ctrl_out_t* src)
{
  assert(src != NULL);

  assert(0!=0 && "Not implemented" ); 
  mac_ctrl_out_t ret = {0}; 
  return ret;
}

bool eq_mac_ctrl_out(mac_ctrl_out_t* m0, mac_ctrl_out_t* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);

  assert(0!=0 && "Not implemented" ); 

  return true;
}


//////////////////////////////////////
// RAN Function Definition 
/////////////////////////////////////

void free_mac_func_def( mac_func_def_t* src)
{
  assert(src != NULL);

  assert(0!=0 && "Not implemented" ); 
}

mac_func_def_t cp_mac_func_def(mac_func_def_t* src)
{
  assert(src != NULL);

  assert(0!=0 && "Not implemented" ); 
  mac_func_def_t ret = {0};
  return ret;
}

bool eq_mac_func_def(mac_func_def_t* m0, mac_func_def_t* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);

  assert(0!=0 && "Not implemented" ); 
  return true;
}

///////////////
// RIC Indication
///////////////

mac_ind_data_t cp_mac_ind_data( mac_ind_data_t const* src)
{
  assert(src != NULL);
  mac_ind_data_t dst = {0};
  dst.hdr = cp_mac_ind_hdr(&src->hdr);
  dst.msg = cp_mac_ind_msg(&src->msg);
  
  if(src->proc_id != NULL){
    dst.proc_id = malloc(sizeof(mac_call_proc_id_t)); 
    assert(dst.proc_id != NULL && "Memory exhausted");
    *dst.proc_id = cp_mac_call_proc_id(src->proc_id);
  }

  return dst;
}

void free_mac_ind_data(mac_ind_data_t* ind)
{
  assert(ind != NULL);
  free_mac_ind_hdr(&ind->hdr);
  free_mac_ind_msg(&ind->msg);
  free_mac_call_proc_id(ind->proc_id);
}


