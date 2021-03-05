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
#include <string.h>
#define __USE_GNU
#include <search.h>
#include <time.h>
#include <sys/time.h>

#include <hash_table.h>

static
void free_htable_entry(hentry_t* e)
{
  (void) e;
}

void test_insert_remove()
{
  htable_t ht;
  hash_table_init(&ht, 8);
  int some_data = 12;
  hkey_t key = { .word = {1, 2, 3, 4, 5}, .len = 5 };
  assert(NULL == hash_table_find(&ht, key));
  const hentry_t* e = hash_table_insert(&ht, key, &some_data);
  assert(e);
  assert(e->data == &some_data);
  assert(*((int*)e->data) == some_data);
  assert(eq_hkey(e->key, key));
  assert(ht.filled == 1);
  const hentry_t* f = hash_table_find(&ht, key);
  assert(e == f);
  void iter_func(hentry_t* e, void* data) {
    assert(!data);
    assert(eq_hkey(e->key, key));
    assert(*((int*)e->data) == some_data);
  }
  hash_table_iterate(&ht, iter_func, NULL);
  const bool ret = hash_table_remove(&ht, key);
  assert(ret);
  assert(ht.filled == 0);
  hash_table_destroy(&ht, free_htable_entry);
}

void find_collisions(hkey_t key, uint8_t width)
{
  /* Try to find a collision, then add multiple keys and verify the correct one
   * is inserted/removed */
  assert(key.len == 5);
  const uint32_t hash = knuth(key, width);
  printf("base key %d,%d,%d,%d,%d width %d hash %u\n",
         key.word[0], key.word[1], key.word[2],
         key.word[3], key.word[4], width, hash);
  for (uint16_t i = 0; i < UINT16_MAX; ++i) {
    for (uint16_t j = 0; j < UINT16_MAX; ++j) {
      for (uint16_t k = 0; k < UINT16_MAX; ++k) {
        for (uint16_t l = 0; l < UINT16_MAX; ++l) {
          for (uint16_t m = 0; m < UINT16_MAX; ++m) {
            const hkey_t nkey = { .word = {i, j, k, l, m}, .len = 5 };
            const uint32_t nhash = knuth(nkey, width);
            if (hash == nhash)
              printf("collision key %d,%d,%d,%d,%d width %d hash %u\n",
                     nkey.word[0], nkey.word[1], nkey.word[2],
                     nkey.word[3], nkey.word[4], width, nhash);
          }
        }
      }
    }
  }
}

void test_insert_remove_collision()
{
  const uint8_t width = 8;
  hkey_t k[3] = {
    { .word = { 0, 1, 2, 3, 4 }, .len = 5 },
    { .word = { 0, 0, 0, 0, 103 }, .len = 5 },
    { .word = { 0, 0, 0, 0, 1090 }, .len = 5 }
  };
  assert(knuth(k[0], width) == knuth(k[1], width));
  assert(knuth(k[0], width) == knuth(k[2], width));

  htable_t ht;
  hash_table_init(&ht, width);
  hash_table_insert(&ht, k[0], NULL);
  hash_table_insert(&ht, k[1], NULL);
  hash_table_insert(&ht, k[2], NULL);

  void iter_func(hentry_t* e, void* data) {
    int* count = (int*)data;
    assert(eq_hkey(e->key, k[*count]));
    *count += 1;
  }
  int count = 0;
  hash_table_iterate(&ht, iter_func, &count);

  /* reinsertion leaves old entry intact */
  const int some_data = 12;
  const hentry_t* ag = hash_table_insert(&ht, k[0], (void*)&some_data);
  assert(!ag->data);
  assert(ag->data != &some_data);

  assert(ht.filled == 3);

  const hentry_t* f = hash_table_find(&ht, k[0]);
  assert(f && eq_hkey(f->key, k[0]));

  const bool r2 = hash_table_remove(&ht, k[1]);
  assert(r2);
  assert(ht.filled == 2);

  const bool r1 = hash_table_remove(&ht, k[0]);
  assert(r1);
  assert(ht.filled == 1);

  /* check that it really has been removed */
  const hentry_t* e_ag = hash_table_insert(&ht, k[0], (void*)&some_data);
  assert(e_ag->data == &some_data);
  assert(ht.filled == 2);

  hash_table_destroy(&ht, free_htable_entry);
}

void print_hashes()
{
  const uint8_t width = 8;
  bool collision[2 << 8];
  memset(collision, 0, sizeof(collision));
  for (uint16_t i = 0; i < 100; ++i) {
    hkey_t key = { .word = { 0, 0, 0, 0, i }, .len = 5 };
    const uint32_t hash = knuth(key, width);
    printf("key %d,%d,%d,%d,%3d width %d hash %3u collision %d\n",
           key.word[0], key.word[1], key.word[2],
           key.word[3], key.word[4], width, hash, collision[hash]);
    collision[hash] = true;
  }
}

