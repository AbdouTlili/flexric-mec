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

#include "../../../../src/xApp/e42_xapp_api.h"
#include "../../../../src/util/alg_ds/alg/defer.h"
#include "../../../../src/util/time_now_us.h"

#include "../../../../src/sm/slice_sm/slice_sm_id.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

uint16_t assoc_rnti = 0;

static
void sm_cb_slice(sm_ag_if_rd_t const* rd)
{
  assert(rd != NULL);
  assert(rd->type == SLICE_STATS_V0);

  int64_t now = time_now_us();

  printf("SLICE ind_msg latency = %ld \n", now - rd->slice_stats.msg.tstamp);
  if (rd->slice_stats.msg.ue_slice_conf.len_ue_slice > 0)
    assoc_rnti = rd->slice_stats.msg.ue_slice_conf.ues->rnti; // TODO: assign the rnti after get the indication msg
}

static
void fill_add_mod_slice(slice_conf_t* add)
{
  assert(add != NULL);

  uint32_t set_len_slices = 0;
  uint32_t set_slice_id[] = {0, 2, 5};
  char* set_label[] = {"s1", "s2", "s3"};
  /// NVS/EDF slice are only supported by OAI eNB ///
  slice_algorithm_e set_type = SLICE_ALG_SM_V0_STATIC;
  //slice_algorithm_e set_type = SLICE_ALG_SM_V0_NVS;
  //slice_algorithm_e set_type = SLICE_ALG_SM_V0_EDF;
  //slice_algorithm_e set_type = SLICE_ALG_SM_V0_NONE;
  assert(set_type >= 0);
  if (set_type != 0)
    set_len_slices = 3;
  else
    printf("RESET DL SLICE, algo = NONE\n");
  /// SET DL STATIC SLICE PARAMETER ///
  uint32_t set_st_low_high_p[] = {0, 3, 4, 7, 8, 12};
  /// SET DL NVS SLICE PARAMETER///
  nvs_slice_conf_e nvs_conf[] = {SLICE_SM_NVS_V0_RATE, SLICE_SM_NVS_V0_CAPACITY, SLICE_SM_NVS_V0_RATE};
  float mbps_rsvd = 0.2;
  float mbps_ref = 10.0;
  float pct_rsvd = 0.7;
  /// SET DL EDF SLICE PARAMETER///
  int deadline[] = {20, 20, 40};
  int guaranteed_prbs[] = {10, 4, 10};

  /// DL SLICE CONTROL INFO ///
  ul_dl_slice_conf_t* add_dl = &add->dl;
  char const* dlname = "PF";
  add_dl->len_sched_name = strlen(dlname);
  add_dl->sched_name = malloc(strlen(dlname));
  assert(add_dl->sched_name != NULL && "memory exhausted");
  memcpy(add_dl->sched_name, dlname, strlen(dlname));

  add_dl->len_slices = set_len_slices;
  if (add_dl->len_slices > 0) {
    add_dl->slices = calloc(add_dl->len_slices, sizeof(fr_slice_t));
    assert(add_dl->slices != NULL && "memory exhausted");
  }

  for (uint32_t i = 0; i < add_dl->len_slices; ++i) {
    fr_slice_t* s = &add_dl->slices[i];
    s->id = set_slice_id[i];

    const char* label = set_label[i];
    s->len_label = strlen(label);
    s->label = malloc(s->len_label);
    assert(s->label != NULL && "Memory exhausted");
    memcpy(s->label, label, s->len_label );

    const char* sched_str = "PF";
    s->len_sched = strlen(sched_str);
    s->sched = malloc(s->len_sched);
    assert(s->sched != NULL && "Memory exhausted");
    memcpy(s->sched, sched_str, s->len_sched);

    if (set_type == SLICE_ALG_SM_V0_STATIC) {
      s->params.type = SLICE_ALG_SM_V0_STATIC;
      s->params.u.sta.pos_high = set_st_low_high_p[i * 2 + 1];
      s->params.u.sta.pos_low = set_st_low_high_p[i * 2];
      printf("ADD STATIC DL SLICE: id %u, pos_low %u, pos_high %u\n", s->id, s->params.u.sta.pos_low, s->params.u.sta.pos_high);
    } else if (set_type == SLICE_ALG_SM_V0_NVS) {
      s->params.type = SLICE_ALG_SM_V0_NVS;
      if (nvs_conf[i] == SLICE_SM_NVS_V0_RATE) {
        s->params.u.nvs.conf = SLICE_SM_NVS_V0_RATE;
        s->params.u.nvs.u.rate.u1.mbps_required = mbps_rsvd;
        s->params.u.nvs.u.rate.u2.mbps_reference = mbps_ref;
        printf("ADD NVS DL SLICE: id %u, conf %d(rate), mbps_required %f, mbps_reference %f\n", s->id, s->params.u.nvs.conf, s->params.u.nvs.u.rate.u1.mbps_required, s->params.u.nvs.u.rate.u2.mbps_reference);
      } else if (nvs_conf[i] == SLICE_SM_NVS_V0_CAPACITY) {
        s->params.u.nvs.conf = SLICE_SM_NVS_V0_CAPACITY;
        s->params.u.nvs.u.capacity.u.pct_reserved = pct_rsvd;
        printf("ADD NVS DL SLICE: id %u, conf %d(capacity), pct_reserved %f\n", s->id, s->params.u.nvs.conf, s->params.u.nvs.u.capacity.u.pct_reserved);
      } else {
        assert(0 != 0 && "Unkown NVS conf type\n");
      }
    } else if (set_type == SLICE_ALG_SM_V0_EDF) {
      s->params.type = SLICE_ALG_SM_V0_EDF;
      s->params.u.edf.deadline = deadline[i];
      s->params.u.edf.guaranteed_prbs = guaranteed_prbs[i];
    } else {
      assert(0 != 0 && "Unknown type encountered");
    }
  }


  /// UL SLICE CONTROL INFO ///
  ul_dl_slice_conf_t* add_ul = &add->ul;
  char const* ulname = "round_robin_ul";
  add_ul->len_sched_name = strlen(ulname);
  add_ul->sched_name = malloc(strlen(ulname));
  assert(add_ul->sched_name != NULL && "memory exhausted");
  memcpy(add_ul->sched_name, ulname, strlen(ulname));

  add_ul->len_slices = 0;
}

