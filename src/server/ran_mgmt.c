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
#include <stdlib.h>
#include "ran_mgmt.h"

static
void free_ran(ran_t* r);

/* Maps an SCTP association to a RAN */
typedef struct pair_e2n_to_ran_s {
  int assoc_id;
  ran_t* ran;
} pair_e2n_to_ran_t;

static
pair_e2n_to_ran_t* alloc_pair_e2n_to_ran(int assoc_id, ran_t* ran)
{
  pair_e2n_to_ran_t* p = malloc(sizeof(*p));
  assert(p);
  p->assoc_id = assoc_id;
  p->ran = ran;
  return p;
}

static
void free_pair_e2n_to_ran(pair_e2n_to_ran_t* p)
{
  assert(p);
  free(p);
}

static
int cmp_pair_e2n_to_ran(const void* a_v, const void* b_v)
{
  const pair_e2n_to_ran_t* a = (pair_e2n_to_ran_t*)a_v;
  const pair_e2n_to_ran_t* b = (pair_e2n_to_ran_t*)b_v;
  if(a->assoc_id < b->assoc_id) return 1;
  if(a->assoc_id == b->assoc_id) return 0;
  return -1;
}

/* Maps a RAN key to a RAN (which again has the key). The point is: if e.g. a
 * CU is already connected and a DU connects, we need to map them to the same
 * RAN using their global_e2_node_id. */
typedef struct pair_key_to_ran_s {
  uint64_t key;
  ran_t* ran;
} pair_ran_key_to_ran_t;

static
pair_ran_key_to_ran_t* alloc_pair_ran_key_to_ran(uint64_t key, ran_t* ran)
{
  pair_ran_key_to_ran_t* p = malloc(sizeof(*p));
  assert(p);
  p->key = key;
  p->ran = ran;
  return p;
}

static
void free_pair_ran_key_to_ran(pair_ran_key_to_ran_t* p)
{
  assert(p);
  free_ran(p->ran);
  free(p);
}

static
int cmp_pair_ran_key_to_ran(const void* a_v, const void* b_v)
{
  const pair_ran_key_to_ran_t* a = (pair_ran_key_to_ran_t*)a_v;
  const pair_ran_key_to_ran_t* b = (pair_ran_key_to_ran_t*)b_v;
  if(a->key < b->key) return 1;
  if(a->key == b->key) return 0;
  return -1;
}

static
rat_type_e get_rat_type(ngran_node_t type)
{
  switch (type) {
    case ngran_eNB:
    case ngran_ng_eNB:
    case ngran_eNB_CU:
    case ngran_eNB_DU:
    case ngran_ng_eNB_CU:
    case ngran_eNB_MBMS_STA:
      return RAT_TYPE_LTE;
    case ngran_gNB:
    case ngran_gNB_CU:
    case ngran_gNB_DU:
      return RAT_TYPE_NR;
    default:
      assert(0 && "illegal path");
  }
  assert(0 && "illegal path");
}

static
char* get_str_rat_type(rat_type_e type)
{
  switch (type) {
    case RAT_TYPE_LTE:
      return "lte";
    case RAT_TYPE_NR:
      return "nr";
    default:
      assert(0 && "illegal path");
  }
  assert(0 && "illegal path");
}

static
uint64_t get_ran_key(const global_e2_node_id_t* id)
{
  /* the RAN key allows to associate CU/DU to the same base station */
  /* key: 8 bit rat type, 12 bit mcc (assume always 3 digits), 12 bit mnc, 32
   * bytes nb_id */
  const uint64_t rat = ((uint64_t) get_rat_type(id->type)) << (12 + 12 + 32);
  const uint64_t mcc = ((uint64_t) id->plmn.mcc) << (12 + 32);
  const uint64_t mnc = ((uint64_t) id->plmn.mnc) << (32);
  assert(id->nb_id <= 0xfffffff); /* nb_id up to 28 bits */
  const uint64_t nb_id = id->nb_id;
  return rat | mcc | mnc | nb_id;
}

