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

#include <rib.h>
#include <cassert>

namespace flexric {
namespace flexran {
namespace rib {

Rib::Rib(e2ap_ric_t& ric)
  : _ric(ric)
{
  ed_t ev1 = subscribe_e2node_connect(&ric.events,
      [](const e2node_event_t* event, void* object) {
        static_cast<Rib*>(object)->node_connect(event);
      }, this);
  (void) ev1;
}

std::shared_ptr<node_rib_info> Rib::get_node(uint64_t key) const
{
  auto it = _node_info.find(key);
  assert(it != _node_info.end());
  return it->second;
}

void Rib::node_connect(const e2node_event_t* event)
{
  const uint64_t key = event->ran->generated_ran_key;
  if (_node_info.find(key) != _node_info.end())
    return;

  _node_info.emplace(std::make_pair(
        key,
        std::make_shared<node_rib_info>(key)));
}

void Rib::node_disconnect(const e2node_event_t* event)
{
  /* TODO: verify is last or remove when not complete base station (i.e., when
   * first of BS disconnects) */
  _node_info.erase(event->ran->generated_ran_key);
}

}
}
}
