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

#ifndef SM_INPUT_OUTPUT_H
#define SM_INPUT_OUTPUT_H

#include "agent_if/read/sm_ag_if_rd.h"
#include "agent_if/write/sm_ag_if_wr.h"
#include "agent_if/ans/sm_ag_if_ans.h"

// The SM agent uses this two functions to communicate with the RAN and with the server.
typedef struct{

  void (*read)(sm_ag_if_rd_t* data);

  sm_ag_if_ans_t (*write)(sm_ag_if_wr_t const* data);

} sm_io_ag_t;

#endif

