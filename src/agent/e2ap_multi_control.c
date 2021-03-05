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

#include <assert.h>
#include <stdio.h>

#include "e2ap_multi_control.h"
#include "e2ap_msg_enc_generic.h"

ep_id_t get_endpoint(e2ap_agent_t* ag, int fd)
{
  for (int i = 0; i < ag->num_ep; ++i)
    if (fd == ag->ep[i].base.fd)
      return i;
  return -1;
}

ep_id_t add_endpoint(e2ap_agent_t* ag, const char* addr, int port, const e2_setup_request_t* sr)
{
  assert(ag);
  assert(addr);

  if (ag->num_ep >= ag->cap_ep - 1) {
    printf("realloc ep after num_ep %d cap_ep %d\n", ag->num_ep, ag->cap_ep);
    ag->cap_ep *= 2;
    ag->ep = reallocarray(ag->ep, ag->cap_ep, sizeof(*ag->ep));
    assert(ag->ep);
  }

  /* TODO: use rb_tree or ticket_map for multiple controllers so we can search
   * efficiently and remove them easily while using the fd as key */
  ep_id_t new_ep = ag->num_ep;
  assert(new_ep > 0);
  e2ap_ep_ag_t* ep = &ag->ep[new_ep];
  e2ap_init_ep_agent(ep, addr, port);
  add_fd_epoll(ag->efd, ep->base.fd);
  /* initialize a hash table for associated UEs */
  hash_table_init(&ep->assoc_ues, 6);
  ag->num_ep++;

  printf("%s(): sending e2 setup request\n", __func__);
  byte_array_t ba = e2ap_enc_setup_request(sr, &ag->enc.type);

  e2ap_send_bytes_agent(&ag->ep[new_ep], ba);
  free_byte_array(ba);

  return new_ep;
}

void remove_endpoint(e2ap_agent_t* ag, ep_id_t ep_id)
{
  assert(ag);
  /* hacky implementation: can only remove the last endpoint */
  assert(ep_id == ag->num_ep - 1);
  void free_func(hentry_t* e) {
    /* nothing to free */
    assert(!e->data);
  }
  ag->num_ep--;
  e2ap_ep_ag_t* ep = &ag->ep[ep_id];
  hash_table_destroy(&ep->assoc_ues, free_func);
}

void endpoint_associate_rnti(e2ap_agent_t* ag, ep_id_t ep_id, uint16_t rnti)
{
  assert(ag);
  /* only additional controllers can have associated UEs */
  assert(ep_id > 0 && ep_id < ag->num_ep);
  e2ap_ep_ag_t* ep = &ag->ep[ep_id];
  const hkey_t key = { .word = { rnti }, .len = 1 };
  const hentry_t* e = hash_table_insert(&ep->assoc_ues, key, NULL);
  assert(e);
}

void endpoint_dissociate_rnti(e2ap_agent_t* ag, ep_id_t ep_id, uint16_t rnti)
{
  assert(ag);
  /* only additional controllers can have associated UEs */
  assert(ep_id > 0 && ep_id < ag->num_ep);
  e2ap_ep_ag_t* ep = &ag->ep[ep_id];
  const hkey_t key = { .word = { rnti }, .len = 1 };
  const bool ret = hash_table_remove(&ep->assoc_ues, key);
  assert(ret);
}

bool endpoint_has_rnti(e2ap_agent_t* ag, ep_id_t ep_id, uint16_t rnti)
{
  assert(ag);
  assert(ep_id < ag->num_ep);
  if (ep_id == 0)
    return true; /* any UE is associated to the main controller */

  /* only additional controllers can have associated UEs */
  e2ap_ep_ag_t* ep = &ag->ep[ep_id];
  const hkey_t key = { .word = { rnti }, .len = 1 };
  const hentry_t* f = hash_table_find(&ep->assoc_ues, key);
  return f != NULL;
}

void endpoint_rnti_iterate(e2ap_agent_t* ag, ep_id_t ep_id, void (*func)(uint16_t, void*), void* data)
{
  assert(ag);
  /* only additional controllers can have associated UEs, so we can only
   * iterate for those */
  assert(ep_id > 0 && ep_id < ag->num_ep);
  e2ap_ep_ag_t* ep = &ag->ep[ep_id];
  void ht_iter(hentry_t* e, void* d) {
    assert(!d);
    /* first word in key has rnti */
    func(e->key.word[0], data);
  }
  hash_table_iterate(&ep->assoc_ues, ht_iter, NULL);
}
