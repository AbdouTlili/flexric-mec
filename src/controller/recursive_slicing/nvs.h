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

#ifndef NVS_H
#define NVS_H

#include <cstdint>
#include <iostream>

namespace flexric {
namespace recursive_slicing {

enum class nvs_type {
  rate,
  capacity
};

typedef struct rate {
  rate(float mbpsRequired, float mbpsReference);
  bool operator==(const rate& other);
  float mbpsRequired;
  float mbpsReference;
} rate_t;

typedef struct cap {
  cap(float pctReserved);
  bool operator==(const cap& other);
  float pctReserved;
} cap_t;

struct nvs {
  nvs(uint32_t id, float mbpsRequired, float mbpsReference)
    : id(id), type(nvs_type::rate), rate(mbpsRequired, mbpsReference)
  {}
  nvs(uint32_t id, float pctReserved)
    : id(id), type(nvs_type::capacity), cap(pctReserved)
  {}
  nvs(const struct nvs& nvs);
  bool operator<(const struct nvs& other);
  nvs& operator*=(float rhs);
  nvs operator*(float f) const;
  nvs& operator/=(float rhs);
  nvs operator/(float f) const;
  friend std::ostream& operator<< (std::ostream& stream, const nvs& nvs);

  float get_resources() const;

  uint32_t id;
  nvs_type type;
  union {
    rate_t rate;
    cap_t cap;
  };
  std::string label;
  std::string scheduler;
};



}
}

#endif /* NVS_H */
