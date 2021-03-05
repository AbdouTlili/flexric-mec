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
#include <iostream>
#include "ind_info.h"

namespace flexric {
namespace flexran {
namespace rib {

void ind_info::update(const ric_indication_t* ind)
{
  std::lock_guard<std::mutex> guard(_lock);
  _msg.assign(ind->msg.buf, ind->msg.buf + ind->msg.len);
  _hdr.assign(ind->hdr.buf, ind->hdr.buf + ind->hdr.len);
  if (ind->sn)
    _sn = *ind->sn;
}

void ind_info::reset()
{
  std::lock_guard<std::mutex> guard(_lock);
  _msg.clear();
  _hdr.clear();
  _sn = 0;
}

std::string ind_info::to_json() const
{
  return "{" + to_json_inner() + "}";
}

}
}
}
