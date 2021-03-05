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

#ifndef RRC_UE_STATS_H_
#define RRC_UE_STATS_H_

#include "ind_info.h"

namespace flexric {
namespace flexran {
namespace rib {

class rrc_ue_stats : public ind_info {
public:
  rrc_ue_stats(const ric_indication_t* ind);
  // TODO: overwrite update() to handle multiple MeasReport with different IDs
  std::string to_json_inner() const;
};

}
}
}

#endif /* RRC_UE_STATS_H_ */
