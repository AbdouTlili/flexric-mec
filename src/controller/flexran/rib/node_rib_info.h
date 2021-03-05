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

#ifndef NODE_RIB_INFO_H_
#define NODE_RIB_INFO_H_

#include <cstdint>

#include "mac_info.h"
#include "pdcp_stats.h"
#include "rlc_stats.h"
#include "rrc_info.h"

extern "C" {
#include "type_defs.h"
}

namespace flexric {
namespace flexran {
namespace rib {

class node_rib_info {
public:
  node_rib_info(uint64_t node_id)
    : _node_id(node_id)
  {}

  void update_mac_stats(const ric_indication_t* ind);
  void reset_mac_stats();

  void update_mac_rslicing(const ric_indication_t* ind);
  void reset_mac_rslicing();

  void update_pdcp_stats(const ric_indication_t* ind);
  void reset_pdcp_stats();

  void update_rlc_stats(const ric_indication_t* ind);
  void reset_rlc_stats();

  void update_rrc_conf(const ric_indication_t* ind);
  void reset_rrc_conf();
  void update_rrc_stats(const ric_indication_t* ind);
  void reset_rrc_stats();

  std::string to_json_inner() const;
  std::string to_json() const;

private:
  const uint64_t _node_id;
  mac_info _mac_info;
  pdcp_stats _pdcp_stats;
  rlc_stats _rlc_stats;
  rrc_info _rrc_info;
};

}
}
}

#endif /* NODE_RIB_INFO_H_ */