static
e2_setup_request_t deep_copy_e2_setup_req(const e2_setup_request_t* sr)
{
  (void) sr;
  e2_setup_request_t ret = {
    .id = sr->id,
    .len_rf = sr->len_rf,
    .len_ccu = sr->len_ccu
  };

  ret.ran_func_item = calloc(ret.len_rf, sizeof(*ret.ran_func_item));
  if (ret.len_rf > 0)
    assert(ret.ran_func_item);
  for (size_t i = 0; i < ret.len_rf; ++i) {
    ran_function_t* old_rf = &sr->ran_func_item[i];
    ran_function_t* new_rf = &ret.ran_func_item[i];

    new_rf->def = copy_byte_array(old_rf->def);

    new_rf->id = old_rf->id;
    new_rf->rev = old_rf->rev;

    if (old_rf->oid) {
      new_rf->oid = malloc(sizeof(*new_rf->oid));
      assert(new_rf->oid);
      *new_rf->oid = copy_byte_array(*old_rf->oid);
    }
  }

  ret.comp_conf_update = calloc(ret.len_ccu, sizeof(*ret.comp_conf_update));
  if (ret.len_ccu > 0)
    assert(ret.comp_conf_update);
  for (size_t i = 0; i < ret.len_ccu; ++i) {
    e2_node_component_config_update_t* old_ccu = &sr->comp_conf_update[i];
    e2_node_component_config_update_t* new_ccu = &ret.comp_conf_update[i];

    new_ccu->e2_node_component_type = old_ccu->e2_node_component_type;

    if (old_ccu->id_present) {
      new_ccu->id_present = malloc(*new_ccu->id_present);
      assert(new_ccu->id_present);
      *new_ccu->id_present = *old_ccu->id_present;
      switch(*new_ccu->id_present) {
        case E2_NODE_COMPONENT_ID_E2_NODE_COMPONENT_TYPE_GNB_CU_UP:
          new_ccu->gnb_cu_up_id = old_ccu->gnb_cu_up_id;
          break;
        case E2_NODE_COMPONENT_ID_E2_NODE_COMPONENT_TYPE_GNB_DU:
          new_ccu->gnb_du_id = old_ccu->gnb_du_id;
          break;
      }
    }

    new_ccu->update_present = old_ccu->update_present;
    switch(new_ccu->update_present) {
      case E2_NODE_COMPONENT_CONFIG_UPDATE_NOTHING:
        break;
      case E2_NODE_COMPONENT_CONFIG_UPDATE_GNB_CONFIG_UPDATE:
        if (old_ccu->gnb.ngap_gnb_cu_cp) {
          new_ccu->gnb.ngap_gnb_cu_cp = malloc(sizeof(*new_ccu->gnb.ngap_gnb_cu_cp));
          assert(new_ccu->gnb.ngap_gnb_cu_cp);
          *new_ccu->gnb.ngap_gnb_cu_cp = copy_byte_array(*old_ccu->gnb.ngap_gnb_cu_cp);
        }
        if (old_ccu->gnb.xnap_gnb_cu_cp) {
          new_ccu->gnb.xnap_gnb_cu_cp = malloc(sizeof(*new_ccu->gnb.xnap_gnb_cu_cp));
          assert(new_ccu->gnb.xnap_gnb_cu_cp);
          *new_ccu->gnb.xnap_gnb_cu_cp = copy_byte_array(*old_ccu->gnb.xnap_gnb_cu_cp);
        }
        if (old_ccu->gnb.e1ap_gnb_cu_cp) {
          new_ccu->gnb.e1ap_gnb_cu_cp = malloc(sizeof(*new_ccu->gnb.e1ap_gnb_cu_cp));
          assert(new_ccu->gnb.e1ap_gnb_cu_cp);
          *new_ccu->gnb.e1ap_gnb_cu_cp = copy_byte_array(*old_ccu->gnb.e1ap_gnb_cu_cp);
        }
        if (old_ccu->gnb.f1ap_gnb_cu_cp) {
          new_ccu->gnb.f1ap_gnb_cu_cp = malloc(sizeof(*new_ccu->gnb.f1ap_gnb_cu_cp));
          assert(new_ccu->gnb.f1ap_gnb_cu_cp);
          *new_ccu->gnb.f1ap_gnb_cu_cp = copy_byte_array(*old_ccu->gnb.f1ap_gnb_cu_cp);
        }
        break;
      case E2_NODE_COMPONENT_CONFIG_UPDATE_EN_GNB_CONFIG_UPDATE:
        if (old_ccu->en_gnb.x2ap_en_gnb) {
          new_ccu->en_gnb.x2ap_en_gnb = malloc(sizeof(*new_ccu->en_gnb.x2ap_en_gnb));
          assert(new_ccu->en_gnb.x2ap_en_gnb);
          *new_ccu->en_gnb.x2ap_en_gnb = copy_byte_array(*old_ccu->en_gnb.x2ap_en_gnb);
        }
        break;
      case E2_NODE_COMPONENT_CONFIG_UPDATE_NG_ENB_CONFIG_UPDATE:
        if (old_ccu->ng_enb.ngap_ng_enb) {
          new_ccu->ng_enb.ngap_ng_enb = malloc(sizeof(*new_ccu->ng_enb.ngap_ng_enb));
          assert(new_ccu->ng_enb.ngap_ng_enb);
          *new_ccu->ng_enb.ngap_ng_enb = copy_byte_array(*old_ccu->ng_enb.ngap_ng_enb);
        }
        if (old_ccu->ng_enb.xnap_ng_enb) {
          new_ccu->ng_enb.xnap_ng_enb = malloc(sizeof(*new_ccu->ng_enb.xnap_ng_enb));
          assert(new_ccu->ng_enb.xnap_ng_enb);
          *new_ccu->ng_enb.xnap_ng_enb = copy_byte_array(*old_ccu->ng_enb.xnap_ng_enb);
        }
        break;
      case E2_NODE_COMPONENT_CONFIG_UPDATE_ENB_CONFIG_UPDATE:
        if (old_ccu->enb.s1ap_enb) {
          new_ccu->enb.s1ap_enb = malloc(sizeof(*new_ccu->enb.s1ap_enb));
          assert(new_ccu->enb.s1ap_enb);
          *new_ccu->enb.s1ap_enb = copy_byte_array(*old_ccu->enb.s1ap_enb);
        }
        if (old_ccu->enb.x2ap_enb) {
          new_ccu->enb.x2ap_enb = malloc(sizeof(*new_ccu->enb.x2ap_enb));
          assert(new_ccu->enb.x2ap_enb);
          *new_ccu->enb.x2ap_enb = copy_byte_array(*old_ccu->enb.x2ap_enb);
        }
        break;
    }
  }
  return ret;
}

