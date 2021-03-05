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

#ifndef CLRS_RB_TREE_H
#define CLRS_RB_TREE_H

/*
    RB tree implemented a la CLRS

    Red-Black trees have 5 properties:
    1- Every node is Red or Black
    2- The root is black
    3- Every leaf is black
    4- If a node is red, then both its children are black
    5- For each node, all simple paths from the node to descendant leaves contain the same number of black nodes 

    Lemma 1: a red-black tree with n internal nodes has a height at most 2lg(n+1)
*/

#include <stddef.h>

typedef enum color_ee
{
  BLACK,
  RED,
} color_e;

typedef struct node_s{
  void* key;
  struct node_s* parent;
  struct node_s* left;
  struct node_s* right;
  color_e color;
} node_t;

typedef struct rb_tree_s{
   node_t* dummy;
   node_t* root;
  int (*comp)(const void*, const void*);
  size_t size;
} rb_tree_t;

void init_rb_tree(rb_tree_t* , int (*func)(const void*, const void*)); 

void free_rb_tree(rb_tree_t* tree, void (*f)(void*));

void insert_rb_tree(rb_tree_t* tree, node_t* z_node);

void delete_rb_tree(rb_tree_t* tree, node_t* z_node);

node_t* find_rb_tree(rb_tree_t* tree, node_t* node, void* key);

// Quick and very dirty
void iterate_rb_tree(rb_tree_t* tree, node_t* node, void* ret, void (*f)(void*, void*));

node_t* create_node_rb_tree(rb_tree_t* tree, void* key);

void free_node_rb_tree(node_t* n);

#endif

