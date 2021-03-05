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

#ifndef MULTI_CONTROL_H
#define MULTI_CONTROL_H

#include <stdint.h>
#include <stdbool.h>
#include "e2ap_agent.h"

typedef int ep_id_t;

ep_id_t get_endpoint(e2ap_agent_t* ag, int fd);

ep_id_t add_endpoint(e2ap_agent_t* ag, const char* addr, int port, const e2_setup_request_t* sr);
void remove_endpoint(e2ap_agent_t* ag, ep_id_t ep_id);

void endpoint_associate_rnti(e2ap_agent_t* ag, ep_id_t ep_id, uint16_t rnti);
void endpoint_dissociate_rnti(e2ap_agent_t* ag, ep_id_t ep_id, uint16_t rnti);
bool endpoint_has_rnti(e2ap_agent_t* ag, ep_id_t ep_id, uint16_t rnti);
void endpoint_rnti_iterate(e2ap_agent_t* ag, ep_id_t ep_id, void (*func)(uint16_t, void*), void* data);

#endif /* MULTI_CONTROL_H */
