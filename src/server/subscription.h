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

#ifndef SUBSCRIPTION_H
#define SUBSCRIPTION_H

typedef struct cb_sub_s {
  void (*sub_resp)(const ric_subscription_response_t *p, void *);
  void (*sub_fail)(const ric_subscription_failure_t *p, void *);
  void (*ind)(const ric_indication_t *p, void *);
} cb_sub_t;

typedef struct cb_sub_del_s {
  void (*sub_del_resp)(const ric_subscription_delete_response_t *, void *);
  void (*sub_del_fail)(const ric_subscription_delete_failure_t *, void *);
} cb_sub_del_t;

typedef struct cb_ctrl_s {
  void (*ctrl_ack)(const ric_control_acknowledge_t*, void *);
  void (*ctrl_fail)(const ric_control_failure_t*, void *);
} cb_ctrl_t;

#endif /* SUBSCRIPTION_H */