static
void free_e2_setup_request(e2_setup_request_t* sr)
{
  for (size_t i = 0; i < sr->len_rf; ++i) {
    ran_function_t* rf = &sr->ran_func_item[i];
    free(rf->def.buf);
    if (rf->oid) {
      free(rf->oid->buf);
      free(rf->oid);
    }
  }
  free(sr->ran_func_item);
  for (size_t i = 0; i < sr->len_ccu; ++i) {
    e2_node_component_config_update_t* ccu = &sr->comp_conf_update[i];
    if (ccu->id_present)
      free(ccu->id_present);

    switch(ccu->update_present) {
      case E2_NODE_COMPONENT_CONFIG_UPDATE_NOTHING:
        break;
      case E2_NODE_COMPONENT_CONFIG_UPDATE_GNB_CONFIG_UPDATE:
        if (ccu->gnb.ngap_gnb_cu_cp) {
          free(ccu->gnb.ngap_gnb_cu_cp->buf);
          free(ccu->gnb.ngap_gnb_cu_cp);
        }
        if (ccu->gnb.xnap_gnb_cu_cp) {
          free(ccu->gnb.xnap_gnb_cu_cp->buf);
          free(ccu->gnb.xnap_gnb_cu_cp);
        }
        if (ccu->gnb.e1ap_gnb_cu_cp) {
          free(ccu->gnb.e1ap_gnb_cu_cp->buf);
          free(ccu->gnb.e1ap_gnb_cu_cp);
        }
        if (ccu->gnb.f1ap_gnb_cu_cp) {
          free(ccu->gnb.f1ap_gnb_cu_cp->buf);
          free(ccu->gnb.f1ap_gnb_cu_cp);
        }
        break;
      case E2_NODE_COMPONENT_CONFIG_UPDATE_EN_GNB_CONFIG_UPDATE:
        if (ccu->en_gnb.x2ap_en_gnb) {
          free(ccu->en_gnb.x2ap_en_gnb->buf);
          free(ccu->en_gnb.x2ap_en_gnb);
        }
        break;
      case E2_NODE_COMPONENT_CONFIG_UPDATE_NG_ENB_CONFIG_UPDATE:
        if (ccu->ng_enb.ngap_ng_enb) {
          free(ccu->ng_enb.ngap_ng_enb->buf);
          free(ccu->ng_enb.ngap_ng_enb);
        }
        if (ccu->ng_enb.xnap_ng_enb) {
          free(ccu->ng_enb.xnap_ng_enb->buf);
          free(ccu->ng_enb.xnap_ng_enb);
        }
        break;
      case E2_NODE_COMPONENT_CONFIG_UPDATE_ENB_CONFIG_UPDATE:
        if (ccu->enb.s1ap_enb) {
          free(ccu->enb.s1ap_enb->buf);
          free(ccu->enb.s1ap_enb);
        }
        if (ccu->enb.x2ap_enb) {
          free(ccu->enb.x2ap_enb->buf);
          free(ccu->enb.x2ap_enb);
        }
        break;
    }
  }
  free(sr->comp_conf_update);
}

