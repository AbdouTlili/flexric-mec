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

#include "e2ap_server.h"
#include "e2sm_hw_fb_app.h"

int main()
{
  e2ap_ric_t ric;
  memset(&ric, 0, sizeof(ric));
  e2ap_init_server(&ric, "127.0.0.1", 36421);

  e2sm_hw_fb_app_t hw_app;
  memset(&hw_app, 0, sizeof(hw_app));
  e2sm_hw_fb_app_init(&hw_app, &ric, 1000);

  e2ap_start_server(&ric);

  e2sm_hw_fb_app_free(&hw_app);
  e2ap_free_server(&ric);
  return 0;
}
