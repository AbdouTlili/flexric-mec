//#include "FlexRIC.h"
#include "swig_wrapper.h"

#ifdef XAPP_LANG_PYTHON
#include "Python.h"
#endif

#include "../e42_xapp_api.h"

#include "../../sm/mac_sm/mac_sm_id.h"
#include "../../sm/rlc_sm/rlc_sm_id.h"
#include "../../sm/pdcp_sm/pdcp_sm_id.h"

#include "../../sm/slice_sm/slice_sm_id.h"

#include "../../util/conf_file.h"


#include <arpa/inet.h>
#include <cassert>
#include <ctime>
#include <cstdlib>
#include <pthread.h>
#include <unistd.h>
#include <iostream>

/*
static
int64_t time_now_us(void)
{
  struct timespec tms;

  if (clock_gettime(CLOCK_REALTIME, &tms)) {
    return -1;
  }
  // seconds, multiplied with 1 million 
  int64_t micros = tms.tv_sec * 1000000;

  // Add full microseconds 
  micros += tms.tv_nsec/1000;

  // Round up if necessary 
  if (tms.tv_nsec % 1000 >= 500) {
    ++micros;
  }
  return micros;
}
*/

/*
static
bool valid_ipv4(const char* addr)
{
  assert(addr != NULL);

  struct sockaddr_in sa;
  int result = inet_pton(AF_INET, addr, &(sa.sin_addr));
  return result == 1;
}
*/

static
bool initialized = false;

void init()
{
  assert(initialized == false && "Already initialized!");

  int const argc = 1;
  char** argv = NULL;
  fr_args_t args = init_fr_args(argc, argv);

  initialized = true; 

  init_xapp_api(&args);
}

bool try_stop()
{
  return try_stop_xapp_api();
}

std::vector<E2Node> conn_e2_nodes(void)
{
  e2_node_arr_t arr = e2_nodes_xapp_api();
   

  std::vector<E2Node> x; //(arr.len);

  for(int i = 0; i < arr.len; ++i){

    E2Node tmp;

    e2_node_connected_t const* src = &arr.n[i];
    tmp.id = cp_global_e2_node_id(&src->id); 

    std::vector<ran_function_t> ran_func;//(src->len_rf);

    for(size_t j = 0; j < src->len_rf; ++j){
      ran_function_t rf = cp_ran_function(&src->ack_rf[j]);
      ran_func.push_back(rf);// [j] = rf;
    }
    tmp.ran_func = ran_func;
    x.push_back(tmp);//[i] = tmp;
  }

  free_e2_node_arr(&arr);

  return x;
}

static 
mac_cb* hndlr_mac_cb; 

static
sm_ans_xapp_t hndlr_mac_ans;

static
void sm_cb_mac(sm_ag_if_rd_t const* rd)
{
  assert(rd != NULL);
  assert(rd->type == MAC_STATS_V0);
  assert(hndlr_mac_cb != NULL);

  mac_ind_data_t const* data = &rd->mac_stats; 

  swig_mac_ind_msg_t ind;
  ind.tstamp = data->msg.tstamp;

  for(uint32_t i = 0; i < data->msg.len_ue_stats; ++i){
      mac_ue_stats_impl_t tmp = cp_mac_ue_stats_impl(&data->msg.ue_stats[i]) ;
      ind.ue_stats.emplace_back(tmp);
  }

#ifdef XAPP_LANG_PYTHON
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();
#endif

    hndlr_mac_cb->handle(&ind);

#ifdef XAPP_LANG_PYTHON
    PyGILState_Release(gstate);
#endif

}

void report_mac_sm(global_e2_node_id_t* id, Interval inter_arg, mac_cb* handler)
{
  assert(id != NULL);
  assert(handler != NULL);

  hndlr_mac_cb = handler;

  inter_xapp_e i;
  if(inter_arg == Interval::ms_1 ){
    i = ms_1;
  } else if (inter_arg == Interval::ms_2) {
    i = ms_2;
  } else if(inter_arg == Interval::ms_5) {
    i = ms_5;
  } else if(inter_arg == Interval::ms_10) {
    i = ms_10;
  } else {
    assert(0 != 0 && "Unknown type");
  }

  sm_ans_xapp_t ans = report_sm_xapp_api(id , SM_MAC_ID, i, sm_cb_mac);
  assert(ans.success == true); 
  hndlr_mac_ans = ans;
}