static
ran_t* create_new_ran(const e2_setup_request_t* sr, int assoc_id)
{
  ran_t* r = calloc(1, sizeof(*r));
  assert(r);
  r->generated_ran_key = get_ran_key(&sr->id);
  r->rat = get_rat_type(sr->id.type);

  assert(sr->id.type == ngran_gNB && "cannot handle splits yet");
  entity_info_t *entity = malloc(sizeof(*entity));
  assert(entity);
  entity->e2_setup_request = deep_copy_e2_setup_req(sr);
  entity->assoc_id = assoc_id;
  r->entities.split = SPLIT_NONE;
  r->entities.nosplit.full = entity;
  return r;
}

static
void free_entity_info(entity_info_t** e)
{
  free_e2_setup_request(&(*e)->e2_setup_request);
  free(*e);
  *e = NULL;
}

static
void free_ran(ran_t* r)
{
  entities_t* e = &r->entities;
  switch (e->split) {
    case SPLIT_NONE:
      if (e->nosplit.full)
        free_entity_info(&e->nosplit.full);
      break;
    case SPLIT_F1:
      if (e->f1.cu)
        free_entity_info(&e->f1.cu);
      if (e->f1.du)
        free_entity_info(&e->f1.du);
      break;
    case SPLIT_F1_E1:
      if (e->f1e1.du)
        free_entity_info(&e->f1e1.du);
      if (e->f1e1.cu_cp)
        free_entity_info(&e->f1e1.cu_cp);
      if (e->f1e1.cu_up)
        free_entity_info(&e->f1e1.cu_up);
      break;
    default:
      assert(0 && "illegal code path");
  }
  free(r);
}

