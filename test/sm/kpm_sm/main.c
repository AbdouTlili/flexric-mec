/*  
 * E2E test for KPM SM, no messages on the wire transported by E2AP.
 */

#include "../../../src/sm/kpm_sm_v2.02/kpm_sm_agent.h"
#include "../../../src/sm/kpm_sm_v2.02/kpm_sm_ric.h"
#include "../../../src/sm/kpm_sm_v2.02/kpm_sm_id.h"
#include "../../../src/util/alg_ds/alg/defer.h"
#include "../common/fill_ind_data.h"


#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

// 'cp' is buffer in  reception to compare the received indication message against the sent one
static kpm_ind_data_t cp; 
#define Logme printf


// AGENT part of the architecture. The communication with the RAN is achieved via READ/WRITE methods below
// write_RAN() for the moment does nothing.
static void read_RAN(sm_ag_if_rd_t* read)
{
  assert(read != NULL);
  assert(read->type == KPM_STATS_V0);

  fill_kpm_ind_data(&read->kpm_stats); 
  cp.hdr = cp_kpm_ind_hdr(&read->kpm_stats.hdr);
  cp.msg = cp_kpm_ind_msg(&read->kpm_stats.msg);
}

static 
sm_ag_if_ans_t write_RAN(sm_ag_if_wr_t const* data)
{
  assert(data != NULL);
  
  sm_ag_if_ans_t ans = {0};
  return ans;
}


/////////////////////////////
// Check Functions
// //////////////////////////

/* IE exchanged: none. Direction: none
 * XXX: This should be extended to check IE Ran Function definition
 */
static
void check_eq_ran_function(sm_agent_t * ag, sm_ric_t const* ric)
{
  assert(ag != NULL);
  assert(ric != NULL);
  assert(ag->ran_func_id == ric->ran_func_id);
  assert(ag->ran_func_id == SM_KPM_ID);
  Logme ("[IE RAN function ID]: Agent and RIC are using the same RAN function ID for KPM: %d\n", SM_KPM_ID);

  // TODO: check the encoding/decoding of RAN function definition. 
  // this can't be done until the signature of `ric_on_e2_setup_kpm_sm_ric()` is not changed to return function definition data structure.
  sm_e2_setup_t data = ag->proc.on_e2_setup(ag);
  ric->proc.on_e2_setup(ric, &data);

  free_sm_e2_setup(&data);
}

/* Direction: RIC -> E2 
 * IE exchanged: RIC Event Trigger Definition + RIC Action Definition 
 */
static
void check_subscription(sm_agent_t* ag, sm_ric_t* ric)
{
  assert(ag != NULL);
  assert(ric != NULL);
 
  sm_subs_data_t data = ric->proc.on_subscription(ric, "2_ms");
  Logme ("[IE RIC Event Trigger Definition] correctly encoded\n");
  Logme ("[IE RIC Action Definition] correctly encoded\n");
  assert ((ag->proc.on_subscription(ag, &data)).ms == 2 && "error in decoding trigger");
  Logme ("[IE RIC Event Trigger Definition] correctly decoded\n");
  
  free_sm_subs_data(&data);
}

/* Direction: E2 -> RIC
 * IE exchanged: RIC Indication Header, RIC Indication Message
 */
static
void check_indication(sm_agent_t* ag, sm_ric_t* ric)
{
  assert(ag != NULL);
  assert(ric != NULL);

  // sending IE indication. Behind the scenes it will call the read_RAN()
  sm_ind_data_t sm_data = ag->proc.on_indication(ag);
  Logme ("[IE RIC Indication Header]: correctly encoded\n");

  // receiving IE indication  (decoding)
  sm_ag_if_rd_t msg = ric->proc.on_indication(ric, &sm_data);
  Logme ("[IE RIC Indication Message] correctly decoded\n");

  // check for indication message correctness
  kpm_ind_data_t* data = &msg.kpm_stats;
  assert(msg.type == KPM_STATS_V0);
  assert(eq_kpm_ind_msg(&cp.msg, &data->msg) == true && "Failure checking for correctness in indication data IE");
    
  free_kpm_ind_hdr(&data->hdr); 
  free_kpm_ind_msg(&data->msg); 

  free_sm_ind_data(&sm_data);

  free_kpm_ind_hdr(&cp.hdr);
  free_kpm_ind_msg(&cp.msg);
}

int main()
{
  Logme("KPM SM unit test launched\n");
  sm_io_ag_t io_ag = {.read = read_RAN, .write = write_RAN};  
  sm_agent_t* sm_ag = make_kpm_sm_agent(io_ag);
  sm_ric_t* sm_ric = make_kpm_sm_ric();

  Logme("-> STEP 1. Controlling RAN function ................\n");
  check_eq_ran_function(sm_ag, sm_ric);
  Logme("-> STEP 2. Controlling Subscription procedure.......\n");
  check_subscription(sm_ag, sm_ric);
  Logme("-> STEP 3. Controlling Indication procedure.........\n");
  check_indication(sm_ag, sm_ric);

  Logme("-> STEP 4. Freeing memory...........................\n");
  sm_ag->free_sm(sm_ag);
  sm_ric->free_sm(sm_ric);
  
  Logme("-> Test completed with success. Closing all\n");

  return EXIT_SUCCESS;
}