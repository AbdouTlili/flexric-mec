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

#ifndef MAC_INFO_H_
#define MAC_INFO_H_

#include <string>

extern "C" {
#include <type_defs.h>
}

#include "mac_rslicing.h"
#include "mac_stats.h"

namespace flexric {
namespace flexran {
namespace rib {

class mac_info {
public:
  void update_mac_rslicing(const ric_indication_t* ind);
  void reset_mac_rslicing();

  void update_mac_stats(const ric_indication_t* ind);
  void reset_mac_stats();

  std::string to_json_inner() const;
  std::string to_json() const;

private:
  mac_rslicing _mac_rslicing;
  mac_stats _mac_stats;
};

}
}
}

#endif /* MAC_INFO_H_ */
