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

#include <pistache/http.h>
#include <pistache/http_header.h>

#include "stats_manager_calls.h"

using namespace flexric::flexran::north_api;

void stats_manager_calls::register_calls(Pistache::Rest::Description& desc)
{
  auto stats = desc.path("/stats");

  stats.route(desc.get("/"), "Get RAN info in JSON")
       .bind(&stats_manager_calls::get_ran_info, this);
}

void stats_manager_calls::get_ran_info(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response)
{
  (void) request;
  std::string resp = _stats_manager.get_ran_as_json();
  response.headers().add<Pistache::Http::Header::AccessControlAllowOrigin>("*");
  response.send(Pistache::Http::Code::Ok, resp, MIME(Application, Json));
}
