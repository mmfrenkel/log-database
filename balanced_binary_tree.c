#include <stdio.h>
#include <string.h>


typedef struct tree_node {
	int key;
	char *node_value;
// Linked_List llist;
} Tree_Node;


typedef struct group {
	Tree_Node *left_node;
	Tree_Node *right_node;
	struct group *left_child;
	struct group *middle_child;
	struct group *right_child;
	struct group *parent;
} Group;


typedef struct two_three_tree {
	Group *root;
} TwoThreeTree;


/*Inserts new Tree_Node into 2-3 Tree*/
void insert_node(TwoThreeTree tttree, char *key, char *value) {
	Tree_Node *new_node = create_node(key, value);

	if (!tttree->root) {
		Group *root_group = create_group();
		root_group->left_child = new_node;
		tttree->root = root_group;
	} else {
		Group *root_group = insert(tttree, new_node);
	}
}


/*Does the work of rearranging a tree to insert a new node */
Group* insert(TwoThreeTree tttree, Tree_Node *node) {

	Group *root = tttree->root;
	Group *group = find_group(tttree->root, node);

	if (!group->right_node) {
		/* CASE 1: Insert Node into Group with Only 1 Data Element */
		/* left node is full, as nodes are added to left side of group first */
		case_one(group, node);
	} else if () {
		/* CASE 2: Insert Node into Group with 2 Nodes, but whose parent Group
		 * has only 1 Node. */
		case_two(group, node);
	}

	if (group->left_node && group->right_node) {
		/* This group is already full; need to sift around elements */
		root = sift_up_to_insert(group, node);
	} else if (group->left_node) {
	}
	return root;
}


/* Finds the Group that the new Tree_Node would belong to */
Group* find_group(Group *root, Tree_Node *node) {
	if (!root->left_child && !root->right_child) {
		/* we have reached a leaf; node will sift up from here */
		return root;
	} else if (node->key < root->left_node->key) {
		/* if root isn't empty, it will first have a left node */
		return find_group(root->left_child, node);
	} else if (node->key < root->right_node->key && root->right_child) {
		return find_group(root->middle_child, node);
	} else {
		return find_group(root->right_child, node);
	}
}


void case_one(Group *group, Tree_Node *node){

	if (group->left_node->key == node->key) {
		/* TO DO: will add to a linked list */
		continue;
	} else if (group->left_node->key < node->key) {
		group->right_node = node;
	} else {
		/* swap positions */
		group->right_node = group->left_node;
		group->left_node = node;
	}
}

void case_two(Group *group, Tree_Node *node) {
	/* First find which node needs to sift up */
	Tree_Node *to_sift_up;
	Tree_Node *left_node;
	Tree_Node *right_node;

	if (node->key > group->left_node->key
			&& node->key < group->right_node->key) {

		/* then new node is middle value */
		to_sift_up = node;
		left_node = group->left_node;
		right_node = group->right_node;

	} else if (node->key < group->left_node->key) {
		/* left node is middle value */
		to_sift_up = group->left_node;
		left_node = node;
		right_node = group->right_node;

	} else {
		/* right node is middle value */
		to_sift_up = group->right_node;
		right_node = node;
		left_node = group->left_node;
	}

	Group *parent = group->parent;

	group->left_node = left_node;
	group->right_node = 0;

	Group *new_right_group = create_group();
	new_right_group->left_node = right_node;
	new_right_group->parent = parent;

	if (parent->left_node->key < group->left_node->key) {
		/* sifting up from right branch */
		parent->middle_child = group;
		parent->right_node = to_sift_up;
		parent->right_child = new_right_group;
	} else {
		parent->middle_child = new_right_group;
		parent->right_node = parent->left_node;
		parent->left_node = to_sift_up;
	}
}

void case_three(Group *group, Tree_Node *node) {
	/* First find which node needs to sift up */
	Tree_Node *to_sift_up;
	Tree_Node *left_node;
	Tree_Node *right_node;

	if (node->key > group->left_node->key
			&& node->key < group->right_node->key) {

		/* then new node is middle value */
		to_sift_up = node;
		left_node = group->left_node;
		right_node = group->right_node;

	} else if (node->key < group->left_node->key) {
		/* left node is middle value */
		to_sift_up = group->left_node;
		left_node = node;
		right_node = group->right_node;

	} else {
		/* right node is middle value */
		to_sift_up = group->right_node;
		right_node = node;
		left_node = group->left_node;
	}

	Group *parent = group->parent;

	/* node to sift becomes parent for left and right node/groups */

	/* consider root condition */
	if (!group->parent) {

		/* edit the root group as to not loose pointer to root */
		Group *new_left_group = create_group();
		new_left_group->parent = group;
		new_left_group->left_node = left_node;
		new_left_group->left_child = group->left_child;
		new_left_group->right_child = group->middle_child;

		Group *new_right_group = create_group();
		new_right_group->parent = group;
		new_right_group->right_node = left_node;
		new_right_group->right_child = group->right_child;

		group->left_node = to_sift_up;
		group->right_node = 0;
		group->left_child = new_left_group;
		group->right_child = new_right_group;
	}
}

/*Creates new Tree_Node */
Tree_Node* create_node(int key, char *value) {
	Tree_Node *new_node = (Tree_Node*) malloc(sizeof(Tree_Node));
	new_node->key = key;

	// Linked_List *llist_ptr = (Linked_List *) malloc(sizeof(Linked_List));
	// Linked_list llist = *llist_ptr;
	// llist.headptr = NULL;
	// llist.tailptr = NULL;
	// add_node_start(&llist, );

	strcpy(new_node->node_value, value);
	return new_node;
}

/*Creates new Group; Initialize Values to 0*/
Group* create_group() {
	Group *new_group = (Group*) malloc(sizeof(Group));
	new_group->left_node = 0;
	new_group->right_node = 0;
	new_group->left_child = 0;
	new_group->middle_child = 0;
	new_group->right_child = 0;
	new_group->parent = 0;
	return new_group;
}

