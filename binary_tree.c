#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "binary_tree.h"


/* Insert a new node into the binary search tree */
void insert(Binary_Tree *tree, int key, char *data) {

	BT_Node *new_node = create_bt_node(key, data);

	if (!tree->root) {
		// need to make root for tree
		tree->root = new_node;
	} else {
		do_insert(tree->root, new_node);
	}

}

/* Traverse tree recursively in order to place new
 * node in correct location as new leaf node.
 */
void do_insert(BT_Node *root, BT_Node *to_insert){

	// keep going until you find a leaf node
	if (to_insert->key == root->key) {
		// TODO: replace value, for now, then should implement as linked list
		root->data = to_insert->data;
	} else if (to_insert->key < root->key) {
		if (root->left_child) {
			do_insert(root->left_child, to_insert);
		} else {
			root->left_child = to_insert;
		}
	} else {
		if (root->right_child) {
			do_insert(root->right_child, to_insert);
		} else {
			root->right_child = to_insert;
		}
	}
}

/* Search to see if a node is in a tree */
BT_Node * search(Binary_Tree *tree, int key) {
	return do_search(tree->root, key);
}

/* Recursive helper function to do actual search */
BT_Node * do_search(BT_Node *root, int key) {

	if (root->key == key) {
		return root;
	} else if (root->key > key) {
		if (root->left_child != NULL) {
			return do_search(root->left_child, key);
		}
		return NULL;
	} else {
		if (root->right_child != NULL) {
			return do_search(root->right_child, key);
		}
		return NULL;
	}
}


/* Remove a node from tree */
void delete(Binary_Tree *tree, int key) {

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
		do_delete(trav, parent, is_right_child);
	}
}

void do_delete(BT_Node *to_delete, BT_Node *parent, int is_right_child) {

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
		do_delete(to_swap, trail, r_child);
	}
	free(to_delete);
}

void print_tree(Binary_Tree *tree, char *print_type) {

	if (tree->root == NULL) {
		printf("Tree is empty");
	}

	if (strcmp(print_type, "in_order_traversal") == 0) {
		in_order_print(tree->root);
    } else if (strcmp(print_type, "pre_order_traversal") == 0) {
        pre_order_print(tree->root);
    } else if (strcmp(print_type, "post_order_traversal") == 0) {
        post_order_print(tree->root);
    } else {
		printf("Print type %s not recognized", print_type);
	}
}


/* Preorder traversal of Tree */
void pre_order_print(BT_Node *root) {

	printf("( key: %d )\n", root->key);
    
    if (root->left_child != NULL) {
		in_order_print(root->left_child);
	}
	if (root->right_child != NULL) {
		in_order_print(root->right_child);
	}
}

/* In order traversal of Tree */
void in_order_print(BT_Node *root) {
	if (root->left_child != NULL) {
		in_order_print(root->left_child);
	}
	printf("( key: %d )\n", root->key);

	if (root->right_child != NULL) {
		in_order_print(root->right_child);
	}
}

/* Postorder traversal of Tree */
void post_order_print(BT_Node *root) {

    if (root->left_child != NULL) {
		in_order_print(root->left_child);
	}
	if (root->right_child != NULL) {
		in_order_print(root->right_child);
	}
	printf("( key: %d )\n", root->key);
}

/* Allocates memory and creates new node struct */
BT_Node * create_bt_node(int key, char *data) {

	BT_Node *node = (BT_Node *) malloc(sizeof(BT_Node));
	node->key = key;
	node->data = malloc(sizeof(char) * strlen(data));
	node->left_child = NULL;
	node->right_child = NULL;

	return node;
}