typedef struct {
  int assoc_id;
  uint16_t ric_req_id;
  uint16_t ric_inst_id;
  uint16_t ran_func_id;
} raw_key_t;

hkey_t generate_hash_map_key(raw_key_t rk)
{
  const hkey_t k = {
    .word = {
      rk.assoc_id >> 16,
      rk.assoc_id & 0xffff,
      rk.ric_req_id,
      rk.ric_inst_id,
      rk.ran_func_id
    },
    .len = 5
  };
  return k;
}

void time_hash_map_insert(htable_t* ht, raw_key_t* rk, uint16_t num_raw_keys)
{
  for (uint16_t i = 0; i < num_raw_keys; ++i) {
    hkey_t k = generate_hash_map_key(rk[i]);
    hash_table_insert(ht, k, NULL);
  }
}

void time_hash_map_lookup(htable_t* ht, raw_key_t* rk, uint16_t num_raw_keys, uint32_t lookups)
{
  const uint32_t inc = 7919;
  uint32_t index = 1;
  for (uint32_t i = 0; i < lookups; ++i) {
    hkey_t k = generate_hash_map_key(rk[index]);
    const hentry_t* f = hash_table_find(ht, k);
    uint8_t b = ((uint8_t*) f->key.word) [0];
    (void) b;
    assert(!f->data);
    index = (index + inc) % num_raw_keys;
  }
}

void generate_hsearch_key(raw_key_t rk, char* key, size_t key_len)
{
  /* DIRTY: because we use hsearch, we need keys in the form of strings. This
   * function does this before we go towards our own hash map implementation.
   * We have: 32 bits assoc_id, 16 bits RIC req ID, 16 bits RIC inst ID, 16
   * bits RAN func ID. max width of a 16 bits number is 4 chars (in hex), so we
   * need a string of 5*4+1 length */
  assert(key_len == 21);
  assert(key);
  int num;
  num = (rk.assoc_id >>    28) & 0xf; key[ 0] = num < 10 ? '0' + num : 'a' + num - 10;
  num = (rk.assoc_id >>    24) & 0xf; key[ 1] = num < 10 ? '0' + num : 'a' + num - 10;
  num = (rk.assoc_id >>    20) & 0xf; key[ 2] = num < 10 ? '0' + num : 'a' + num - 10;
  num = (rk.assoc_id >>    16) & 0xf; key[ 3] = num < 10 ? '0' + num : 'a' + num - 10;
  num = (rk.assoc_id >>    12) & 0xf; key[ 4] = num < 10 ? '0' + num : 'a' + num - 10;
  num = (rk.assoc_id >>     8) & 0xf; key[ 5] = num < 10 ? '0' + num : 'a' + num - 10;
  num = (rk.assoc_id >>     4) & 0xf; key[ 6] = num < 10 ? '0' + num : 'a' + num - 10;
  num = (rk.assoc_id >>     0) & 0xf; key[ 7] = num < 10 ? '0' + num : 'a' + num - 10;
  num = (rk.ric_req_id >>  12) & 0xf; key[ 8] = num < 10 ? '0' + num : 'a' + num - 10;
  num = (rk.ric_req_id >>   8) & 0xf; key[ 9] = num < 10 ? '0' + num : 'a' + num - 10;
  num = (rk.ric_req_id >>   4) & 0xf; key[10] = num < 10 ? '0' + num : 'a' + num - 10;
  num = (rk.ric_req_id >>   0) & 0xf; key[11] = num < 10 ? '0' + num : 'a' + num - 10;
  num = (rk.ric_inst_id >> 12) & 0xf; key[12] = num < 10 ? '0' + num : 'a' + num - 10;
  num = (rk.ric_inst_id >>  8) & 0xf; key[13] = num < 10 ? '0' + num : 'a' + num - 10;
  num = (rk.ric_inst_id >>  4) & 0xf; key[14] = num < 10 ? '0' + num : 'a' + num - 10;
  num = (rk.ric_inst_id >>  0) & 0xf; key[15] = num < 10 ? '0' + num : 'a' + num - 10;
  num = (rk.ran_func_id >> 12) & 0xf; key[16] = num < 10 ? '0' + num : 'a' + num - 10;
  num = (rk.ran_func_id >>  8) & 0xf; key[17] = num < 10 ? '0' + num : 'a' + num - 10;
  num = (rk.ran_func_id >>  4) & 0xf; key[18] = num < 10 ? '0' + num : 'a' + num - 10;
  num = (rk.ran_func_id >>  0) & 0xf; key[19] = num < 10 ? '0' + num : 'a' + num - 10;
                                      key[20] = 0;
}
void time_hsearch_insert(struct hsearch_data* hsd, raw_key_t* rk, uint16_t num_raw_keys)
{
  for (uint16_t i = 0; i < num_raw_keys; ++i) {
    char* key = malloc(21);
    assert(key);
    generate_hsearch_key(rk[i], key, 21);
    ENTRY* ret;
    ENTRY e = { .key = key, .data = NULL };
    hsearch_r(e, ENTER, &ret, hsd);
  }
}
void time_hsearch_lookup(struct hsearch_data* hsd, raw_key_t* rk, uint16_t num_raw_keys, uint32_t num_lookups)
{
  const uint32_t inc = 7919;
  uint32_t index = 1;
  for (uint32_t i = 0; i < num_lookups; ++i) {
    char key[21];
    generate_hsearch_key(rk[index], key, 21);
    ENTRY e = { .key = key, .data = NULL };
    ENTRY* f;
    hsearch_r(e, FIND, &f, hsd);
    uint8_t b = ((uint8_t*) f->key) [0];
    (void) b;
    assert(!f->data);
    index = (index + inc) % num_raw_keys;
  }
}
void hsearch_free(struct hsearch_data* hsd, raw_key_t* rk, uint16_t num_raw_keys)
{
  for (uint16_t i = 0; i < num_raw_keys; ++i) {
    char key[21];
    generate_hsearch_key(rk[i], key, 21);
    ENTRY e = { .key = key, .data = NULL };
    ENTRY* f;
    hsearch_r(e, FIND, &f, hsd);
    free(f->key);
  }
}

