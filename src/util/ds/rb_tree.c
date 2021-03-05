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

#include "rb_tree.h"
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

static
node_t* init_node(void* key)
{
  node_t* n = malloc(sizeof(node_t));
  assert(n != NULL);
  n->key = key;
  n->parent = NULL;
  n->left = NULL;
  n->right = NULL;
  n->color = RED;
  return n;
}

node_t* create_node_rb_tree(rb_tree_t* tree, void* key)
{
  node_t* x = init_node(key);
  x->parent = tree->dummy;
  x->left = tree->dummy;
  x->right = tree->dummy;
  return x;
}

void free_node_rb_tree(node_t* n)
{
  assert(n != NULL);
  free(n);
}

void init_rb_tree(rb_tree_t* tree, int (*func)(const void*, const void*)) // not optimized for use in the stack...
{
  assert(tree != NULL);
  tree->dummy = init_node(NULL);
  tree->dummy->color = BLACK;
  tree->root = tree->dummy;
  tree->comp = func;
  tree->size = 0;
}

// depth first search
void iterate_rb_tree(rb_tree_t* tree, node_t* node, void* ret, void (*f)(void*, void*))
{
  assert(tree != NULL);
  assert(node != NULL);
  if(node != tree->dummy){
    iterate_rb_tree(tree, node->left, ret ,f);
    iterate_rb_tree(tree, node->right, ret ,f);
    f(ret, node->key);
  }
}

static
void depth_first_search_free(rb_tree_t* tree, node_t* node, void (*f)(void*))
{
  if(node != tree->dummy){
    depth_first_search_free(tree, node->left, f);
    depth_first_search_free(tree, node->right, f);
    if(f != NULL)
      f(node->key);
    free_node_rb_tree(node);
  }
}

void free_rb_tree(rb_tree_t* tree, void (*f)(void*))
{
  assert(tree);
  depth_first_search_free(tree, tree->root, f);
  free(tree->dummy);
  //free(tree);
}

static
void left_rotate(rb_tree_t* tree, node_t* x_node)
{
  assert(tree != NULL);
  assert(x_node != NULL);
  assert(x_node->right != tree->dummy);
  
  node_t* y_node = x_node->right;
  x_node->right = y_node->left;
  if(y_node->left != tree->dummy)
    y_node->left->parent = x_node;

  y_node->parent = x_node->parent;
  if(x_node->parent == tree->dummy)
    tree->root = y_node;
  else if(x_node == x_node->parent->left)
    x_node->parent->left = y_node;
  else
    x_node->parent->right = y_node;

  y_node->left = x_node;
  x_node->parent = y_node;
}

static
void right_rotate(rb_tree_t* tree, node_t* x_node)
{
  assert(tree != NULL);
  assert(x_node != NULL);
  assert(x_node->left != tree->dummy);

  node_t* y_node = x_node->left;
  x_node->left = y_node->right;
  if(y_node->right != tree->dummy)
    y_node->right->parent = x_node;

  y_node->parent = x_node->parent;

  if(x_node->parent == tree->dummy)
    tree->root = y_node;
  else if( x_node->parent->left == x_node)
    x_node->parent->left = y_node;
  else
    x_node->parent->right = y_node;

  y_node->right = x_node;
  x_node->parent = y_node;

}

static
void insert_fixup(rb_tree_t* tree, node_t* z_node)
{
  while(z_node->parent->color == RED){
    // z_node->parent.color == RED) which means:
    // 1- violating only property 4 of the red-black tree
    // 2- z_node->parent was not the root by property 2, root must be black
    // 3- z_node->parent->parent must exist and is Black, since Red-Red is illegal

    node_t* parent = z_node->parent;
    node_t* grand_parent = z_node->parent->parent;
    if(parent == grand_parent->left){
      node_t* uncle = grand_parent->right;
      // Case 1: uncle is Red, flip colors and continue up the tree
      if(uncle->color == RED){
        parent->color = BLACK;
        uncle->color = BLACK;
        grand_parent->color = RED;
        z_node = grand_parent;
      } else { // uncle is BLACK
      // Case 2: z_node at right
        if(z_node == parent->right){
          z_node = parent;
          left_rotate(tree,z_node);
          parent = z_node->parent; // the grand_parent is the same, so do not refresh
        } 
      // Case 3: z_node at left
        parent->color = BLACK;
        grand_parent->color = RED;
        right_rotate(tree, grand_parent);
      }
    } else {
      node_t* uncle = grand_parent->left; 
      if(uncle->color == RED){
        parent->color = BLACK;
        uncle->color = BLACK;
        grand_parent->color = RED;
        z_node = grand_parent;
      } else { // uncle->color == BLACK
        if(z_node == parent->left){
          z_node = parent;
          right_rotate(tree, z_node);
          parent = z_node->parent; // the grand_parent is the same, so do not refresh
        }
        parent->color = BLACK;
        grand_parent->color = RED;
        left_rotate(tree, grand_parent);
      }
    }
  }
  // Assure that property 2 of the red-black tree is maintained
  tree->root->color = BLACK;
}

void insert_rb_tree(rb_tree_t* tree, node_t* z_node)
{
  assert(z_node != tree->dummy);
  assert(z_node->left == tree->dummy);
  assert(z_node->right == tree->dummy);
  assert(z_node->color == RED);

  node_t* x_node = tree->root;
  node_t* y_node = tree->dummy;

  // Find leaf to insert
  while(x_node != tree->dummy){
    y_node = x_node;
    if(tree->comp(z_node->key, x_node->key) == 1)
      x_node = x_node->left;
    else
      x_node = x_node->right;
  }

  z_node->parent = y_node;

  if(y_node == tree->dummy)
    tree->root = z_node;
  else if(tree->comp(z_node->key, y_node->key) == 1)
      y_node->left = z_node;
  else
    y_node->right = z_node;
  insert_fixup(tree, z_node);

  tree->size++;
}

