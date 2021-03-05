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
#include <string.h>

#include "hash_table.h"

typedef struct table_s {
  hentry_t* entries;
  size_t size;
  size_t filled;
} table_t;

/* I don't have the TAOCP book, so source is
 * https://stackoverflow.com/a/41537995 */
uint32_t knuth(hkey_t key, uint8_t p)
{
  assert(p <= 32);
  const uint32_t knuth = 2654435769;

  assert(key.len > 0 && key.len <= MAX_KEY_LEN);
  uint32_t x = 0;
  for (uint8_t i = 0; i < key.len; ++i)
    x ^= ((uint32_t) key.word[i]) << (16 * (i%2));

  return (x * knuth) >> (32 - p);
}

static
size_t get_size(uint8_t width)
{
  assert(width > 0 && width < 32);
  return 2 << (width - 1);
}

bool eq_hkey(hkey_t a, hkey_t b)
{
  return a.len == b.len && memcmp(a.word, b.word, sizeof(a.word)) == 0;
}

void hash_table_init(htable_t* ht, uint8_t width)
{
  assert(ht);
  const uint32_t num = get_size(width);

  ht->table = calloc(num, sizeof(*ht->table));
  assert(ht->table);
  ht->width = width;
  ht->filled = 0;
  for (size_t i = 0; i < num; ++i) {
    table_t* t = &ht->table[i];
    const size_t init_size = 4;
    t->entries = calloc(init_size, sizeof(*t->entries));
    assert(t->entries);
    t->size = init_size;
    t->filled = 0;
  }
}

void hash_table_destroy(htable_t* ht, void (*f)(hentry_t*))
{
  assert(ht);
  for (size_t i = 0; i < get_size(ht->width); ++i) {
    table_t* t = &ht->table[i];
    for (size_t j = 0; j < t->filled; ++j)
      f(&t->entries[j]);
    free(t->entries);
  }
  free(ht->table);
  ht->table = NULL;
  ht->width = 0;
  ht->filled = 0;
}

size_t hash_table_get_size(htable_t* ht)
{
  assert(ht);
  return get_size(ht->width);
}

hentry_t* hash_table_find(const htable_t* ht, hkey_t key)
{
  assert(ht && ht->width > 0);
  const uint32_t index = knuth(key, ht->width);
  table_t* t = &ht->table[index];
  for (size_t i = 0; i < t->filled; ++i) {
    hentry_t* e = &t->entries[i];
    if (eq_hkey(key, e->key))
      return e;
  }
  return NULL;
}

hentry_t* hash_table_insert(htable_t* ht, hkey_t key, void* data)
{
  assert(ht && ht->width > 0);
  const uint32_t index = knuth(key, ht->width);
  table_t* t = &ht->table[index];
  for (size_t i = 0; i < t->filled; ++i) {
    hentry_t* e = &t->entries[i];
    if (eq_hkey(key, e->key))
      return e;
  }
  assert(t->filled + 1 <= t->size && "too many key collisions, table growing not implemented");
  hentry_t* e = &t->entries[t->filled];
  e->key = key;
  e->data = data;
  t->filled += 1;
  ht->filled += 1;
  return e;
}

bool hash_table_remove(htable_t* ht, hkey_t key)
{
  assert(ht);
  const uint32_t index = knuth(key, ht->width);
  table_t* t = &ht->table[index];
  for (size_t i = 0; i < t->filled; ++i) {
    hentry_t* e = &t->entries[i];
    if (eq_hkey(key, e->key)) {
      ht->filled -= 1;
      t->filled -= 1;
      /* if found element was not last one, put the last one in its place */
      if (i < t->filled)
        t->entries[i] = t->entries[t->filled];
      return true;
    }
  }
  return false;
}

void hash_table_iterate(htable_t* ht, void (*f)(hentry_t*, void*), void* data)
{
  assert(ht);
  for (size_t i = 0; i < get_size(ht->width); ++i) {
    table_t* t = &ht->table[i];
    for (size_t j = 0; j < t->filled; ++j) {
      hentry_t* e = &t->entries[j];
      f(e, data);
    }
  }
}
