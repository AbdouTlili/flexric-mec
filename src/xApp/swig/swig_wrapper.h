#ifndef SWIG_WRAPPER_H
#define SWIG_WRAPPER_H 

#include <string>
#include <memory>
#include <vector>
#include <cstddef>

#include "../../lib/ap/e2ap_types/common/e2ap_global_node_id.h"
#include "../../lib/ap/e2ap_types/common/e2ap_plmn.h"
#include "../../lib/ap/e2ap_types/common/e2ap_ran_function.h"

#include "../../sm/mac_sm/ie/mac_data_ie.h"
#include "../../sm/rlc_sm/ie/rlc_data_ie.h"
#include "../../sm/pdcp_sm/ie/pdcp_data_ie.h"
#include "../../sm/slice_sm/ie/slice_data_ie.h"


//#include "slice_data_ie.h"

//////////////////////////////////////
// General    
/////////////////////////////////////

struct E2Node {
  global_e2_node_id_t id;
  std::vector<ran_function_t> ran_func;
};


void init(const char* addr); 

bool try_stop(void);

std::vector<E2Node> conn_e2_nodes(void);

enum class Interval {
  ms_1,
  ms_2,
  ms_5,
  ms_10,
};


//////////////////////////////////////
// MAC SM   
/////////////////////////////////////

struct swig_mac_ind_msg_t{
  std::vector<mac_ue_stats_impl_t> ue_stats;
  int64_t tstamp;
};

struct mac_cb {
    virtual void handle(swig_mac_ind_msg_t* a) = 0;
    virtual ~mac_cb() {}
};

void report_mac_sm(global_e2_node_id_t* id, Interval inter, mac_cb* handler);

void rm_report_mac_sm(void);

//////////////////////////////////////
// RLC SM   
/////////////////////////////////////

struct swig_rlc_ind_msg_t{
  std::vector<rlc_radio_bearer_stats_t> rb_stats; 
  int64_t tstamp;
};

struct rlc_cb {
    virtual void handle(swig_rlc_ind_msg_t* a) = 0;
    virtual ~rlc_cb() {}
};

void report_rlc_sm(global_e2_node_id_t* id, Interval inter, rlc_cb* handler);

void rm_report_rlc_sm(void);

//////////////////////////////////////
// PDCP SM   
/////////////////////////////////////

struct swig_pdcp_ind_msg_t{
  std::vector<pdcp_radio_bearer_stats_t> rb_stats;
  int64_t tstamp;
};

struct pdcp_cb {
    virtual void handle(swig_pdcp_ind_msg_t* a) = 0;
    virtual ~pdcp_cb() {}
};

void report_pdcp_sm(global_e2_node_id_t* id, Interval inter, pdcp_cb* handler);

//void rm_report_pdcp_sm(void);

//////////////////////////////////////
// SLICE SM   
/////////////////////////////////////

struct swig_slice_ind_msg_t{
  int64_t tstamp;
};

struct slice_cb {
    virtual void handle(swig_slice_ind_msg_t* a) = 0;
    virtual ~slice_cb() {}
};

void report_slice_sm(global_e2_node_id_t* id, Interval inter, slice_cb* handler);

void control_slice_sm(global_e2_node_id_t* id, slice_ctrl_msg_t* ctrl);


#endif