static
void delete_fixup(rb_tree_t* tree, node_t* x_node)
{
  while(x_node != tree->root && x_node->color == BLACK){
    if(x_node == x_node->parent->left){
      node_t* w_node = x_node->parent->right;
      // Case 1: x's sibling is RED
      if(w_node->color == RED){
        w_node->color = BLACK;
        x_node->parent->color = RED;
        left_rotate(tree, x_node->parent);
        w_node = x_node->parent->right;
      }
      assert(w_node->color == BLACK);
      // Case 2: x's siblings nodes are both black and w itself is black
      if(w_node->left->color == BLACK && w_node->right->color == BLACK){
        w_node->color = RED;
        // if we come from Case 1, this will terminate, as x_node->parent == RED
        // else, spread the problem to upper levels in the tree
        // (maybe left and right branch decompensated after removing
        // black node in the left branch).
        x_node = x_node->parent;        
      } else {
      // Case 3: x's sibling w is black w->left->color == RED && w->right->color == BLACK
        if(w_node->right->color == BLACK){
          assert(w_node->color == BLACK);
          assert(w_node->left->color == RED);
          w_node->left->color = BLACK; 
          w_node->color = RED;
          right_rotate(tree, w_node);
          w_node = x_node->parent->right;
        }
        // Case 4: x's sibling is black and w's right child is red
        assert(w_node->color == BLACK);
        assert(w_node->right->color == RED);
        w_node->color = x_node->parent->color;
        x_node->parent->color = BLACK;
        // since w_node->right->color == RED we insert a new black 
        // in the w_node path, changing the color
        w_node->right->color = BLACK;
        // the rotation adds one black in the x_node path and deletes
        // one in the w_node path. As a result after the function, the x_node path
        // gets one extra black in the w_node, and ends with the number of blacks
        // +1, -1
        left_rotate(tree, x_node->parent); 
        x_node = tree->root;
      }
    } else {
      node_t* w_node = x_node->parent->left;
      // Case 1: x' sibling is red
      if(w_node->color == RED){
        w_node->color = BLACK;
        x_node->parent->color = RED;
        right_rotate(tree, x_node->parent);
        w_node = x_node->parent->left;
      }
      assert(w_node->color == BLACK);
      // Case 2: x' siblings nodes are both black and w itself is black
      if(w_node->left->color == BLACK && w_node->right->color == BLACK){
          w_node->color = RED;
          x_node = x_node->parent;
      } else {
      // Case 3: x's sibling w is black w->left == RED && w->right == BLACK
        if(w_node->left->color == BLACK){
          assert(w_node->color == BLACK);
          assert(w_node->right->color == RED);
          w_node->right->color = BLACK;
          w_node->color = RED;
          left_rotate(tree, w_node);
          w_node = x_node->parent->left;
        }
        // Case 4: x's sibling is black and w's left child is red
        assert(w_node->color == BLACK);
        assert(w_node->left->color == RED);
        w_node->color = x_node->parent->color;
        x_node->parent->color = BLACK;
        w_node->left->color = BLACK;
        right_rotate(tree, x_node->parent);
        x_node = tree->root;
      } 
    }
  }
  x_node->color = BLACK;
}

static
void transplant(rb_tree_t* tree, node_t* u_node, node_t* v_node)
{
  assert(u_node != tree->dummy);
  
  if(u_node->parent == tree->dummy)
    tree->root = v_node;
  else if( u_node == u_node->parent->left)
    u_node->parent->left = v_node;
  else
    u_node->parent->right = v_node;

  v_node->parent = u_node->parent;
}

static
node_t* minimum(rb_tree_t* tree, node_t* x_node)
{
  while(x_node->left != tree->dummy)
    x_node = x_node->left;
  return x_node;
}

void delete_rb_tree(rb_tree_t* tree, node_t* z_node)
{
  assert(tree!= NULL);
  assert(z_node != NULL); // && z_node MUST be present in the tree. Use find function before deleting
  color_e original_color = z_node->color;
  node_t* x_node = z_node;
  node_t* y_node = z_node;
  if(z_node->left == tree->dummy){
    x_node = z_node->right;
    transplant(tree, z_node, z_node->right);
    // note that nobody points to z_node, even though z_node still points to parent and right
  } else if(z_node->right == tree->dummy){
    x_node = z_node->left;
    transplant(tree, z_node, z_node->left);
  } else {
    y_node = minimum(tree, z_node->right); 
    original_color = y_node->color;
    x_node = y_node->right;

    if(y_node->parent == z_node){
      x_node->parent = y_node; // necessary when x_node is the dummy node
    } else {
      transplant(tree, y_node, y_node->right); 
      y_node->right = z_node->right;
      y_node->right->parent = y_node; 
    }
    transplant(tree, z_node, y_node);
    y_node->left = z_node->left;
    y_node->left->parent = y_node;
    y_node->color = z_node->color;
  }
  free_node_rb_tree(z_node);
  if(original_color == BLACK)
    delete_fixup(tree, x_node);

  assert(tree->size != 0);
  tree->size--;
}

node_t* find_rb_tree(rb_tree_t* tree, node_t* node, void* key)
{
  assert(tree != NULL);
  assert(node != NULL);
  assert(key != NULL);

 while(node != tree->dummy){
    int rc = tree->comp(key, node->key); 
    if(rc == 1)
      node = node->left;
    else if (rc == -1)
      node = node->right;
    else{
      assert(rc == 0);
      break;
    }
  }
  return node;
}

