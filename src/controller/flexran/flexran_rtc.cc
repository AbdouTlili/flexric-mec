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

#include <memory>
#include <csignal>
#include <iostream>

#include <boost/program_options.hpp>

extern "C" {
#include "e2ap_server.h"
#include "rrc_event_app.h"
}

#include "stats_manager.h"
#include "rslicing_manager.h"
using namespace flexric::flexran;

#ifdef REST_NORTHBOUND
#include <pistache/endpoint.h>
#include "call_manager.h"
#include "stats_manager_calls.h"
#include "rslicing_manager_calls.h"
#endif

namespace po = boost::program_options;

int main(int argc, char* argv[])
{
  int sport = 36421;
  std::string saddress = "127.0.0.1";
#ifdef REST_NORTHBOUND
  int nport = 9999;
  std::string naddress = "127.0.0.1";
#endif

  po::options_description desc("Help");
  desc.add_options()
    ("help,h", "Prints this help message")
#ifdef REST_NORTHBOUND
    ("nport,n", po::value<int>()->default_value(nport),
     "Port for northbound API calls")
    ("naddress,s", po::value<std::string>()->default_value(naddress),
     "Address to bind for northbound API calls")
#endif
    ("sport,p", po::value<int>()->default_value(sport),
     "Port for incoming E2 node connections")
    ("saddress,a", po::value<std::string>()->default_value(saddress),
     "Address to bind for incoming E2 node connections");

  po::variables_map opts;
  po::store(po::parse_command_line(argc, argv, desc), opts);

  if ( opts.count("help")  ) {
    std::cout << "FlexRIC: FlexRAN-like real-time controller\n" << desc << "\n";
    return 0;
  }

  try {
    po::notify(opts);
  } catch (po::error& e) {
    std::cerr << "Error: Unrecognized parameter\n";
    return 1;
  }

  sport = opts["sport"].as<int>();
  saddress = opts["saddress"].as<std::string>();
#ifdef REST_NORTHBOUND
  nport = opts["nport"].as<int>();
  naddress = opts["naddress"].as<std::string>();
#endif

  e2ap_ric_t ric = {};
  e2ap_init_server(&ric, saddress.c_str(), sport);

  auto rib = rib::Rib(ric);

  uint16_t ric_req_id = 0;
  e2sm_rrc_event_app_t ue_event_app;
  memset(&ue_event_app, 0, sizeof(ue_event_app));
  e2sm_rrc_event_app_init(&ue_event_app, &ric, ric_req_id++);
  auto stats_manager = app::stats_manager(ric, ue_event_app, rib, ric_req_id++);
  auto rslicing_manager = app::rslicing_manager(ric, ue_event_app, rib, ric_req_id++);

#ifdef REST_NORTHBOUND
  std::cout <<  "Listening on " << naddress << ":" << nport << " for incoming REST connections\n";
  Pistache::Address addr(naddress, nport);
  north_api::call_manager north_api(addr);

  north_api::stats_manager_calls stats_manager_calls(stats_manager);
  north_api.register_calls(stats_manager_calls);
  north_api::rslicing_manager_calls rslicing_manager_calls(rslicing_manager);
  north_api.register_calls(rslicing_manager_calls);

  north_api.init(1);
  north_api.start();
#endif

  e2ap_start_server(&ric); // blocking
  e2ap_free_server(&ric);
  return 0;
}
