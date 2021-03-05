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

#include <string.h>
#include "e2sm_hw_app.h"
#include "e2sm_hw_rf.h"


int main()
{
  /* the action definition has nothing useful, so we don't get any parameter
   * here. */
  byte_array_t b;
  b = e2sm_hw_generate_action_definition();
  e2sm_hw_decode_action_definition(b);
  free_byte_array(b);

  b = e2sm_hw_generate_event_trigger_definition(HW_TriggerNature_onchange);
  const e_HW_TriggerNature n = e2sm_hw_decode_event_trigger_definition(b);
  free_byte_array(b);
  assert(n == HW_TriggerNature_onchange);

  char *ind_text = "Hello from E2SM-HW, this is an indication";
  byte_array_t ind = { .buf = (uint8_t*) ind_text, .len = strlen(ind_text) };
  b = e2sm_hw_generate_indication_message(ind);
  byte_array_t ind_resp = e2sm_hw_decode_indication_message(b);
  free_byte_array(b);
  assert(ind.len == ind_resp.len && memcmp(ind.buf, ind_resp.buf, ind.len) == 0);
  free_byte_array(ind_resp);

  b = e2sm_hw_generate_indication_header(1333);
  const long h = e2sm_hw_decode_indication_header(b);
  free_byte_array(b);
  assert(h == 1333);

  char *ctrl_text = "Hello from E2SM-HW, this is a control";
  byte_array_t ctrl = { .buf = (uint8_t*) ctrl_text, .len = strlen(ctrl_text) };
  b = e2sm_hw_generate_control_message(ctrl);
  byte_array_t ctrl_resp = e2sm_hw_decode_control_message(b);
  free_byte_array(b);
  assert(ctrl.len == ctrl_resp.len && memcmp(ctrl.buf, ctrl_resp.buf, ctrl.len) == 0);
  free_byte_array(ctrl_resp);

  b = e2sm_hw_generate_control_header(12345);
  const long c = e2sm_hw_decode_control_header(b);
  free_byte_array(b);
  assert(c == 12345);
}
