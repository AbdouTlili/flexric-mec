#include "../ric/near_ric_api.h"

#include <assert.h>
#include <stdbool.h>

static
const uint16_t MAC_ran_func_id = 142;
static
const uint16_t RLC_ran_func_id = 143;
static
const char* cmd = "1_ms";
static
bool start = false;

void init()
{
  if(start == false){
  const char* addr = "127.0.0.1";
  init_near_ric_api(addr);
  start = true;
  }
}

void stop()
{
  if(start == true){
   stop_near_ric_api();
   start = false;
  }
}

void report_service(int ran_func_id)
{
  if(start == false)
    return;

  assert(ran_func_id == MAC_ran_func_id  || ran_func_id == RLC_ran_func_id);

  report_service_near_ric_api(ran_func_id, cmd);
}

void control_service(int ran_func_id)
{
  if(start == false)
    return;

  assert(ran_func_id == MAC_ran_func_id  || ran_func_id == RLC_ran_func_id);

  const char* cmd = "Hello";
  control_service_near_ric_api(MAC_ran_func_id, cmd );  

}


void remove_service(int ran_func_id)
{
  if(start == false)
    return;

  const uint16_t MAC_ran_func_id = 142;
  const uint16_t RLC_ran_func_id = 143;

  assert(ran_func_id == MAC_ran_func_id  || ran_func_id == RLC_ran_func_id);
 
  rm_report_service_near_ric_api(ran_func_id,cmd);
}


void load_sm(const char* file)
{
  if(start == false)
    return;

  load_sm_near_ric_api(file);
}

