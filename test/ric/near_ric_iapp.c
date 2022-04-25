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


#include "../src/ric/near_ric_api.h"

#include <arpa/inet.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>


const uint16_t MAC_ran_func_id = 142;
const uint16_t RLC_ran_func_id = 143;
const uint16_t PDCP_ran_func_id = 144;
const uint16_t SLICE_ran_func_id = 145; // Not implemented yet
const char* cmd = "5_ms";

/*
static inline
bool e2_node_connected()
{
  e2_nodes_api_t nodes = e2_nodes_near_ric_api();
 
  bool const node_connected = nodes.len != 0; 

  free_e2_nodes_api(&nodes);

  return node_connected;
}
*/

static
void sig_handler(int sig_num)
{
  printf("\nEnding the near-RIC with signal number = %d\n", sig_num);

//  if(e2_node_connected() == false ) {
//    printf("[NEAR-RIC]: E2 Node never connected \n");
//  }

  // Stop the RIC
  stop_near_ric_api();

  exit(0);
}


int main()
{
  // Signal handler
  signal(SIGINT, sig_handler);

  // Init the RIC
  init_near_ric_api();

//  wait_e2_node();

  while(1){
    sleep(42);
  }
  return EXIT_SUCCESS;
}

