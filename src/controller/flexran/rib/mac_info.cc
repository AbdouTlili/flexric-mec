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

#include "mac_info.h"

namespace flexric {
namespace flexran {
namespace rib {

void mac_info::update_mac_rslicing(const ric_indication_t* ind)
{
  _mac_rslicing.update(ind);
}

void mac_info::reset_mac_rslicing()
{
  _mac_rslicing.reset();
}

void mac_info::update_mac_stats(const ric_indication_t* ind)
{
  _mac_stats.update(ind);
}

void mac_info::reset_mac_stats()
{
  _mac_stats.reset();
}

std::string mac_info::to_json_inner() const
{
  return "\"slicing\":{" + _mac_rslicing.to_json_inner() + "},\"stats\":{" + _mac_stats.to_json_inner() + "}";
}

std::string mac_info::to_json() const
{
  return "{" + to_json_inner() + "}";
}

}
}
}

