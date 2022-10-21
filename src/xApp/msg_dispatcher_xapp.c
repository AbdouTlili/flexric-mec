#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "../util/alg_ds/alg/defer.h"

#include "msg_dispatcher_xapp.h"



static
msg_dispatch_t static_msg; 

static
void* create_val(void* it)
{
  if(it == NULL)
    return NULL;

 // msg_dispatch_t* msg = calloc(1, sizeof( msg_dispatch_t )); 
//  assert(msg != NULL && "Memory exhausted");

  memcpy(&static_msg, it, sizeof(msg_dispatch_t ) );
  
  return &static_msg;
}


static
void* worker_thread(void* arg)
{
  tsq_t* q = (tsq_t*)arg;

  while(true){
    msg_dispatch_t* msg = wait_and_pop_tsq(q,  create_val);
    if(msg == NULL)
      break;

    msg->sm_cb(&msg->rd);
    free_sm_ag_if_rd(&msg->rd);
  }
  q->stopped = true;

  return NULL;
}


void init_msg_dispatcher( msg_dispatcher_xapp_t* d)
{
  assert(d != NULL);

  init_tsq(&d->q, sizeof(msg_dispatch_t));
  int rc = pthread_create(&d->p, NULL, worker_thread, &d->q);
  assert(rc == 0);
}

void free_msg_dispatcher(msg_dispatcher_xapp_t* d)
{
  assert(d != NULL);

  free_tsq(&d->q, NULL);
  int rc = pthread_join(d->p, NULL);
  assert(rc == 0);
}

void send_msg_dispatcher( msg_dispatcher_xapp_t* d, msg_dispatch_t* msg )
{
  assert(d != NULL);
  assert(msg != NULL);

  push_tsq(&d->q, msg, sizeof(msg_dispatch_t));
}

size_t size_msg_dispatcher(msg_dispatcher_xapp_t* d)
{
  assert(d != NULL);

  return size_tsq(&d->q);
}

