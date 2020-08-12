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
		/* There aren't any groups in the tree yet; must create it */
		Group *root_group = create_group();
		root_group->left_child = new_node;
		tttree->root = root_group;
	} else {
		tttree->root = insert(tttree, new_node);
	}
}


/*Does the work of rearranging a tree to insert a new node */
Group* insert(TwoThreeTree tttree, Tree_Node *node) {

	Group *root = tttree->root;
	Group *returned_root;

	/* find the leaf node that the element should belong to */
	Group *group = find_group(tttree->root, node);

	/* Case 1: Insert Node into Group with Only 1 Data Element */
	if (!group->right_node) {
		case_one(group, node);
	}
	else {
		/* Split the node, then reconstruct */
		Group *new_group = split_node(group, node);
		returned_root = push_up(group->parent, new_group);
	}

	if (returned_root->parent) {
		return tttree->root;
	} else {
		return returned_root;
	}
}


/* Finds the Group that the new Tree_Node would belong to */
Group * find_group(Group *root, Tree_Node *node) {
	if (!root->left_child && !root->middle_child && !root->right_child) {
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

/* Splits a full 3-group into a 2-group */
Group * split_node(Group *group, Tree_Node *new_node) {
	/* First find which node needs to sift up */
	Tree_Node *to_sift_up;
	Tree_Node *left_node;
	Tree_Node *right_node;

	if (new_node->key > group->left_node->key
			&& new_node->key < group->right_node->key) {

		/* then new node is middle value */
		to_sift_up = new_node;
		left_node = group->left_node;
		right_node = group->right_node;

	} else if (new_node->key < group->left_node->key) {
		/* left node is middle value */
		to_sift_up = group->left_node;
		left_node = new_node;
		right_node = group->right_node;

	} else {
		/* right node is middle value */
		to_sift_up = group->right_node;
		right_node = new_node;
		left_node = group->left_node;
	}

	Group *new_parent = create_group();
	Group *new_right_group = create_group();

	group->left_node = left_node;
	group->right_node = 0;
	group->parent = new_parent;

	new_right_group->left_node = right_node;
	new_right_group->parent = new_parent;

	return new_parent;
}

/* Takes a parent node and tries to add a 2-node to it */
Group * push_up(Group *parent_group, Group *new_group) {

	/* The parent is a 2-group (not full) */
	if (!parent_group->right_node) {

		/* New group contains keys larger than parent */
		if (parent_group->left_node < new_group->left_node) {
			parent_group->right_node = new_group->left_node;
			parent_group->middle_child = new_group->left_child;
			parent_group->right_child = new_group->right_child;
		} else {
			parent_group->right_node = parent_group->left_node;
			parent_group->right_node = new_group->left_node;
			parent_group->left_child = new_group->left_child;
			parent_group->middle_child = new_group->right_child;
		}
		/* Now get rid of old Group */
		free(new_group);
		return parent_group;
	}
	/* The parent is already a 3-group (full) */
	else {
		Group *split_group = split_three_node(parent_group, new_group);

		if (split_group->parent) {
			return push_up(split_group->parent, split_group);
		}
		return split_group;
	}
}

Group * split_three_node(Group *original_parent, Group *sift_group) {
	/* First find which node needs to sift up */
	Group *new_parent_group = create_new_group();

	if (sift_group->left_node->key > original_parent->left_node->key
			&& sift_group->left_node->key < original_parent->right_node->key) {

		/* the sift node is middle value */
		new_parent_group->left_node = original_parent->right_node;
		new_parent_group->right_child = original_parent->right_child;
		new_parent_group->left_child = sift_group->right_child;
		new_parent_group->parent = sift_group;

		original_parent->right_child = sift_group->left_child;
		original_parent->right_node = 0;
		original_parent->middle_child = 0;

		sift_group->right_child = new_parent_group;
		sift_group->left_child = original_parent;
		sift_group->parent = original_parent->parent;
		original_parent->parent = sift_group;

		return sift_group;

	} else if (sift_group->left_node->key < original_parent->left_node->key) {
		/* left node of parent group is middle value */
		new_parent_group->left_node = original_parent->right_node;
		new_parent_group->right_child = original_parent->right_child;
		new_parent_group->left_child = original_parent->middle_child;
		new_parent_group->parent = original_parent;

		original_parent->right_node = 0;
		original_parent->right_child = new_parent_group;
		original_parent->left_child = sift_group;
		original_parent->parent = new_parent_group;

		sift_group->parent = original_parent;

		return new_parent_group;

	} else {
		/* right node of parent is middle value */
		new_parent_group->left_node = original_parent->right_node;
		new_parent_group->left_child = original_parent;
		new_parent_group->right_child = sift_group;
		new_parent_group->parent = original_parent->parent;

		original_parent->right_child = original_parent->middle_child;
		original_parent->middle_child = 0;
		original_parent->parent = new_parent_group;

		sift_group->parent = new_parent_group;

		return new_parent_group;
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