void rm_report_mac_sm(void)
{

#ifdef XAPP_LANG_PYTHON
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();
#endif

  assert(hndlr_mac_ans.u.handle != 0);
  rm_report_sm_xapp_api(hndlr_mac_ans.u.handle);

#ifdef XAPP_LANG_PYTHON
    PyGILState_Release(gstate);
#endif

}


//////////////////////////////////////
// RLC SM   
/////////////////////////////////////

//static
//pthread_t t_rlc;

static 
rlc_cb* hndlr_rlc_cb; 

static
sm_ans_xapp_t hndlr_rlc_ans;

static
void sm_cb_rlc(sm_ag_if_rd_t const* rd)
{
  assert(rd != NULL);
  assert(rd->type == RLC_STATS_V0);
  assert(hndlr_rlc_cb != NULL);

  rlc_ind_data_t const* data = &rd->rlc_stats; 

  swig_rlc_ind_msg_t ind;
  ind.tstamp = data->msg.tstamp;

  for(uint32_t i = 0; i < data->msg.len; ++i){
    rlc_radio_bearer_stats_t tmp = data->msg.rb[i];
    ind.rb_stats.push_back(tmp);
  }

#ifdef XAPP_LANG_PYTHON
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();
#endif

    hndlr_rlc_cb->handle(&ind);

#ifdef XAPP_LANG_PYTHON
    PyGILState_Release(gstate);
#endif

}

void report_rlc_sm(global_e2_node_id_t* id, Interval inter_arg, rlc_cb* handler)
{

  assert(id != NULL);
  assert(handler != NULL);

  hndlr_rlc_cb = handler;

  inter_xapp_e i;
  if(inter_arg == Interval::ms_1 ){
    i = ms_1;
  } else if (inter_arg == Interval::ms_2) {
    i = ms_2;
  } else if(inter_arg == Interval::ms_5) {
    i = ms_5;
  } else if(inter_arg == Interval::ms_10) {
    i = ms_10;
  } else {
    assert(0 != 0 && "Unknown type");
  }

  sm_ans_xapp_t ans = report_sm_xapp_api(id , SM_RLC_ID, i, sm_cb_rlc);
  assert(ans.success == true); 
  hndlr_rlc_ans = ans;

}

void rm_report_rlc_sm(void)
{

#ifdef XAPP_LANG_PYTHON
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();
#endif

  assert(hndlr_rlc_ans.u.handle != 0);
  rm_report_sm_xapp_api(hndlr_rlc_ans.u.handle);

#ifdef XAPP_LANG_PYTHON
    PyGILState_Release(gstate);
#endif

}



//////////////////////////////////////
// PDCP 
/////////////////////////////////////

static 
pdcp_cb* hndlr_pdcp_cb; 

static
sm_ans_xapp_t hndlr_pdcp_ans;

static
void sm_cb_pdcp(sm_ag_if_rd_t const* rd)
{
  assert(rd != NULL);
  assert(rd->type == PDCP_STATS_V0);
  assert(hndlr_pdcp_cb != NULL);

  pdcp_ind_data_t const* data = &rd->pdcp_stats; 

  swig_pdcp_ind_msg_t ind;
  ind.tstamp = data->msg.tstamp;

  for(uint32_t i = 0; i < data->msg.len; ++i){
    pdcp_radio_bearer_stats_t tmp = data->msg.rb[i];
    ind.rb_stats.push_back(tmp);
  }

#ifdef XAPP_LANG_PYTHON
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();
#endif

    hndlr_pdcp_cb->handle(&ind);

#ifdef XAPP_LANG_PYTHON
    PyGILState_Release(gstate);
#endif

}

void report_pdcp_sm(global_e2_node_id_t* id, Interval inter_arg, pdcp_cb* handler)
{
  assert(id != NULL);
  assert(handler != NULL);

  hndlr_pdcp_cb = handler;

  inter_xapp_e i;
  if(inter_arg == Interval::ms_1 ){
    i = ms_1;
  } else if (inter_arg == Interval::ms_2) {
    i = ms_2;
  } else if(inter_arg == Interval::ms_5) {
    i = ms_5;
  } else if(inter_arg == Interval::ms_10) {
    i = ms_10;
  } else {
    assert(0 != 0 && "Unknown type");
  }

  sm_ans_xapp_t ans = report_sm_xapp_api(id , SM_PDCP_ID, i, sm_cb_pdcp);
  assert(ans.success == true); 
  hndlr_pdcp_ans = ans;

}

