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
#include <cmath>
#include "nvs.h"

namespace flexric {
namespace recursive_slicing {

static
bool areEqual(float a, float b)
{
  /* a, b in [0,1] */
  return fabs(a - b) <= 0.00005;
}

rate::rate(float mbpsRequired, float mbpsReference)
  : mbpsRequired(mbpsRequired), mbpsReference(mbpsReference)
{
  assert(mbpsRequired <= mbpsReference);
}

bool rate::operator==(const rate& other)
{
  return areEqual(mbpsReference, other.mbpsReference) && areEqual(mbpsRequired, other.mbpsRequired);
}

cap::cap(float pctReserved)
  : pctReserved(pctReserved)
{
  assert(pctReserved <= 1.0f);
}

bool cap::operator==(const cap& other)
{
  return areEqual(pctReserved, other.pctReserved);
}

nvs::nvs(const struct nvs& nvs)
  : id(nvs.id), type(nvs.type)
{
  switch (nvs.type) {
    case nvs_type::rate:
      rate = nvs.rate;
      break;
    case nvs_type::capacity:
      cap = nvs.cap;
      break;
    default:
      assert(0 && "illegal slice type");
      break;
  }
}

bool nvs::operator<(const struct nvs& other)
{
  return id < other.id;
}

nvs& nvs::operator*=(float rhs)
{
  assert(rhs <= 1.0f);
  switch (type) {
    case nvs_type::rate:
      rate.mbpsReference /= rhs;
      break;
    case nvs_type::capacity:
      cap.pctReserved *= rhs;
      break;
    default:
      assert(0 && "illegal slice type");
      break;
  }
  return *this;
}

nvs nvs::operator*(float f) const
{
  nvs copy(*this);
  copy *= f;
  return copy;
}

nvs& nvs::operator/=(float rhs)
{
  assert(rhs <= 1.0f);
  switch (type) {
    case nvs_type::rate:
      rate.mbpsReference *= rhs;
      break;
    case nvs_type::capacity:
      cap.pctReserved /= rhs;
      break;
    default:
      assert(0 && "illegal slice type");
      break;
  }
  return *this;
}

nvs nvs::operator/(float f) const
{
  nvs copy(*this);
  copy /= f;
  return copy;
}

/*
nvs nvs::operator*(float f)
{
  assert(f <= 1.0f);
  switch (type) {
    case nvs_type::rate:
      return nvs(id, rate.mbpsRequired, rate.mbpsReference / f);
      break;
    case nvs_type::capacity:
      return nvs(id, cap.pctReserved * f);
    default:
      assert(0 && "illegal slice type");
      break;
  }
  return *this;
}
*/

std::ostream& operator<<(std::ostream& stream, const nvs& nvs)
{
  stream << "ID " << nvs.id;
  switch (nvs.type) {
    case nvs_type::rate:
      std::cout << " rate req " << nvs.rate.mbpsRequired << " ref " << nvs.rate.mbpsReference;
      break;
    case nvs_type::capacity:
      std::cout << " capacity " << nvs.cap.pctReserved;
      break;
    default:
      assert(0 && "illegal slice type");
      break;
  }
  return stream;
}

/*
nvs nvs::operator/(float f)
{
  assert(f <= 1.0f);
  switch (type) {
    case nvs_type::rate:
      return nvs(id, rate.mbpsRequired, rate.mbpsReference * f);
      break;
    case nvs_type::capacity:
      return nvs(id, cap.pctReserved / f);
      break;
    default:
      assert(0 && "illegal slice type");
      break;
  }
  return *this;
}
*/

float nvs::get_resources() const
{
  switch (type) {
    case nvs_type::rate:
      return rate.mbpsRequired / rate.mbpsReference;
    case nvs_type::capacity:
      return cap.pctReserved;
    default:
      assert(0 && "illegal slice type");
      break;
  }
  return 0;
}

}
}
