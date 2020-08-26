#ifndef BTREE_H
#define BTREE_H

#include <stdbool.h>

#define DEL_MARKER "*-*"   // special marker, denoting a key was deleted

typedef struct binary_tree_node {
	int key;
	char *data;
	struct binary_tree_node *left_child;
	struct binary_tree_node *right_child;
} BT_Node;

typedef struct binary_tree {
	BT_Node *root;
	int count_keys;
} Binary_Tree;

Binary_Tree* init_binary_tree();

int insert(Binary_Tree *tree, int key, char *data);

int delete(Binary_Tree *tree, int key, bool hard_delete);

BT_Node* create_bt_node(int key, char *data);

BT_Node* search(Binary_Tree *tree, int key);

void print_tree(Binary_Tree *tree, char *print_type);

void clear_tree(Binary_Tree *tree);

void delete_tree(Binary_Tree *tree);

#endif
