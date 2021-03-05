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

#ifndef RRC_STATS_DEFS_H
#define RRC_STATS_DEFS_H

#include <stdint.h>

// Fake OID so the controller recognizes the stats
// iso(1) identified-organization(3) dod(6) internet(1) private(4) enterprise(1) 53148 e2(1) version1 (1) e2sm_fb(99) rrc_stats(135)  // last two would be: e2sm(2) e2sm-KPM-IEs (2)
__attribute__((unused)) static const char* rrc_stats_oid = "1.3.6.1.4.1.1.1.99.135";

// this should be equal to the Flatbuffers type rrc
typedef enum {
  RRC_STATS_REPORT_OCCASION_ONCHANGE = 0
} rrc_stats_report_occasion_e;

// this should be equal to the Flatbuffers type rrc
typedef enum {
  RRC_STATS_REPORT_STYLE_TYPE_MINIMAL = 0,
  RRC_STATS_REPORT_STYLE_TYPE_COMPLETE = 1
} rrc_stats_report_style_type_e;

typedef struct rrc_stats_report_style_s {
  rrc_stats_report_style_type_e type;
} rrc_stats_report_style_t;

struct flatcc_builder;
typedef void (*rrc_stats_fill_ind_msg_cb)(struct flatcc_builder*, const rrc_stats_report_style_t* style, void*);

#endif /* RRC_STATS_DEFS_H */
