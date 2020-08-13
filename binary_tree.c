#include <stdio.h>
#include <string.h>
#include "binary_tree.h"


typedef struct binary_tree_node {
	int key;
	char *data;
	struct binary_tree_node left_child;
	struct binary_tree_node right_child;
} BT_Node;


/* Insert a new node into the binary search tree */
void insert(Binary_Tree *tree, int key, char *data) {

	BT_Node *new_node = create_node(key, data);

	if (!tree->root) {
		// need to make root for tree
		tree->root = new_node;
	} else {
		insert_node(tree->root, new_node);
	}

}

/* Traverse tree recursively in order to place new
 * node in correct location as new leaf node.
 */
void insert_node(BT_Node root, BT_Node to_insert){

	// keep going until you find a leaf node
	if (to_insert->key == root->key) {
		// replace value, for now
		root->data = to_insert->data;
	} else if (to_insert->key < root->key) {
		if (root->left_child) {
			insert_node (root->left_child, to_insert);
		} else {
			root->left_child = to_insert;
		}
	} else {
		if (root->right_child) {
			insert_node (root->right_child, to_insert);
		} else {
			root->right_child = to_insert;
		}
	}
}

/* Remove a node from tree */
void delete_node(Binary_Tree *tree, int key) {

	BT_Node *parent = NULL;
	BT_Node *trav = tree->root;
	int is_right_child = 0;

	while (trav != NULL && trav->key != key) {
		parent = trav;

		if (trav->key > key) {
			trav = trav->left_child;
			is_right_child = 0;
		} else {
			trav = trav->right_child;
			is_right_child = 1;
		}
	}

	if (trav == NULL) {
		// didn't find the node to delete
		printf("Node with key %d does not exist.", key);
	} else {
		printf("Found node with key %d, value: %s", key, trav->data);
		delete_node(trav, parent, is_right_child);
	}
}

void delete_node(BT_Node *to_delete, BT_Node *parent, int is_right_child) {

	/* Case 0: Node to be deleted is the root */
	if (to_delete == parent) {
	}
	/* Case 1: Node has no Children */
	else if (to_delete->left_child == NULL && to_delete->right_child == NULL) {
		if (is_right_child) {
			parent->right_child = NULL;
		} else {
			parent->left_child = NULL;
		}
	}
	/* Case 2: Node has 1 LEFT Child */
	else if (to_delete->left_child != NULL && to_delete->right_child != NULL) {
		if (is_right_child) {
			parent->right_child = to_delete->left_child;
		} else {
			parent->left_child = to_delete->left_child;
		}
	}
	/* Case 3: Node has 1 RIGHT Child */
	else if (to_delete->right_child != NULL && to_delete->left_child != NULL) {
		if (is_right_child) {
			parent->right_child = to_delete->right_child;
		} else {
			parent->left_child = to_delete->right_child;
		}
	}
	/* Case 4: Node has TWO Children */
	else {
		// Best option is to replace "to_delete" node with the smallest node
		// in the right subtree, then delete that node.
		BT_Node *to_swap = to_delete->right_child;
		BT_Node *trail = to_delete;
		int r_child = 1;  // we go right first

		while(to_swap->left_child != NULL) {
			trail = to_swap;
			to_swap = to_swap->left_child;
			r_child = 0;
		}
		// replace contents
		to_delete->key = to_swap->key;
		to_delete->data = to_swap->data;

		// this node can then be deleted with either case 1 or 2
		delete_node(to_swap, trail, r_child);
	}
	free(to_delete);
}


/* Allocates memory and creates new node struct */
BT_Node * create_node(int key, char *data) {

	BT_Node *node = malloc(sizeof(BT_Node));
	node->key = key;
	node->data = malloc(sizeof(char) * strlen(data));
	node->left_child = NULL;
	node->right_child = NULL;

	return node;
}

