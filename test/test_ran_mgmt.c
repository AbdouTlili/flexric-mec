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
#include <stdint.h>
#include "ran_mgmt.h"

void test_insert_ran(void)
{
  plmn_t plmn = { .mcc = 10, .mnc = 15, .mnc_digit_len = 2 };
  const ngran_node_t type = ngran_gNB; 
  global_e2_node_id_t id = { .type = type, .plmn = plmn, .nb_id = 555 };

  const size_t len_rf = 1;
  ran_function_t ran_func_item[len_rf];
  memset(ran_func_item, 0, sizeof(ran_func_item));
  ran_func_item[0].id = 32;
  ran_func_item[0].rev = 0;
  const char* def = "This is the possible deficniotn";
  ran_func_item[0].def.buf = (uint8_t*) def;
  ran_func_item[0].def.len = strlen(def); 
  e2_node_component_config_update_t* comp_conf_update = NULL;
  const size_t len_ccu = 0;
  e2_setup_request_t e2_stp_req = {
    .id = id,
    .ran_func_item = ran_func_item ,
    .len_rf = len_rf, 
    .comp_conf_update = comp_conf_update,
    .len_ccu = len_ccu
  };

  ran_mgmt_t rm;
  ran_mgmt_init(&rm);
  int assoc_id = 1337;
  ran_mgmt_add_e2_node(&rm, assoc_id, &e2_stp_req);

  /* verify that we have one RAN, one e2node assoc */
  assert(1 == ran_mgmt_get_ran_num(&rm));
  assert(1 == ran_mgmt_get_e2_node_num(&rm));

  /* verify that the RAN ID is what we expect */
  ran_t* ran;
  const ssize_t num_rans = ran_mgmt_get_rans(&rm, &ran, 1);
  assert(1 == num_rans);
  assert(ran->generated_ran_key == 72233580322882091);
  char *ran_name = generate_ran_name(ran->generated_ran_key);
  assert(strcmp("nr_010_015_000022b", ran_name) == 0);
  free(ran_name);

  /* verify that the RAN obtained through assoc ID is the same as the one
   * we obtained from the first RAN in the list */
  const ran_t* r = ran_mgmt_get_ran_from_assoc_id(&rm, assoc_id);
  assert(r != NULL);
  assert(ran == r);

  /* remove the RAN and verify that everythis is 0/NULL */
  ran_mgmt_remove_e2_node(&rm, assoc_id);
  assert(0 == ran_mgmt_get_ran_num(&rm));
  assert(0 == ran_mgmt_get_e2_node_num(&rm));
  assert(ran_mgmt_get_ran_from_assoc_id(&rm, assoc_id) == NULL);

  ran_mgmt_free(&rm);
}

int main()
{
  test_insert_ran();
  /* TODO: test_insert_split_ran() */
}
