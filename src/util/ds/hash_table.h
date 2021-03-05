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

#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_KEY_LEN 5

typedef struct hkey_s {
  uint16_t word[MAX_KEY_LEN];
  uint8_t len;
} hkey_t;
bool eq_hkey(hkey_t a, hkey_t b);

typedef struct entry_s {
  hkey_t key;
  void* data;
} hentry_t;

struct table_s;
typedef struct htable_s {
  struct table_s* table;
  uint8_t width;
  size_t filled;
} htable_t;

void hash_table_init(htable_t* ht, uint8_t width);
void hash_table_destroy(htable_t* ht, void (*f)(hentry_t*));

size_t hash_table_get_size(htable_t* ht);

/* Returns pointer to hash table entry. Returns NULL if no such key exists */
hentry_t* hash_table_find(const htable_t* ht, hkey_t key);
/* Inserts new data into hash table and returns pointer to that element.  If
 * such key exists, returns the existing entry (the user might overwrite the
 * data). */
hentry_t* hash_table_insert(htable_t* ht, hkey_t key, void* data);
/* Removes the hash table entry. Returns bool if the key has been removed */
bool hash_table_remove(htable_t* ht, hkey_t key);

void hash_table_iterate(htable_t* ht, void (*f)(hentry_t*, void*), void* data);

/* the used hashing algorithm */
uint32_t knuth(hkey_t key, uint8_t p);

#endif /* HASH_TABLE_H */
