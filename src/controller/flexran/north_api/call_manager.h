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

#ifndef CALL_MANAGER_H_
#define CALL_MANAGER_H_

#include <pistache/endpoint.h>
#include <pistache/http.h>
#include <pistache/description.h>

#include "app_calls.h"

namespace flexric {
namespace flexran {
namespace north_api {

class call_manager {

public:

  call_manager(Pistache::Address addr);

  void init(size_t thr = 1);
  void start();
  void shutdown();
  void register_calls(flexran::north_api::app_calls& calls);

private:

  void setup_routes();
  void list_api(const Pistache::Rest::Request& request,
                Pistache::Http::ResponseWriter response);

  std::shared_ptr<Pistache::Http::Endpoint> _httpEndpoint;
  Pistache::Rest::Description _desc;
};

}
}
}

#endif /* CALL_MANAGER_H_ */