/* returns the number of remaining connected E2 nodes */
static
int remove_entity_from_ran(ran_t* r, int assoc_id)
{
  entities_t* e = &r->entities;
  switch (e->split) {
    case SPLIT_NONE:
      assert(e->nosplit.full && e->nosplit.full->assoc_id == assoc_id);
      free_entity_info(&e->nosplit.full);
      return 0;
    case SPLIT_F1:
      {
        const bool remove_cu = e->f1.cu && e->f1.cu->assoc_id == assoc_id;
        const bool remove_du = e->f1.du && e->f1.du->assoc_id == assoc_id;
        assert(remove_cu || remove_du);
        if (remove_cu)
          free_entity_info(&e->f1.cu);
        if (remove_du)
          free_entity_info(&e->f1.du);
        return !(!e->f1.cu) + !(!e->f1.du);
      }
    case SPLIT_F1_E1:
      {
        const bool remove_du = e->f1e1.du && e->f1e1.du->assoc_id == assoc_id;
        const bool remove_cucp = e->f1e1.cu_cp && e->f1e1.cu_cp->assoc_id == assoc_id;
        const bool remove_cuup = e->f1e1.cu_up && e->f1e1.cu_up->assoc_id == assoc_id;
        assert(remove_du || remove_cucp || remove_cuup);
        if (remove_du)
          free_entity_info(&e->f1e1.du);
        if (remove_cucp)
          free_entity_info(&e->f1e1.cu_cp);
        if (remove_cuup)
          free_entity_info(&e->f1e1.cu_up);
        return !(!e->f1e1.du) + !(!e->f1e1.cu_cp) + !(!e->f1e1.cu_up);
      }
    default:
      assert(0 && "illegal code path");
  }
  assert(0 && "illegal code path");
}

void ran_mgmt_init(ran_mgmt_t* ran_mgmt)
{
  assert(ran_mgmt);
  memset(ran_mgmt, 0, sizeof(*ran_mgmt));
  init_rb_tree(&ran_mgmt->ran_key_to_ran, cmp_pair_ran_key_to_ran);
  init_rb_tree(&ran_mgmt->e2n_to_ran, cmp_pair_e2n_to_ran);
}

void ran_mgmt_free(ran_mgmt_t* ran_mgmt)
{
  assert(ran_mgmt);
  free_rb_tree(&ran_mgmt->ran_key_to_ran, (void (*)(void*))free_pair_ran_key_to_ran);
  free_rb_tree(&ran_mgmt->e2n_to_ran, (void (*)(void*))free_pair_e2n_to_ran);
}

const ran_t* ran_mgmt_add_e2_node(ran_mgmt_t* rm, int assoc_id, const e2_setup_request_t* sr)
{
  uint64_t ran_key = get_ran_key(&sr->id);
  node_t *nkey = find_rb_tree(&rm->ran_key_to_ran, rm->ran_key_to_ran.dummy, &ran_key);
  ran_t *r;
  if (nkey == rm->ran_key_to_ran.dummy) {
    /* create new RAN info and store it */
    r = create_new_ran(sr, assoc_id);
    pair_ran_key_to_ran_t* pk = alloc_pair_ran_key_to_ran(ran_key, r);
    nkey = create_node_rb_tree(&rm->ran_key_to_ran, pk);
    insert_rb_tree(&rm->ran_key_to_ran, nkey);
    /* map the assoc_id to the previously created RAN info */
    node_t* nrr = find_rb_tree(&rm->e2n_to_ran, rm->e2n_to_ran.dummy, &assoc_id);
    assert(nrr == rm->e2n_to_ran.dummy && "this association is already in the RAN tree");
    pair_e2n_to_ran_t* p = alloc_pair_e2n_to_ran(assoc_id, r);
    nrr = create_node_rb_tree(&rm->e2n_to_ran, p);
    insert_rb_tree(&rm->e2n_to_ran, nrr);
    char *name = generate_ran_name(r->generated_ran_key);
    printf("create new RAN information: name %s/ID %lx for E2 node at assoc_id %d\n",
           name, r->generated_ran_key, assoc_id);
    free(name);
  } else {
    assert(0 && "merge of two RANs is not implemented yet");
  }
  return r;
}

