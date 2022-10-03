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
#include "../src/agent/e2_agent_api.h"

#include "../sm/common/fill_ind_data.h"

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


static
void read_RAN(sm_ag_if_rd_t* data)
{
  assert(data->type == MAC_STATS_V0 || data->type == RLC_STATS_V0 ||  data->type == PDCP_STATS_V0 || data->type == SLICE_STATS_V0);

  if(data->type == MAC_STATS_V0 ){
      fill_mac_ind_data(&data->mac_stats);
  } else if(data->type == RLC_STATS_V0) {
      fill_rlc_ind_data(&data->rlc_stats);
  } else if (data->type == PDCP_STATS_V0 ){
      fill_pdcp_ind_data(&data->pdcp_stats);
  } else if(data->type == SLICE_STATS_V0 ){
    fill_slice_ind_data(&data->slice_stats);
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
  } else {
    assert(0 != 0 && "Not supported function ");
  }
  sm_ag_if_ans_t ans = {0};
  return ans;
}

int main(int argc, char *argv[])
{
  // Init the Agent
  const int mcc = 208; 
  const int mnc = 92; 
  const int mnc_digit_len = 2;
  const int nb_id = 42;
  const int cu_du_id = 0;
  ngran_node_t ran_type = ngran_gNB;
  sm_io_ag_t io = {.read = read_RAN, .write = write_RAN};
  fr_args_t args = init_fr_args(argc, argv);
  // Parse arguments
  init_agent_api( mcc, mnc, mnc_digit_len, nb_id, cu_du_id, ran_type, io, &args);
  sleep(1);

  // Init the RIC
  init_near_ric_api(&args);
  sleep(3);

  e2_nodes_api_t e2_nodes = e2_nodes_near_ric_api();
  assert(e2_nodes.len > 0 && "No E2 Nodes connected");

  global_e2_node_id_t const* id = &e2_nodes.n[0].id;

  const uint16_t MAC_ran_func_id = 142;
  const char* cmd = "5_ms";
  report_service_near_ric_api(id, MAC_ran_func_id, cmd );
  sleep(2);

//  const char* cmd2 = "Hello";
//  control_service_near_ric_api(id, MAC_ran_func_id, cmd2 );  
//  sleep(2);

//  load_sm_near_ric_api("../test/so/librlc_sm.so");

  const uint16_t RLC_ran_func_id = 143;
  report_service_near_ric_api(id,RLC_ran_func_id, cmd);
  sleep(2);

  const uint16_t PDCP_ran_func_id = 144;
  report_service_near_ric_api(id,PDCP_ran_func_id, cmd);
  sleep(2);

  const uint16_t SLICE_ran_func_id = 145;
  report_service_near_ric_api(id,SLICE_ran_func_id, cmd);
  sleep(2);

  rm_report_service_near_ric_api(id ,MAC_ran_func_id, cmd);
  rm_report_service_near_ric_api(id ,RLC_ran_func_id, cmd);
  rm_report_service_near_ric_api(id ,PDCP_ran_func_id, cmd);
  rm_report_service_near_ric_api(id ,SLICE_ran_func_id, cmd);

  sleep(1);

  // Stop the Agent
  stop_agent_api();

  // Stop the RIC
  stop_near_ric_api();

  printf("Test communicating E2-Agent and Near-RIC run SUCCESSFULLY\n");

}
