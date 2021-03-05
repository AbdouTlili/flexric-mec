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

#include "call_manager.h"
#include <pistache/router.h>
#include <iostream>

using namespace flexric::flexran::north_api;

call_manager::call_manager(Pistache::Address addr)
	: _httpEndpoint(std::make_shared<Pistache::Http::Endpoint>(addr)),
    _desc("FlexRIC-FlexRAN NB API", "0.1")
{
  _desc.info()
       .license("Apache", "http://www.apache.org/licenses/LICENSE-2.0");

  _desc.schemes(Pistache::Rest::Scheme::Http)
       .basePath("/")
       .produces(MIME(Application, Json))
       .consumes(MIME(Application, Json));

  _desc.route(_desc.get("/capabilities"), "list active REST endpoints")
       .bind(&call_manager::list_api, this);
}


void call_manager::init(size_t thr) {
  auto opts = Pistache::Http::Endpoint::options().threads(thr)
      .flags(Pistache::Tcp::Options::ReuseAddr);
  _httpEndpoint->init(opts);
}

void call_manager::start()
{
  Pistache::Rest::Router router;
  router.initFromDescription(_desc);
  _httpEndpoint->setHandler(router.handler());
  _httpEndpoint->serveThreaded();
}

void call_manager::shutdown() {
  _httpEndpoint->shutdown();
}

void call_manager::register_calls(app_calls& calls)
{
  calls.register_calls(_desc);
}

void call_manager::list_api(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response)
{
  (void) request;
  /* creates list of API endpoints like so:
   * {
   *   "info": {
   *     "title": "Text",
   *     "version": "Text"
   *   },
   *   "paths": {
   *     "/endpointA": {
   *       "GET": {
   *         "description": "Text"
   *       },
   *       "POST": {
   *         ...
   *       }
   *     },
   *     ...
   *   }
   * }
   * Could be extended for Swagger documentation as described in
   * pistache/examples/rest_description.cc with real JSON serializer
   */
  std::string json = "{\"info\":{\"title\":\"";
  json += _desc.rawInfo().title;
  json += "\",\"version\":\"";
  json += _desc.rawInfo().version;
  json += "\"},\"paths\":{";
  auto paths = _desc.rawPaths();
  for (auto it = paths.flatBegin(), end = paths.flatEnd(); it != end; ++it) {
    const auto& ppaths = *it;
    if (it != paths.flatBegin()) json += ",";
    json += "\"" + ppaths.begin()->value + "\":{";
    for (auto p = ppaths.begin(), pend = ppaths.end(); p != pend; ++p) {
      if (p != ppaths.begin()) json += ",";
      json += "\"";
      json += Pistache::Http::methodString(p->method);
      json += "\":";
      json += "{\"description\":\"" + p->description + "\"}";
    }
    json += "}";
  }
  json += "}}\n";
  response.send(Pistache::Http::Code::Ok, json, MIME(Application, Json));
}