const ran_t* ran_mgmt_remove_e2_node(ran_mgmt_t* rm, int assoc_id)
{
  pair_e2n_to_ran_t k = { .assoc_id = assoc_id, .ran = NULL };
  node_t* n = find_rb_tree(&rm->e2n_to_ran, rm->e2n_to_ran.root, &k);
  assert(n != rm->e2n_to_ran.dummy && "RAN info not found for assoc_id");
  pair_e2n_to_ran_t* p = (pair_e2n_to_ran_t*) n->key;
  const int remaining_nodes = remove_entity_from_ran(p->ran, assoc_id);
  /* if no entity references this RAN info, remove the RAN info */
  if (remaining_nodes == 0) {
    const ran_t* r = p->ran;
    char *name = generate_ran_name(r->generated_ran_key);
    printf("remove RAN information: name %s/ID %lx (last E2 node at assoc_id %d is gone)\n",
           name, r->generated_ran_key, assoc_id);
    free(name);
    pair_ran_key_to_ran_t l = { .key = r->generated_ran_key, .ran = NULL };
    node_t* m = find_rb_tree(&rm->ran_key_to_ran, rm->ran_key_to_ran.root, &l);
    assert(m != rm->ran_key_to_ran.dummy);
    free_pair_ran_key_to_ran(m->key);
    delete_rb_tree(&rm->ran_key_to_ran, m);
  }

  /* remove from e2n_to_ran */
  free_pair_e2n_to_ran(n->key);
  delete_rb_tree(&rm->e2n_to_ran, n);
  return NULL;
}

size_t ran_mgmt_get_ran_num(ran_mgmt_t* rm)
{
  assert(rm);
  return rm->ran_key_to_ran.size;
}

size_t ran_mgmt_get_e2_node_num(ran_mgmt_t* rm)
{
  assert(rm);
  return rm->e2n_to_ran.size;
}

ssize_t ran_mgmt_get_rans(ran_mgmt_t* rm, ran_t** rans, ssize_t max_ran_ids)
{
  assert(rm);
  assert(rans);
  assert(max_ran_ids > 0);
  const ssize_t num_keys = rm->ran_key_to_ran.size;
  const ssize_t num_ids = max_ran_ids < num_keys ? max_ran_ids : num_keys;
  ssize_t iter = 0;
  void copy_ran(void* c, void* n) {
    (void) c;
    if (iter >= num_ids)
      return; /* is there a way to abort rb_tree_iterate()? */
    pair_ran_key_to_ran_t* p = (pair_ran_key_to_ran_t*) n;
    rans[iter++] = p->ran;
  }
  iterate_rb_tree(&rm->ran_key_to_ran, rm->ran_key_to_ran.root, NULL, copy_ran);
  return max_ran_ids < num_keys ? -1 : num_keys;
}

/*
 * We don't want to keep two separate RB trees, but as things stand, this is
 * currently the easiest solution. We don't expose the below function, but
 * I leave it so we might reuse later.
 */
/*
const ran_t* ran_mgmt_get_ran_from_ran_key(ran_mgmt_t* rm, uint64_t id)
{
  assert(rm);
  pair_ran_key_to_ran_t k = { .key = id, .ran = NULL };
  node_t* n = find_rb_tree(&rm->ran_key_to_ran, rm->ran_key_to_ran.root, &k);
  if (n == rm->ran_key_to_ran.dummy)
    return NULL;
  pair_ran_key_to_ran_t* p = (pair_ran_key_to_ran_t*) n->key;
  return p->ran;
}
*/

const ran_t* ran_mgmt_get_ran_from_assoc_id(ran_mgmt_t* rm, int assoc_id)
{
  assert(rm);
  pair_e2n_to_ran_t k = { .assoc_id = assoc_id, .ran = NULL };
  node_t* n = find_rb_tree(&rm->e2n_to_ran, rm->e2n_to_ran.root, &k);
  if (n == rm->e2n_to_ran.dummy)
    return NULL;
  pair_ran_key_to_ran_t* p = (pair_ran_key_to_ran_t*) n->key;
  return p->ran;
}