void rm_report_pdcp_sm(void)
{

#ifdef XAPP_LANG_PYTHON
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();
#endif

  assert(hndlr_pdcp_ans.u.handle != 0);
  rm_report_sm_xapp_api(hndlr_pdcp_ans.u.handle);

#ifdef XAPP_LANG_PYTHON
    PyGILState_Release(gstate);
#endif

}

/*

static
pthread_t t_pdcp;

static 
pdcp_cb* hndlr_pdcp; 

static
void *start_routine_pdcp(void *arg)
{
  Interval inter = *(Interval*)arg;
  delete (Interval*)arg;
  assert(hndlr_pdcp != NULL);
  for(int i = 0; i < 3; ++i){
    swig_pdcp_ind_msg_t ind;
    ind.tstamp = time_now_us();

    for(int j = 0; j < 2; ++j){
      pdcp_radio_bearer_stats_t tmp;
      tmp.rnti = 42 + j;
      ind.rb_stats.emplace_back(tmp);
    }

#ifdef XAPP_LANG_PYTHON
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();
#endif

    hndlr_pdcp->handle(&ind);

#ifdef XAPP_LANG_PYTHON
    PyGILState_Release(gstate);
#endif

    sleep(1);
  }

  std::cout << "Ending the PDCP routine \n";
  return NULL;
}



void report_pdcp_sm(global_e2_node_id_t* id, Interval inter_arg, pdcp_cb* handler)
{
  hndlr_pdcp = handler; 
 
  Interval* inter = new Interval;
  *inter = inter_arg;

  if(inter_arg == Interval::ms_1)
    std::cout << "Everything went fine \n";

  std::cout << "Before thread create \n";
  int rc = pthread_create(&t_pdcp, NULL, start_routine_pdcp, inter);
  assert(rc == 0);
}

*/




//////////////////////////////////////
// SLICE Control  
/////////////////////////////////////

void report_slice_sm(global_e2_node_id_t* id, Interval inter, slice_cb* handler)
{
  assert( id != NULL);
  (void)inter;
  assert(handler != NULL);

  assert(0!=0 && "not implemented");
}

void control_slice_sm(global_e2_node_id_t* id, slice_ctrl_msg_t* ctrl)
{
  assert(id != NULL);
  assert(ctrl != NULL);


  if(ctrl->type == SLICE_CTRL_SM_V0_UE_SLICE_ASSOC){
    assert(ctrl->u.ue_slice.len_ue_slice == 2);
    for(size_t i = 0; i <  ctrl->u.ue_slice.len_ue_slice; ++i){
      std::cout << "RNTI = " <<  ctrl->u.ue_slice.ues[i].rnti << '\n' ;
    }
  } else if(ctrl->type == SLICE_CTRL_SM_V0_ADD){
      slice_conf_t* s_conf = &ctrl->u.add_mod_slice;
      assert(s_conf->dl.len_sched_name == 0);
      assert(s_conf->dl.sched_name == NULL);

      assert( s_conf->dl.len_slices == 2 && "For the test");
      for(size_t i =0; i < s_conf->dl.len_slices; ++i){
        slice_t* s = &s_conf->dl.slices[i];
        std::cout << "Slice id " << s->id << '\n';
        assert(s->len_label == 0 && "For the test");
        assert(s->label == NULL && "For the test");

        assert(s->len_sched == 0 && "For the test");
        assert(s->sched == NULL && "For the test");

        slice_params_t* p = &s->params;
        std::cout << "p->type == " << p->type << '\n';
        assert(p->type == SLICE_ALG_SM_V0_STATIC);
        static_slice_t* sta_sli = &p->u.sta;
        std::cout << "high pos = " << sta_sli->pos_high << " and low pos = " << sta_sli->pos_low << '\n';
        assert(sta_sli->pos_high >= sta_sli->pos_low);
      }
  } else {
    assert(0!=0 && "not foreseen case");
  }

  sm_ag_if_wr_t wr;
  wr.type = SLICE_CTRL_REQ_V0;
  wr.slice_req_ctrl.msg = cp_slice_ctrl_msg(ctrl);

  control_sm_xapp_api(id, SM_SLICE_ID,  &wr);
}

