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


#ifndef NEAR_RIC_SERVER_API
#define NEAR_RIC_SERVER_API

#include <stddef.h>
#include <stdint.h>

void init_near_ric_api(const char* addr /*, int port*/ );

void stop_near_ric_api(void);

size_t e2_nodes(void);

// NEAR-RT RIC services
// 4 basic Service reports defined 
// in Near-Real-time RAN Intelligent Controller
// E2 Service Model (E2SM)

void report_service_near_ric_api(/*global_e2_node_id_t const* id,*/ uint16_t ran_func_id, const char* cmd );

void rm_report_service_near_ric_api(/*global_e2_node_id_t const* id,*/ uint16_t ran_func_id, const char* cmd );

void control_service_near_ric_api(uint16_t sm_id, const char* cmd);

void insert_service_near_ric_api(uint16_t sm_id, const char* cmd);

void policy_service_near_ric_api(uint16_t sm_id, const char* cmd);


// NEAR-RT RIC support functions:
// Interface Management (E2 Setup, E2 Reset, 
// E2 Node Configuration Update, Reporting of 
// General Error Situations)
// Near-RT RIC Service Update, i.e. a E2 Node 
// initiated procedure to inform Near-RT RIC of
// changes to list of supported Near-RT RIC 
// services and mapping of services to functions.


// Plug-in functions
void load_sm_near_ric_api(const char* file_path);


// Observer pattern. Interface for subscription/publish for xApps
typedef struct{
  void (*update)(const char* data);
} subs_t;

void susbscribe_near_ric(/*global_e2_node_id_t const* id,*/ uint16_t sm_id, subs_t subscription);

void unsusbscribe_near_ric( /*global_e2_node_id_t const* id,*/ uint16_t sm_id, subs_t subscription);

// ToDo: Provide connected Agents and their ran functions 
// (note that the interface should be callable from Python, Go
// and NodeJS, at least)


#endif