bool ran_mgmt_is_ran_formed(const ran_t* r)
{
  const entities_t* e = &r->entities;
  switch (e->split) {
    case SPLIT_NONE:
      assert(e->nosplit.full);
      return true;
    case SPLIT_F1:
      assert(e->f1.cu || e->f1.du);
      return e->f1.cu && e->f1.du;
    case SPLIT_F1_E1:
      assert(e->f1e1.du || e->f1e1.cu_cp || e->f1e1.cu_up);
      return e->f1e1.du && e->f1e1.cu_cp && e->f1e1.cu_up;
    default:
      assert(0 && "illegal code path");
  }
  assert(0 && "illegal code path");
}

char* generate_ran_name(uint64_t ran_key)
{
  /* O-RAN RIC generates name in the form "type_mcc_mnc_nbid" where
   * - type: is gnb/enb/maybe cu -> we use the RAT type to keep the nodes together
   * - mcc: always 3 digits(!) of the actual MCC (12 bits)
   * - mnc: 3 digits of the MNC (12 bits)
   * - nb_id: binary representation of node ID, 28 bits long. We just print it
   *   hex for the moment, up to 7 bytes
   * Assuming up to 5 chars for "type" (to be safe), we have 5+1+3+1+3+1+7=21
   * characters long RAN name
   */
  const rat_type_e rat = ran_key >> (12 + 12 + 32);
  assert(rat == RAT_TYPE_LTE || rat == RAT_TYPE_NR);
  const uint32_t mcc = (ran_key >> (12 + 32)) & 0xfff;
  assert(mcc < 1000);
  const uint32_t mnc = (ran_key >> 32) & 0xfff;
  assert(mcc < 1000);
  const uint32_t nb_id = ran_key & 0xfffffff; /* nb_id is actually only 28 bits wide */
  assert(mcc < 1000);
  const size_t max_offset = 23;
  char* ran_name = malloc(sizeof(char) * max_offset); /* including NULL */
  size_t offset = 0;
  offset += snprintf(&ran_name[offset], max_offset - offset, "%s", get_str_rat_type(rat));
  assert(offset < max_offset);
  offset += snprintf(&ran_name[offset], max_offset - offset, "_%03d", mcc);
  assert(offset < max_offset);
  offset += snprintf(&ran_name[offset], max_offset - offset, "_%03d", mnc);
  assert(offset < max_offset);
  offset += snprintf(&ran_name[offset], max_offset - offset, "_%07x", nb_id);
  assert(offset < max_offset);
  return ran_name;
}

entity_info_t* ran_mgmt_get_entity(const ran_t* ran, int assoc_id)
{
  assert(ran);
  entity_info_t* e = NULL;
  switch (ran->entities.split) {
    case SPLIT_NONE:
      e = ran->entities.nosplit.full;
      break;
    case SPLIT_F1:
      if (ran->entities.f1.cu && ran->entities.f1.cu->assoc_id == assoc_id)
        e = ran->entities.f1.cu;
      else if (ran->entities.f1.du && ran->entities.f1.du->assoc_id == assoc_id)
        e = ran->entities.f1.du;
      break;
    case SPLIT_F1_E1:
      if (ran->entities.f1e1.cu_cp && ran->entities.f1e1.cu_cp->assoc_id == assoc_id)
        e = ran->entities.f1e1.cu_cp;
      else if (ran->entities.f1e1.cu_up && ran->entities.f1e1.cu_up->assoc_id == assoc_id)
        e = ran->entities.f1e1.cu_up;
      else if (ran->entities.f1e1.du && ran->entities.f1e1.du->assoc_id == assoc_id)
        e = ran->entities.f1e1.du;
      break;
  }
  assert(e && e->assoc_id == assoc_id);
  return e;
}
