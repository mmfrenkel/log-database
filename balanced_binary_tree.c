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

    if (!ttree->root){
        Group *root_group = create_group();
        root_group->left_child = new_node;
    } else {
        Group *root_group = insert(tttree, new_node);
    }
    tree->root = root_group;
}

/*Does the work of rearranging a tree to insert a new node */
Group * insert(TwoThreeTree tttree, Tree_Node *node){
    
    Group *root = tttree->root;
    Group *group = find_group(tttree->root, node);
    
    if (group->left_node && group->right_node) {
        root = sift_up_to_insert(group, node);
    } else if (group->left_node) {  
        /* left node only, as nodes are added to left side of group first */
        if (group->left_node->key == node->key) {
            continue;
        }
        else if (group->left_node->key < node->key) {
            group->right_node = node;
        }
        else {
            group->right_node = group->left_node;
            group->left_node = node;
        }
    }
    return root;
}

/* Finds the Group that the new Tree_Node would belong to */
Group * find_group(Group *root, Tree_Node *node){
    if (!root->left_node && !root->middle_node && !root->right_node) {
        return root;
    } else if (node->key < root->left_node->key) {
        return find_group(root->left_child, node);
    } else if (node->key < root->middle_node->key) { 
        return find_group(root->middle_child, node);
    } else {
        return find_group(root->right_child, node);
    }
}

Group * sift_up_to_insert(Group *group, Node *node) {
    Tree_Node *to_sift_up;
    Tree Node *left_node;
    Tree_Node *right_node;

    if (node->key > group->left_node->key && node->key < group->right_node->key) {
        to_sift_up = node;
        left_node = group->left_node;
        right_node = group->right_node;
    } else if (node->key < group->left_node->key){
        to_sift_up = group->left_node;
        left_node = node;
        right_node = group->right_node;
    } else {
        to_sift_up = group->right_node;
        right_node = node;
        left_node = group->left_node; 
    }

    if (group->parent_group) {
        Group *parent_group = group->parent_group;

        if (parent_group->middle_child != 0) { */then we know it's full */
            sift_up_to_insert(parent_group, to_sift_up);
        }  
        else /* there is only a left node */ {
            if (parent_group->right_child
        }
    } else { /* Then we're at the root node */
    }
}

/*Creates new Tree_Node */
Tree_Node * create_node(int key, char *value) {
    Tree_Node *new_node = (Tree_Node *) malloc(sizeof(Tree_Node));
    new_node->node_key = key;

    // Linked_List *llist_ptr = (Linked_List *) malloc(sizeof(Linked_List));
    // Linked_list llist = *llist_ptr;
    // llist.headptr = NULL;
    // llist.tailptr = NULL;
    // add_node_start(&llist, );
    
    strcpy(new_node->node_value, value);
    return new_node;
}

/*Creates new Group; Initialize Values to 0*/
Group * create_group() {
    Group *new_group = (Group *) malloc(sizeof(Group));
    new_group->left_node = 0;
    new_group->right_node = 0;
    new_group->left_child = 0;
    new_group->middle_child = 0;
    new_group->right_child = 0;
    new_group->parent = 0;
    return new_group;
}

