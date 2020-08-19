#ifndef BTREE_H
#define BTREE_H

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

void insert(Binary_Tree *tree, int key, char *data);

void do_insert(BT_Node *root, BT_Node *to_insert);

void delete(Binary_Tree *tree, int key);

BT_Node* do_delete(BT_Node *to_delete, BT_Node *parent, int is_right_child);

BT_Node* create_bt_node(int key, char *data);

BT_Node* search(Binary_Tree *tree, int key);

BT_Node* do_search(BT_Node *root, int key);

void post_order_print(BT_Node *root);

void pre_order_print(BT_Node *root);

void in_order_print(BT_Node *root);

void print_tree(Binary_Tree *tree, char *print_type);

void delete_tree(Binary_Tree *tree);

void delete_btree_nodes(BT_Node *root);

#endif
