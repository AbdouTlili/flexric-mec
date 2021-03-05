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

#ifndef COMPONENT_H_
#define COMPONENT_H_

extern "C" {
#include "e2ap_server.h"
#include "rrc_event_app.h"
}

#include "rib.h"

namespace flexric {
namespace flexran {
namespace app {

class component {
  public:
  component(e2ap_ric_t& ric, e2sm_rrc_event_app_t& ue_events, const rib::Rib& rib, uint16_t ric_req_id)
    : _ric(ric), _ue_events(ue_events), _rib(rib), _ric_req_id(ric_req_id), _ric_inst_id(0)
  {}

  protected:
    e2ap_ric_t& _ric;
    e2sm_rrc_event_app_t& _ue_events;
    const rib::Rib& _rib;
    const uint16_t _ric_req_id;
    uint16_t _ric_inst_id;
};

}
}
}

#endif /* COMPONENT_H_ */
