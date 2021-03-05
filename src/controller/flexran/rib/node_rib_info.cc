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

#include <cassert>
#include "node_rib_info.h"

namespace flexric {
namespace flexran {
namespace rib {

void node_rib_info::update_mac_stats(const ric_indication_t* ind)
{
  _mac_info.update_mac_stats(ind);
}

void node_rib_info::reset_mac_stats()
{
  _mac_info.reset_mac_stats();
}

void node_rib_info::update_mac_rslicing(const ric_indication_t* ind)
{
  _mac_info.update_mac_rslicing(ind);
}

void node_rib_info::reset_mac_rslicing()
{
  _mac_info.reset_mac_rslicing();
}

void node_rib_info::update_pdcp_stats(const ric_indication_t* ind)
{
  _pdcp_stats.update(ind);
}

void node_rib_info::reset_pdcp_stats()
{
  _pdcp_stats.reset();
}

void node_rib_info::update_rlc_stats(const ric_indication_t* ind)
{
  _rlc_stats.update(ind);
}

void node_rib_info::reset_rlc_stats()
{
  _rlc_stats.reset();
}

void node_rib_info::update_rrc_conf(const ric_indication_t* ind)
{
  _rrc_info.update_rrc_conf(ind);
}

void node_rib_info::reset_rrc_conf()
{
  _rrc_info.reset_rrc_conf();
}

void node_rib_info::update_rrc_stats(const ric_indication_t* ind)
{
  _rrc_info.update_rrc_stats(ind);
}

void node_rib_info::reset_rrc_stats()
{
  _rrc_info.reset_rrc_stats();
}

std::string node_rib_info::to_json_inner() const
{
  std::string json;

  json += "\"mac\":{" + _mac_info.to_json_inner();
  json += "},\"rlc\":{" + _rlc_stats.to_json_inner();
  json += "},\"pdcp\":{" + _pdcp_stats.to_json_inner();
  json += "},\"rrc\":{" + _rrc_info.to_json_inner();

  return json + "}";
}

std::string node_rib_info::to_json() const
{
  return "{" + to_json_inner() + "}";
}

}
}
}
