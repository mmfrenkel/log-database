
typedef struct binary_tree_node BT_Node;

typedef struct binary_tree {
	BT_Node *root;
} Binary_Tree;

void insert(Binary_Tree *tree, int key, char *data);

void insert_node(BT_Node root, BT_Node to_insert);

void delete_node(Binary_Tree *tree, int key);

void delete_node(BT_Node *to_delete, BT_Node *parent, int is_right_child);

BT_Node * create_node(int key, char *data);
