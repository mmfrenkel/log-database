#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "memtable.h"
#include "error.h"

/* Prototypes for static functions for library */
static void do_insert(MNode *root, MNode *to_insert);
static MNode* do_search(MNode *root, int key);
static MNode* do_hard_delete(MNode *to_delete, MNode *parent,
		int is_right_child);
static void pre_order_print(MNode *root);
static void post_order_print(MNode *root);
static void in_order_print(MNode *root);
static void delete_memtable_nodes(MNode *root);

Memtable* init_memtable() {
	Memtable *memtable = (Memtable*) malloc(sizeof(Memtable));
	if (memtable == NULL) {
		printf("Allocation of memory for memtable failed.\n");
		return NULL;
	}

	// initialize values;
	memtable->count_keys = 0;
	memtable->root = NULL;
	return memtable;
}

/* Insert a new node into the binary search memtable.
 * Returns -1 if an error occurred, returns 0 if success.*/
int insert(Memtable *memtable, int key, char *data) {
	MNode *new_node = create_node(key, data);
	if (new_node == NULL) {
		return -1;  // failed to allocate memory for new node
	}

	if (!memtable->root) {
		// need to make root for memtable
		memtable->root = new_node;
	} else {
		do_insert(memtable->root, new_node);
	}
	return 0;
}

/* Traverse memtable recursively in order to place new
 * node in correct location as new leaf node. */
