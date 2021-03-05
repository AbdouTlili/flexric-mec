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

#ifndef RAN_MGMT_H
#define RAN_MGMT_H

#include "e2_setup_request.h"

#include "rb_tree.h"
#include <stdlib.h>

typedef enum {
  RAT_TYPE_LTE,
  RAT_TYPE_NR
} rat_type_e;

typedef enum {
  SPLIT_NONE,
  SPLIT_F1,
  SPLIT_F1_E1
} split_e;

typedef struct entity_info_s {
  e2_setup_request_t e2_setup_request;
  int assoc_id; // To send a message to a specific entity
} entity_info_t;

typedef struct entities_s {
  split_e split;
  union {
    struct { // SPLIT_NONE
      entity_info_t* full;
    } nosplit;
    struct { // SPLIT_F1
      entity_info_t* cu;
      entity_info_t* du;
    } f1;
    struct { // SPLIT_F1_E1
      entity_info_t* du;
      entity_info_t* cu_cp;
      entity_info_t* cu_up;
    } f1e1;
  };
} entities_t;

typedef struct ran_s {
  uint64_t generated_ran_key;
  rat_type_e rat;
  entities_t entities;
} ran_t;

typedef struct ran_mgmt_s {
  rb_tree_t e2n_to_ran;
  rb_tree_t ran_key_to_ran;
} ran_mgmt_t;

void ran_mgmt_init(ran_mgmt_t* ran_mgmt);
void ran_mgmt_free(ran_mgmt_t* ran_mgmt);

/* add a new E2 node to the RAN DB. Returns the newly created or merged RAN.
 * Asserts if such association is already present. Note: Does not handle split
 * base stations yet! */
const ran_t* ran_mgmt_add_e2_node(ran_mgmt_t* rm, int assoc_id, const e2_setup_request_t* sr);
/* remove an E2 node from the RAN DB. Returns the "remaining" RAN, or NULL if
 * it was the last E2 node referencing the RAN */
const ran_t* ran_mgmt_remove_e2_node(ran_mgmt_t* rm, int assoc_id);

/* returns the number of RANs in the DB */
size_t ran_mgmt_get_ran_num(ran_mgmt_t* rm);
/* returns the number of E2 associations known */
size_t ran_mgmt_get_e2_node_num(ran_mgmt_t* rm);

/* Writes the (pointer to) known RANs to array rans, up to max_ran_ids, and
 * returns the number of elements written. If max_ran_ids is smaller than the
 * number of RANs known, only writes up to max_ran_ids, and returns -1. */
ssize_t ran_mgmt_get_rans(ran_mgmt_t* rm, ran_t** rans, ssize_t max_rans_len);

/* returns the RAN matching the association ID */
const ran_t* ran_mgmt_get_ran_from_assoc_id(ran_mgmt_t* rm, int assoc_id);

/* returns true if RAN is "complete", i.e., all layers are referenced in the
 * RAN (e.g., CU-DU, not only CU). */
bool ran_mgmt_is_ran_formed(const ran_t* r);

char* generate_ran_name(uint64_t ran_key);

/* returns the entity_info_t* struct for a given RAN and assoc_id, and asserts
 * if not found */
entity_info_t* ran_mgmt_get_entity(const ran_t* ran, int assoc_id);

#endif /* RAN_MGMT_H */
