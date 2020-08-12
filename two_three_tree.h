typedef struct node Tree_Node;

typedef struct group Group;

typedef struct two_three_tree TwoThreeTree;

void insert_node(TwoThreeTree tttree, char *key, char *value);

static Group* insert(TwoThreeTree tttree, Tree_Node *node);

char * find_value(TwoThreeTree ttthree, int key);

static Tree_Node * search(Group *root, int key);

Group * find_group(Group *root, Tree_Node *node);

static void insert_case_one(Group *group, Tree_Node *node);

Group * split_node(Group *group, Tree_Node *new_node);

Group * push_up(Group *parent_group, Group *new_group);

Group * split_three_node(Group *original_parent, Group *sift_group);

Tree_Node* create_node(int key, char *value);

Group* create_group();