static void do_insert(MNode *root, MNode *to_insert) {

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

/* Search to see if a node is in a memtable */
MNode* search(Memtable *memtable, int key) {
	return do_search(memtable->root, key);
}

/* Recursive helper function to do actual search */
static MNode* do_search(MNode *root, int key) {

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

/* Remove a node from memtable. If hard_delete is specified,
 * then the entire node is removed from the memtable. If it is
 * a soft delete, then system replaces current value of node with specified
 * key with a "tombstone". Returns 0 if success, -1 if failure. */
int delete(Memtable *memtable, int key, bool hard_delete) {
	MNode *parent = NULL;
	MNode *trav = memtable->root;
	int is_right_child = 0;

	// first find the node to delete, if possible
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
		// didn't find the node to delete, so mark deletion by creating a
		// new node with delete marker;
		int error = insert(memtable, key, DEL_MARKER);
		if (error != 0)
			return -1;
		memtable->count_keys++;

	} else if (hard_delete) {
		printf("Found node with key %d, value: %s. Hard deleting...\n", key,
				trav->data);
		MNode *new_root = do_hard_delete(trav, parent, is_right_child);

		// if a new root was assigned, give it to the binary memtable
		if (new_root != NULL) {
			memtable->root = new_root;
		} else if (parent == NULL) {
			// no new root was assigned but we know the node deleted was the root
			printf("\n> LSM System Alert: Memtable is empty.\n");
			memtable->root = NULL;
		}
	} else { // soft delete, just change the value to the delete marker
		trav->data = DEL_MARKER;
	}
	return 0;
}

static MNode* do_hard_delete(MNode *to_delete, MNode *parent,
		int is_right_child) {

	MNode *new_root = NULL;

	/* Case 1: Node has no Children */
	if (to_delete->left_child == NULL && to_delete->right_child == NULL) {
		printf("Node has no children...\n");
		if (parent == NULL) {
			// the node to delete is the root; it has no children, continue
		} else if (is_right_child) {
			parent->right_child = NULL;
		} else {
			parent->left_child = NULL;
		}
	}
	/* Case 2: Node has 1 LEFT Child */
	else if (to_delete->left_child != NULL && to_delete->right_child == NULL) {
		printf("Node has 1 left child...\n");
		if (parent == NULL) {
			// root condition; send left child as new root node
			new_root = to_delete->left_child;
		} else if (is_right_child) {
			parent->right_child = to_delete->left_child;
		} else {
			parent->left_child = to_delete->left_child;
		}
	}
	/* Case 3: Node has 1 RIGHT Child */
	else if (to_delete->right_child != NULL && to_delete->left_child == NULL) {
		printf("Node has 1 right child...\n");
		if (parent == NULL) {
			// root condition; send left child as new root node
			new_root = to_delete->right_child;
		} else if (is_right_child) {
			parent->right_child = to_delete->right_child;
		} else {
			parent->left_child = to_delete->right_child;
		}
	}
	/* Case 4: Node has TWO Children */
	else {
		printf("Node has two children...\n");
		// Best option is to replace "to_delete" node with the smallest node
		// in the right subtree, then delete that node.
		MNode *to_swap = to_delete->right_child;
		MNode *trail = to_delete;
		int is_right_child = 1; // we go right first in our search for smallest node

		while (to_swap->left_child != NULL) {
			trail = to_swap;
			to_swap = to_swap->left_child;
			is_right_child = 0;
		}
		// replace contents
		to_delete->key = to_swap->key;
		to_delete->data = to_swap->data;

		// this node can then be deleted with either case 1 or 2, but we don't free
		// to_delete here, because it will happen on the next function call
		return do_hard_delete(to_swap, trail, is_right_child);
	}
	free(to_delete);
	return new_root;
}

void print_memtable(Memtable *memtable, char *print_type) {
	printf("\nCurrent Memtable:\n");

	if (memtable->root == NULL) {
		printf("memtable is empty\n");
		return;
	}

	if (strcmp(print_type, "in_order_traversal") == 0) {
		in_order_print(memtable->root);
	} else if (strcmp(print_type, "pre_order_traversal") == 0) {
		pre_order_print(memtable->root);
	} else if (strcmp(print_type, "post_order_traversal") == 0) {
		post_order_print(memtable->root);
	} else {
		printf("Print type %s not recognized", print_type);
	}
}

/* Preorder traversal of memtable */
static void pre_order_print(MNode *root) {
	printf("( key: %d , value: %s)\n", root->key, root->data);

	if (root->left_child != NULL) {
		in_order_print(root->left_child);
	}
	if (root->right_child != NULL) {
		in_order_print(root->right_child);
	}
}

/* In order traversal of memtable */
static void in_order_print(MNode *root) {
	if (root->left_child != NULL) {
		in_order_print(root->left_child);
	}
	printf("( key: %d, value: %s)\n", root->key, root->data);

	if (root->right_child != NULL) {
		in_order_print(root->right_child);
	}
}

/* Postorder traversal of memtable */
static void post_order_print(MNode *root) {

	if (root->left_child != NULL) {
		in_order_print(root->left_child);
	}
	if (root->right_child != NULL) {
		in_order_print(root->right_child);
	}
	printf("( key: %d, value: %s )\n", root->key, root->data);
}

/* Allocates memory and creates new node struct */
MNode* create_node(int key, char *data) {

	MNode *node = (MNode*) malloc(sizeof(MNode));
	if (node == NULL) {
		printf("Failed to allocate memory for new node.\n");
		return NULL;
	}

	node->key = key;
	node->data = malloc(sizeof(char) * strlen(data));
	if (node->data == NULL) {
		die("Failed to allocate memory for data within node.\n");
	}
	strcpy(node->data, data);
	node->left_child = NULL;
	node->right_child = NULL;
	return node;
}

/* Keeps memtable, but removes all nodes */
void clear_memtable(Memtable *memtable) {
	delete_memtable_nodes(memtable->root);
	memtable->root = NULL;
	memtable->count_keys = 0;
}

/* Deletes entire memtable from memory */
void delete_memtable(Memtable *memtable) {
	delete_memtable_nodes(memtable->root);
	free(memtable);
}

static void delete_memtable_nodes(MNode *root) {
	printf("Attempting to free memtable nodes\n");
	if (root == NULL)
		return;

	if (root->left_child != NULL) {
		delete_memtable_nodes(root->left_child);
	}
	if (root->right_child != NULL) {
		delete_memtable_nodes(root->right_child);
	}
	free(root->data);
	free(root);
}

