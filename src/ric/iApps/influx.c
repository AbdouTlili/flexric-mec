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



#include "influx.h"

#include <assert.h>                                      // for assert
#include <netinet/in.h>                                  // for sockaddr_in
#include <pthread.h>                                     // for pthread_once
#include <stdint.h>                                      // for uint32_t
#include <stdlib.h>                                      // for exit, EXIT_F...
#include <string.h>                                      // for strlen, memset
#include <stdio.h>
#include <sys/socket.h>                                  // for sendto, MSG_...
#include "ric/iApps/../../sm/mac_sm/ie/mac_data_ie.h"    // for mac_ind_msg_t
#include "ric/iApps/../../sm/pdcp_sm/ie/pdcp_data_ie.h"  // for pdcp_ind_msg_t
#include "ric/iApps/../../sm/rlc_sm/ie/rlc_data_ie.h"    // for rlc_ind_msg_t
#include "string_parser.h"                               // for to_string_ma...

static
pthread_once_t init_socket = PTHREAD_ONCE_INIT;

static
int sockfd = 0;

static
struct sockaddr_in servaddr;

static
int const PORT = 8094;


void init_udp_socket()
{
  if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
    assert(0!=0 && "Error creating the socket");
    exit(EXIT_FAILURE);
  }
  memset(&servaddr, 0, sizeof(servaddr));

  // Filling server information
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(PORT);
  servaddr.sin_addr.s_addr = INADDR_ANY;

}

void notify_influx_listener(sm_ag_if_rd_t const* data)
{
  assert(data != NULL);
  assert(data->type == MAC_STATS_V0 || data->type == RLC_STATS_V0 || data->type == PDCP_STATS_V0 || data->type == SLICE_STATS_V0);
  pthread_once(&init_socket, init_udp_socket);

//  printf("Influx db data called!!!\n");
  if(data->type == MAC_STATS_V0){
    mac_ind_msg_t const* ind =  &data->mac_stats.msg;

    for(uint32_t i = 0; i < ind->len_ue_stats; ++i){
      char stats[1024] = {0};
      to_string_mac_ue_stats(&ind->ue_stats[i], ind->tstamp, stats, 1024);
      int const rc = sendto(sockfd, stats, strlen(stats),  MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));
      assert(rc != -1);
    }
  } else if (data->type == RLC_STATS_V0){
    rlc_ind_msg_t const* rlc = &data->rlc_stats.msg;

    for(uint32_t i = 0; i < rlc->len; ++i){

      char stats[1024] = {0};
      to_string_rlc_rb(&rlc->rb[i], rlc->tstamp, stats, 1024);

     int const rc = sendto(sockfd, stats, strlen(stats),  MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));
     assert(rc != -1);
    }

  } else if (data->type == PDCP_STATS_V0){
    pdcp_ind_msg_t const* pdcp = &data->pdcp_stats.msg;

    for(uint32_t i = 0; i < pdcp->len; ++i){
      char stats[512] = {0};
      pdcp_radio_bearer_stats_t* rb = &pdcp->rb[i];

      to_string_pdcp_rb(rb, pdcp->tstamp, stats, 512);
      int const rc = sendto(sockfd, stats, strlen(stats),  MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));
      assert(rc != -1);
    } 
  } else if(data->type == SLICE_STATS_V0){
    slice_ind_msg_t const* slice = &data->slice_stats.msg;

    char stats[512] = {0};

    to_string_slice(slice, slice->tstamp, stats, 512);
    int const rc = sendto(sockfd, stats, strlen(stats),  MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));
    assert(rc != -1);
  } else {
    assert(0 != 0 || "invalid data type ");
  }

}