uint32_t get_usec_diff(struct timeval start, struct timeval end)
{
  uint32_t start_usec = start.tv_sec * 1000000 + start.tv_usec;
  uint32_t end_usec = end.tv_sec * 1000000 + end.tv_usec;
  return end_usec - start_usec;
}

void compare_hash_map_hsearch()
{
  const uint16_t width = 8;
  const uint32_t size = 2 << width;
  htable_t ht;
  hash_table_init(&ht, width);
  struct hsearch_data hsearch_tab;
  memset(&hsearch_tab, 0, sizeof(hsearch_tab));
  hcreate_r(size, &hsearch_tab);

  const unsigned int seed = 0;//time(NULL);
  printf("seed is %d\n", seed);
  srand(seed);
  const uint16_t num_raw_keys = 100;
  raw_key_t rk[num_raw_keys];
  for (uint16_t i = 0; i < num_raw_keys; ++i) {
    rk[i] = (raw_key_t) {
      .assoc_id = rand() % UINT32_MAX,
      .ric_req_id = rand() % UINT16_MAX,
      .ric_inst_id = rand() % UINT16_MAX,
      .ran_func_id = rand() % UINT16_MAX
    };
  }

  const uint32_t num_lookups = 1000000;
  struct timeval start, end;
  memset(&start, 0, sizeof(start));
  memset(&end, 0, sizeof(end));
  gettimeofday(&start, NULL);
  time_hash_map_insert(&ht, rk, num_raw_keys);
  gettimeofday(&end, NULL);
  const uint32_t usec_hash_map_insert = get_usec_diff(start, end);
  printf("time_hash_map_insert(): %d us\n", usec_hash_map_insert);

  gettimeofday(&start, NULL);
  time_hash_map_lookup(&ht, rk, num_raw_keys, num_lookups);
  gettimeofday(&end, NULL);
  const uint32_t usec_hash_map_lookup = get_usec_diff(start, end);
  printf("time_hash_map_lookup(): %d us\n", usec_hash_map_lookup);

  gettimeofday(&start, NULL);
  time_hsearch_insert(&hsearch_tab, rk, num_raw_keys);
  gettimeofday(&end, NULL);
  const uint32_t usec_hsearch_insert = get_usec_diff(start, end);
  printf("time_hsearch_insert(): %d us\n", usec_hsearch_insert);

  gettimeofday(&start, NULL);
  time_hsearch_lookup(&hsearch_tab, rk, num_raw_keys, num_lookups);
  gettimeofday(&end, NULL);
  const uint32_t usec_hsearch_lookup = get_usec_diff(start, end);
  printf("time_hsearch_lookup(): %d us\n", usec_hsearch_lookup);
  assert(usec_hash_map_insert < usec_hsearch_insert && "insert of hsearch is faster than own implementation");
  assert(usec_hash_map_lookup < usec_hsearch_lookup && "lookup of hsearch is faster than own implementation");

  hsearch_free(&hsearch_tab, rk, num_raw_keys);
  hdestroy_r(&hsearch_tab);
  hash_table_destroy(&ht, free_htable_entry);
}

int main()
{
  //printf("sizeof hentry_t %ld, sizeof hentry_t* %ld\n", sizeof(hentry_t), sizeof(hentry_t*));
  //printf("sizeof hkey_t %ld\n", sizeof(hkey_t));
  test_insert_remove();
  test_insert_remove_collision();

  /*
  hkey_t base_key = { .word = { 0, 1, 2, 3, 4 }, .len = 5 };
  find_collisions(base_key, 8);
  print_hashes();
  */

  /* this comparison is super unfair, as we need to convert to char* to use
   * hsearch. However, the point is to be sure that I did not make a mistake
   * and our performance would actually be worse... */
  compare_hash_map_hsearch();
}