static
void fill_del_slice(del_slice_conf_t* del)
{
  assert(del != NULL);

  /// SET DL ID ///
  uint32_t dl_ids[] = {2};
  del->len_dl = sizeof(dl_ids)/sizeof(dl_ids[0]);
  if (del->len_dl > 0)
    del->dl = calloc(del->len_dl, sizeof(uint32_t));
  for (uint32_t i = 0; i < del->len_dl; i++) {
    del->dl[i] = dl_ids[i];
    printf("DEL DL SLICE: id %u\n", dl_ids[i]);
  }

  /*
  /// SET UL ID ///
  uint32_t ul_ids[] = {0};
  del->len_ul = sizeof(ul_ids)/sizeof(ul_ids[0]);
  if (del->len_ul > 0)
    del->ul = calloc(del->len_ul, sizeof(uint32_t));
  for (uint32_t i = 0; i < del->len_ul; i++)
    del->ul[i] = ul_ids[i];
  */

}

static
void fill_assoc_ue_slice(ue_slice_conf_t* assoc)
{
  assert(assoc != NULL);

  /// SET ASSOC UE NUMBER ///
  assoc->len_ue_slice = 1;
  if(assoc->len_ue_slice > 0){
    assoc->ues = calloc(assoc->len_ue_slice, sizeof(ue_slice_assoc_t));
    assert(assoc->ues);
  }

  for(uint32_t i = 0; i < assoc->len_ue_slice; ++i) {
    /// SET RNTI ///
    assoc->ues[i].rnti = assoc_rnti; // TODO: get rnti from sm_cb_slice()
    assoc->ues[i].rnti = assoc_rnti; // TODO: get rnti from sm_cb_slice()
    /// SET DL ID ///
    assoc->ues[i].dl_id = 5;
    printf("ASSOC DL SLICE: <rnti>, id %u\n", assoc->ues[i].dl_id);
    /*
    /// SET UL ID ///
    assoc->ues[i].ul_id = 0;
    */
  }
}

