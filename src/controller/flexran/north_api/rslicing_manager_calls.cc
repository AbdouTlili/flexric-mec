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

#include "rslicing_manager_calls.h"

using namespace flexric::flexran::north_api;

void rslicing_manager_calls::register_calls(Pistache::Rest::Description& desc)
{
  auto rslicing = desc.path("/rslicing");

  //rslicing.route(desc.get("/"), "Get RAN info in JSON")
  //     .bind(&rslicing_manager_calls::get_ran_info, this);
  rslicing.route(desc.post("/slice/enb/:id?"),
                  "Post a new slice configuration")
          .bind(&rslicing_manager_calls::apply_slice_config, this);

  rslicing.route(desc.del("/slice/enb/:id?"),
                  "Delete slices as specified in the JSON")
          .bind(&rslicing_manager_calls::remove_slice_config, this);

  rslicing.route(desc.post("/ue_slice_assoc/enb/:id?"),
                  "Change the slice association of a UE")
          .bind(&rslicing_manager_calls::change_ue_slice_assoc, this);
}

void rslicing_manager_calls::apply_slice_config(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response)
{
  std::string bs;
  if (request.hasParam(":id")) bs = request.param(":id").as<std::string>();
  std::string policy = request.body();
  if (policy.length() == 0) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"empty request body\" }\n", MIME(Application, Json));
    return;
  }

  try {
    _rslicing_manager.apply_slice_config_policy(bs, policy);
  } catch (const std::invalid_argument& e) {
    std::cerr << "encountered error while processing " << __func__
              << "(): " << e.what() << "\n";
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error:\": \"" + std::string(e.what()) + "\"}\n", MIME(Application, Json));
    return;
  }

  response.send(Pistache::Http::Code::Ok, "{ \"status\": \"Ok\" }\n");
}

void rslicing_manager_calls::remove_slice_config(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response)
{
  std::string bs;
  if (request.hasParam(":id")) bs = request.param(":id").as<std::string>();
  std::string policy = request.body();
  if (policy.length() == 0) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"empty request body\" }\n", MIME(Application, Json));
    return;
  }

  try {
    _rslicing_manager.remove_slice(bs, policy);
  } catch (const std::invalid_argument& e) {
    std::cerr << "encountered error while processing " << __func__
              << "(): " << e.what();
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error:\": \"" + std::string(e.what()) + "\"}\n", MIME(Application, Json));
    return;
  }

  response.send(Pistache::Http::Code::Ok, "{ \"status\": \"Ok\" }\n");
}

void rslicing_manager_calls::change_ue_slice_assoc(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response)
{
  std::string bs;
  if (request.hasParam(":id")) bs = request.param(":id").as<std::string>();
  std::string policy = request.body();
  if (policy.length() == 0) {
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error\": \"empty request body\" }\n", MIME(Application, Json));
    return;
  }

  try {
    _rslicing_manager.change_ue_slice_association(bs, policy);
  } catch (const std::invalid_argument& e) {
    std::cerr << "encountered error while processing " << __func__
              << "(): " << e.what();
    response.send(Pistache::Http::Code::Bad_Request,
        "{ \"error:\": \"" + std::string(e.what()) + "\"}\n", MIME(Application, Json));
    return;
  }

  response.send(Pistache::Http::Code::Ok, "{ \"status\": \"Ok\" }\n");
}
