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

#ifndef LAYER_INFO_H_
#define LAYER_INFO_H_

#include <cstdint>
#include <vector>
#include <mutex>

extern "C" {
#include <type_defs.h>
}

namespace flexric {
namespace flexran {
namespace rib {

class ind_info {
public:
  void update(const ric_indication_t* ind);
  void reset();
  virtual std::string to_json_inner() const = 0;
  std::string to_json() const;
protected:
  std::vector<uint8_t> _msg;
  std::vector<uint8_t> _hdr;
  uint16_t _sn;
  mutable std::mutex _lock;
};

}
}
}

#endif /* LAYER_INFO_H_ */
