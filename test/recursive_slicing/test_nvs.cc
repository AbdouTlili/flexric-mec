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
#include "nvs.h"

using namespace flexric::recursive_slicing;

void test_get_resources()
{
  nvs rate(1, 2, 2);
  assert(rate.get_resources() == 1);
  nvs cap(1, 0.5);
  assert(cap.get_resources() == 0.5);
}

void test_ineq()
{
  // for ordering by ID
  nvs rate (1, 2, 2);
  nvs cap(2, 0.5);
  assert(rate < cap);
}

void test_mul()
{
  nvs cap(2, 1);
  cap *= 0.5;
  assert(cap.get_resources() == 0.5);
  nvs rate(1, 2, 2);
  rate *= 0.5;
  assert(rate.get_resources() == 0.5);
}

void test_div()
{
  nvs cap(2, 0.5);
  cap /= 0.5;
  assert(cap.get_resources() == 1);
  nvs rate(1, 2, 4);
  rate /= 0.5;
  assert(rate.get_resources() == 1);
}

int main()
{
  test_get_resources();
  test_ineq();
  test_mul();
  test_div();
}