static
sm_ag_if_wr_t fill_slice_sm_ctrl_req(uint16_t ran_func_id, slice_ctrl_msg_e type)
{
  assert(ran_func_id == 145);

  sm_ag_if_wr_t wr = {0};
  wr.type = SM_AGENT_IF_WRITE_V0_END;
  if (ran_func_id == 145) {
    wr.type = SLICE_CTRL_REQ_V0;
    wr.slice_req_ctrl.hdr.dummy = 0;

    if (type == SLICE_CTRL_SM_V0_ADD) {
      /// ADD MOD ///
      wr.slice_req_ctrl.msg.type = SLICE_CTRL_SM_V0_ADD;
      fill_add_mod_slice(&wr.slice_req_ctrl.msg.u.add_mod_slice);
    } else if (type == SLICE_CTRL_SM_V0_DEL) {
      /// DEL ///
      wr.slice_req_ctrl.msg.type = SLICE_CTRL_SM_V0_DEL;
      fill_del_slice(&wr.slice_req_ctrl.msg.u.del_slice);
    } else if (type == SLICE_CTRL_SM_V0_UE_SLICE_ASSOC) {
      /// ASSOC SLICE ///
      wr.slice_req_ctrl.msg.type = SLICE_CTRL_SM_V0_UE_SLICE_ASSOC;
      fill_assoc_ue_slice(&wr.slice_req_ctrl.msg.u.ue_slice);
    } else {
      assert(0 != 0 && "Unknown slice ctrl type");
    }
  } else {
    assert(0 !=0 && "Unknown RAN function id");
  }
  return wr;
}

int main(int argc, char *argv[])
{
  fr_args_t args = init_fr_args(argc, argv);

  //Init the xApp
  init_xapp_api(&args);
  sleep(1);

  e2_node_arr_t nodes = e2_nodes_xapp_api();
  defer({ free_e2_node_arr(&nodes); });

  assert(nodes.len > 0);
  printf("Connected E2 nodes len = %d\n", nodes.len);

  // SLICE indication
  inter_xapp_e inter_t = ms_5;
  sm_ans_xapp_t slice_handle;

  for(size_t i = 0; i < nodes.len; ++i) {
    e2_node_connected_t *n = &nodes.n[i];
    for (size_t j = 0; j < n->len_rf; ++j)
      printf("Registered ran func id = %d \n ", n->ack_rf[j].id);

    slice_handle = report_sm_xapp_api(&nodes.n[i].id, n->ack_rf[3].id, inter_t, sm_cb_slice);
    assert(slice_handle.success == true);
    sleep(2);

    // Control ADD slice
    sm_ag_if_wr_t ctrl_msg_add = fill_slice_sm_ctrl_req(SM_SLICE_ID, SLICE_CTRL_SM_V0_ADD);
    control_sm_xapp_api(&nodes.n[i].id, SM_SLICE_ID, &ctrl_msg_add);
    free(ctrl_msg_add.slice_req_ctrl.msg.u.add_mod_slice.dl.slices);
    free(ctrl_msg_add.slice_req_ctrl.msg.u.add_mod_slice.dl.sched_name);

    sleep(10);

    // Control DEL slice
    sm_ag_if_wr_t ctrl_msg_del = fill_slice_sm_ctrl_req(SM_SLICE_ID, SLICE_CTRL_SM_V0_DEL);
    control_sm_xapp_api(&nodes.n[i].id, SM_SLICE_ID, &ctrl_msg_del);
    free(ctrl_msg_del.slice_req_ctrl.msg.u.ue_slice.ues);

    sleep(20);

    // Control ASSOC slice
    sm_ag_if_wr_t ctrl_msg_assoc = fill_slice_sm_ctrl_req(SM_SLICE_ID, SLICE_CTRL_SM_V0_UE_SLICE_ASSOC);
    control_sm_xapp_api(&nodes.n[i].id, SM_SLICE_ID, &ctrl_msg_assoc);
    free(ctrl_msg_assoc.slice_req_ctrl.msg.u.ue_slice.ues);

    sleep(20);
  }

  // Remove the handle previously returned
  rm_report_sm_xapp_api(slice_handle.u.handle);

  sleep(1);

  //Stop the xApp
  while(try_stop_xapp_api() == false)
    usleep(1000);

  printf("Test xApp run SUCCESSFULLY\n");
}


