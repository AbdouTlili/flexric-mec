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
const char* cmd = "10_ms";


static inline
bool e2_node_connected()
{
  size_t const sz = e2_nodes();
  return sz != 0;
}

static
void sig_handler(int sig_num)
{
  printf("\nEnding the near-RIC with signal number = %d\n", sig_num);

  if(e2_node_connected() == true ) {
    rm_report_service_near_ric_api(MAC_ran_func_id, cmd);
    rm_report_service_near_ric_api(RLC_ran_func_id, cmd);
    rm_report_service_near_ric_api(PDCP_ran_func_id, cmd);
  } else {
    printf("[NEAR-RIC]: E2 Node never connected \n");
  }

  // Stop the RIC
  stop_near_ric_api();

  exit(0);
}


static inline
void wait_e2_node()
{
  while(1){
   sleep(1);
    if(e2_node_connected() == true)
      break;
  }
}

static
bool valid_ip(const char* ip)
{
  struct sockaddr_in sa;
  int result = inet_pton(AF_INET, ip, &(sa.sin_addr));
  return result != 0;
}


int main(int argc, char *argv[])
{
  const char* default_ip = "127.0.0.1";

  char ip_addr[24] = {0};

  int opt = 0;
  bool cmd_passed = false;
   while((opt = getopt(argc, argv, ":a:")) != -1){ 
      switch(opt){
         case 'a':
            memcpy(ip_addr, optarg, 24);
            cmd_passed = true;
            break;
         case '?': //used for some unknown options
            printf("Unknown command option: %c\n", optopt);
            exit(0);
            break;
          default:
            printf("Unknown command option: %c\n", optopt);
            exit(0);
      }
   }
 
  if(cmd_passed == false)
    memcpy(ip_addr, default_ip, 24);

  if(valid_ip(ip_addr) == false){
    printf("Invalid IP address provided %s\n", ip_addr);
    exit(0);
  }


  printf("RIC IP address: %s\n", ip_addr);

  // Signal handler
  signal(SIGINT, sig_handler);


  // Init the RIC
  init_near_ric_api(ip_addr);

  wait_e2_node();

  report_service_near_ric_api(MAC_ran_func_id, cmd );

  report_service_near_ric_api(RLC_ran_func_id, cmd);

  report_service_near_ric_api(PDCP_ran_func_id, cmd);

  while(1){
    sleep(5);
  }

}

