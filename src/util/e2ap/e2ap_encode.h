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

#ifndef ENCODE_E2AP_H
#define ENCODE_E2AP_H

#include <assert.h>
#include "type_defs.h"

struct E2AP_PDU;
typedef e2ap_msg_t (*e2ap_gen_fp)(const struct E2AP_PDU*);

struct e2ap_E2Message_table;
typedef e2ap_msg_t (*e2ap_gen_dir_fp)(const struct e2ap_E2Message_table*);
typedef void (*e2ap_free_fp)(e2ap_msg_t*);

typedef struct e2ap_asn
{
  e2ap_gen_fp dec_msg[26];
  e2ap_free_fp free_msg[26];
} e2ap_asn_t;

typedef struct e2ap_fb
{
  e2ap_gen_dir_fp dec_msg[26];
  e2ap_free_fp free_msg[26];
} e2ap_fb_t;



#define init_enc(T) _Generic ((T),  e2ap_asn_t*:  init_enc_asn, \
                                    e2ap_fb_t*:   init_enc_fb, \
                                    default:   init_enc_asn) (T)



typedef struct e2ap_enc
{
#ifdef ASN
 e2ap_asn_t type;
#elif FLATBUFFERS
 e2ap_fb_t type;
#else
  static_assert(0!=0, "Error. No encoding scheme selected");
#endif
} e2ap_enc_t;


#endif
