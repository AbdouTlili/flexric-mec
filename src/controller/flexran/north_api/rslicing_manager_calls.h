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

#ifndef RSLICING_MANAGER_CALLS_H_
#define RSLICING_MANAGER_CALLS_H_

#include <pistache/http.h>
#include <pistache/description.h>

#include "app_calls.h"
#include "rslicing_manager.h"

namespace flexric {
namespace flexran {
namespace north_api {

class rslicing_manager_calls : public app_calls {
public:

  rslicing_manager_calls(app::rslicing_manager& rslicing)
    : _rslicing_manager(rslicing)
  { }

  void register_calls(Pistache::Rest::Description& desc);

  void apply_slice_config(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
  void remove_slice_config(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
  void change_ue_slice_assoc(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);

private:
  app::rslicing_manager& _rslicing_manager;
};

}
}
}

#endif /* RSLICING_MANAGER_CALLS_H_ */
