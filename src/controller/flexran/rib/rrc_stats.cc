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

#include "rrc_stats.h"

extern "C" {
#include "rrc_stats_msg_dec.h"
}

namespace flexric {
namespace flexran {
namespace rib {

void rrc_stats::update(const ric_indication_t* ind)
{
  const uint16_t rnti = rrc_stats_decode_indication_header(ind->hdr);
  std::lock_guard<std::mutex> guard(_lock);
  const auto it = _rrc_stats.find(rnti);
  if (it == _rrc_stats.cend()) {
    _rrc_stats.emplace(rnti, ind);
  } else {
    it->second.update(ind);
  }
}

void rrc_stats::reset()
{
  std::lock_guard<std::mutex> guard(_lock);
  _rrc_stats.clear();
}

void rrc_stats::remove_ue(uint16_t rnti)
{
  std::lock_guard<std::mutex> guard(_lock);
  const auto it = _rrc_stats.find(rnti);
  if (it == _rrc_stats.cend())
    return;
  _rrc_stats.erase(it);
}

std::string rrc_stats::to_json_inner() const
{
  std::lock_guard<std::mutex> guard(_lock);
  std::string json = "\"ueStats\":[";
  for (auto it = _rrc_stats.begin(); it != _rrc_stats.end(); ++it) {
    json += it->second.to_json();
    if (it != std::prev(_rrc_stats.end()))
      json += ",";
  }
  return json + "]";
}

std::string rrc_stats::to_json() const
{
  return "{" + to_json_inner() + "}";
}

}
}
}
