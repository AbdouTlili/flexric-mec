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

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "global_consts.h"
#include "type_defs.h"

#define BOTH_PTRS_NULL(a, b) ((a) == NULL && (a) == (b))


/*
bool cmp_ric_gen_id(ric_gen_id_t a, ric_gen_id_t b)
{
  return a.ric_req_id == b.ric_req_id
      && a.ric_inst_id == b.ric_inst_id
      && a.ran_func_id == b.ran_func_id;
}

ric_gen_id_t gen_ric_id(uint16_t ric_req_id, uint16_t ric_inst_id, uint16_t ran_func_id)
{
  assert(ran_func_id <= MAX_NUM_RAN_FUNC_ID);
  const ric_gen_id_t r = {
    .ric_req_id = ric_req_id, .ric_inst_id = ric_inst_id, .ran_func_id = ran_func_id
  };
  return r;
}

bool cmp_byte_array(const byte_array_t* a, const byte_array_t* b)
{
  return a->len == b->len && memcmp(a->buf, b->buf, a->len) == 0;
}



bool cmp_ran_function(const ran_function_t* a, const ran_function_t* b)
{
  return a->id == b->id
      && a->def.len == b->def.len
      && cmp_byte_array(&a->def, &b->def)
      && a->rev == b->rev
      && (BOTH_PTRS_NULL(a->oid, b->oid) || strcmp(a->oid, b->oid) == 0);
}

void free_ran_function(ran_function_t* rf)
{
  free(rf->def.buf);
  if (rf->oid){
    free(rf->oid->buf);
    free(rf->oid);
  }
}

bool cmp_e2_node_component_config_update(const e2_node_component_config_update_t* a,
                                         const e2_node_component_config_update_t* b)
{
  if (a->type != b->type)
    return false;

  // check one is null and the other is not 
  if (a->id_present == NULL && a->id_present != b->id_present)
    return false;
  if (a->id_present != NULL) {
    if (*a->id_present != *b->id_present)
      return false;
    switch (*a->id_present) {
      case GNB_CU_UP_ID:
        if (a->gnb_cu_up_id != b->gnb_cu_up_id)
          return false;
        break;
      case GNB_DU_ID:
        if (a->gnb_du_id != b->gnb_du_id)
          return false;
        break;
    }
  }

  if (a->update_present != b->update_present)
    return false;
  switch (a->update_present) {
    case GNB:
      return (   BOTH_PTRS_NULL(a->gnb.ngap_gnb_cu_cp, b->gnb.ngap_gnb_cu_cp)
              || cmp_byte_array(a->gnb.ngap_gnb_cu_cp, b->gnb.ngap_gnb_cu_cp))
          && (   BOTH_PTRS_NULL(a->gnb.xnap_gnb_cu_cp, b->gnb.xnap_gnb_cu_cp)
              || cmp_byte_array(a->gnb.xnap_gnb_cu_cp, b->gnb.xnap_gnb_cu_cp))
          && (   BOTH_PTRS_NULL(a->gnb.e1ap_gnb_cu_cp, b->gnb.e1ap_gnb_cu_cp)
              || cmp_byte_array(a->gnb.e1ap_gnb_cu_cp, b->gnb.e1ap_gnb_cu_cp))
          && (   BOTH_PTRS_NULL(a->gnb.f1ap_gnb_cu_cp, b->gnb.f1ap_gnb_cu_cp)
              || cmp_byte_array(a->gnb.f1ap_gnb_cu_cp, b->gnb.f1ap_gnb_cu_cp));
    case EN_GNB:
      return     BOTH_PTRS_NULL(a->en_gnb.x2ap_en_gnb, b->en_gnb.x2ap_en_gnb)
              || cmp_byte_array(a->en_gnb.x2ap_en_gnb, b->en_gnb.x2ap_en_gnb);
    case NG_ENB:
      return (   BOTH_PTRS_NULL(a->ng_enb.ngap_ng_enb, b->ng_enb.ngap_ng_enb)
              || cmp_byte_array(a->ng_enb.ngap_ng_enb, b->ng_enb.ngap_ng_enb))
          && (   BOTH_PTRS_NULL(a->ng_enb.xnap_ng_enb, b->ng_enb.xnap_ng_enb)
              || cmp_byte_array(a->ng_enb.xnap_ng_enb, b->ng_enb.xnap_ng_enb));
    case ENB:
      return (   BOTH_PTRS_NULL(a->enb.s1ap_enb, b->enb.s1ap_enb)
              || cmp_byte_array(a->enb.s1ap_enb, b->enb.s1ap_enb))
          && (   BOTH_PTRS_NULL(a->enb.x2ap_enb, b->enb.x2ap_enb)
              || cmp_byte_array(a->enb.x2ap_enb, b->enb.x2ap_enb));
  }
  return true;
}

void free_e2_node_component_config_update(e2_node_component_config_update_t* nccu)
{
  if (nccu->id_present)
    free(nccu->id_present);
  switch (nccu->update_present) {
    case GNB:
      if (nccu->gnb.ngap_gnb_cu_cp) {
        free_byte_array(nccu->gnb.ngap_gnb_cu_cp);
        free(nccu->gnb.ngap_gnb_cu_cp);
      }
      if (nccu->gnb.xnap_gnb_cu_cp) {
        free_byte_array(nccu->gnb.xnap_gnb_cu_cp);
        free(nccu->gnb.xnap_gnb_cu_cp);
      }
      if (nccu->gnb.e1ap_gnb_cu_cp) {
        free_byte_array(nccu->gnb.e1ap_gnb_cu_cp);
        free(nccu->gnb.e1ap_gnb_cu_cp);
      }
      if (nccu->gnb.f1ap_gnb_cu_cp) {
        free_byte_array(nccu->gnb.f1ap_gnb_cu_cp);
        free(nccu->gnb.f1ap_gnb_cu_cp);
      }
      break;
    case EN_GNB:
      if (nccu->en_gnb.x2ap_en_gnb) {
        free_byte_array(nccu->en_gnb.x2ap_en_gnb);
        free(nccu->en_gnb.x2ap_en_gnb);
      }
      if (nccu->en_gnb.x2ap_en_gnb) {
        free_byte_array(nccu->en_gnb.x2ap_en_gnb);
        free(nccu->en_gnb.x2ap_en_gnb);
      }
      break;
    case NG_ENB:
      if (nccu->ng_enb.ngap_ng_enb) {
        free_byte_array(nccu->ng_enb.ngap_ng_enb);
        free(nccu->ng_enb.ngap_ng_enb);
      }
      if (nccu->ng_enb.xnap_ng_enb) {
        free_byte_array(nccu->ng_enb.xnap_ng_enb);
        free(nccu->ng_enb.xnap_ng_enb);
      }
      break;
    case ENB:
      if (nccu->enb.s1ap_enb) {
        free(nccu->enb.s1ap_enb);
      }
      if (nccu->enb.x2ap_enb) {
        free(nccu->enb.x2ap_enb);
      }
      break;
  }
}

bool cmp_plmn(plmn_t a, plmn_t b)
{
  return a.mcc == b.mcc && a.mnc == b.mnc && a.mnc_digit_len == b.mnc_digit_len;
}

bool cmp_global_e2_node_id(const global_e2_node_id_t* a, const global_e2_node_id_t* b)
{
  return a->type == b->type
      && cmp_plmn(a->plmn, b->plmn)
      && a->nb_id == b->nb_id;
}

bool cmp_e2_setup_request(const e2_setup_request_t* a, const e2_setup_request_t* b)
{
  bool ret = cmp_global_e2_node_id(&a->id, &b->id)
          && a->len_rf == b->len_rf
          && a->len_ccu == b->len_ccu;
  if (!ret)
    return false;
  for (size_t i = 0; i < a->len_rf; ++i)
    if (!cmp_ran_function(&a->ran_func_item[i], &b->ran_func_item[i]))
      return false;
  for (size_t i = 0; i < a->len_ccu; ++i)
    if (!cmp_e2_node_component_config_update(&a->comp_conf_update[i], &b->comp_conf_update[i]))
      return false;
  return true;
}

void free_e2_setup_request(e2_setup_request_t* sr)
{
  for (size_t i = 0; i < sr->len_rf; ++i)
    free_ran_function(&sr->ran_func_item[i]);
  if (sr->ran_func_item)
    free(sr->ran_func_item);
  for (size_t i = 0; i < sr->len_ccu; ++i)
    free_e2_node_component_config_update(&sr->comp_conf_update[i]);
  if (sr->comp_conf_update)
    free(sr->comp_conf_update);
}
*/

