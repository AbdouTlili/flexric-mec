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



#include "../../../src/agent/e2_agent_api.h"
#include "../../../test/sm/common/fill_ind_data.h"
#include "../src/util/conf_file.h"

#include <assert.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <poll.h>
#include <unistd.h>
#include <string.h>
#include <time.h>


static
void read_RAN(sm_ag_if_rd_t* data)
{
  assert(data->type == MAC_STATS_V0 || data->type == RLC_STATS_V0 ||  data->type == PDCP_STATS_V0 || data->type == SLICE_STATS_V0 || data->type == KPM_STATS_V0 || data->type == GTP_STATS_V0);


  if(data->type == MAC_STATS_V0 ){
    fill_mac_ind_data(&data->mac_stats);
  } else if(data->type == RLC_STATS_V0) {
    fill_rlc_ind_data(&data->rlc_stats);
  } else if (data->type == PDCP_STATS_V0 ){
    fill_pdcp_ind_data(&data->pdcp_stats);
  } else if(data->type == SLICE_STATS_V0 ){
    fill_slice_ind_data(&data->slice_stats);
  } else if(data->type == GTP_STATS_V0 ){
    fill_gtp_ind_data(&data->gtp_stats);
  } else if(data->type == KPM_STATS_V0 ){
    fill_kpm_ind_data(&data->kpm_stats);
  } else {
    assert("Invalid data type");
  }
}

static
sm_ag_if_ans_t write_RAN(sm_ag_if_wr_t const* data)
{
  assert(data != NULL);
  if(data->type == MAC_CTRL_REQ_V0){
    //printf("Control message called in the RAN \n");
    sm_ag_if_ans_t ans = {.type = MAC_AGENT_IF_CTRL_ANS_V0};
    ans.mac.ans = MAC_CTRL_OUT_OK;
    return ans;
  } else if(data->type == SLICE_CTRL_REQ_V0 ){

    slice_ctrl_req_data_t const* slice_req_ctrl = &data->slice_req_ctrl;
    slice_ctrl_msg_t const* msg = &slice_req_ctrl->msg;

    if(msg->type == SLICE_CTRL_SM_V0_ADD){
        printf("[E2 Agent]: SLICE CONTROL ADD rx\n");
    } else if (msg->type == SLICE_CTRL_SM_V0_DEL){
        printf("[E2 Agent]: SLICE CONTROL DEL rx\n");
    } else if (msg->type == SLICE_CTRL_SM_V0_UE_SLICE_ASSOC){
        printf("[E2 Agent]: SLICE CONTROL ASSOC rx\n");
    } else {
      assert(0!=0 && "Unknown msg_type!");
    }

    sm_ag_if_ans_t ans = {.type =  SLICE_AGENT_IF_CTRL_ANS_V0};
    return ans;
  } else {
    assert(0 != 0 && "Not supported function ");
  }
  sm_ag_if_ans_t ans = {0};
  return ans;
}

static
void sig_handler(int sig_num)
{
  printf("\nEnding the E2 Agent with signal number = %d\n", sig_num);

  // Stop the E2 Agent
  stop_agent_api();

  exit(0);
}


int main(int argc, char *argv[])
{
  // Signal handler
  signal(SIGINT, sig_handler);

  // Init the Agent

  sm_io_ag_t io = {.read = read_RAN, .write = write_RAN};
  fr_args_t args = init_fr_args(argc, argv);

  srand(time(0));
  int nb_id, mcc, mnc, mnc_digit_len = 0;
  char* nbid_str = get_conf_nbid(&args);
  if (strlen(nbid_str) > 0) nb_id = atoi(nbid_str);
  else nb_id = abs(rand()%2048);

  char* mcc_str = get_conf_mcc(&args);
  if (strlen(mcc_str) > 0) mcc = atoi(mcc_str);
  else mcc = abs(rand()%999);

  char* mnc_str = get_conf_mnc(&args);
  if (strlen(mnc_str) > 0) mnc = atoi(mnc_str);
  else mnc = abs(rand()%999);

  if (mnc%1000 < 100) mnc_digit_len = 2;
  else mnc_digit_len = 3;

  ngran_node_t ran_type = get_conf_rantype(&args);
  if (ran_type != ngran_gNB_DU && ran_type != ngran_gNB_CU && ran_type != ngran_gNB && ran_type != ngran_eNB) {
    printf("not config RAN type, set RAN type as gNB\n");
    ran_type = ngran_gNB;
  }

  printf("[E2 AGENT]: nb_id %d, mcc %d, mnc %d, mnc_digit_len %d, ran_type %d\n", nb_id, mcc, mnc, mnc_digit_len, ran_type);
  init_agent_api(mcc, mnc, mnc_digit_len, nb_id, ran_type, io, &args);

  while(1){
    poll(NULL, 0, 1000);
  }

  return EXIT_SUCCESS;
}
