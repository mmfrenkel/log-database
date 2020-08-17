#include <stdio.h>
#include <binary_tree.h> 

#define MARKER -1
#define BUF_DATA_SIZE 50

void save_tree_to_file(Binary_Tree *tree, char *filename) {
	FILE *fp;
	fp = open(filename, "w");
	serialize_preorder(tree->root, fp);
	fclose(fp);
}

/* Writes an entire binary tree to file, preorder */
void serialize_preorder(BT_Node *root, FILE *fp) {
    if (root == NULL) {
        fprintf(fp, "%d,%s\n", MARKER, MARKER); 
        return; 
    } 
    
    fprintf(fp, "%d,%s\n", root->key, root->data);
    serialize(root->left_child);
    serialize(root->right_child);
}

/* Reads an entire binary tree to file, expects preorder layout */
BT_Node * deserialize_preorder(FILE *fp) {
    int key;
    char buf[BUF_DATA_SIZE]; 

    if (!fscanf(fp, "%d,%s", &key, buf) || key == MARKER) {
    	return NULL;
    }
    
    print("Read in: %d, %s", key, buf);
    BT_Node *root = create_bt_node(key, buf);
    root->left_child = deserialize(fp);
    root->right_child = deserialize(fp);

    return root;
}

/* Writes key, value pair to write ahead log */
void kv_to_wal(char *wal, int key, char *value) {
    char *to_write;
    sprintf(to_write, "%d,%s\n", key, value);
    add_to_file(wal, to_write);
}

/* Deletes contents of a file identified by filename */
void delete_file_contents(char *filename) {
    FILE *fp;
    fp = open(filename, "w");
    fclose(fp);
}

/* Append to file */
void add_to_file(char *filename, char *to_write) {
    
    FILE *fp;
    fp = fopen(filename, "a");

    fprintf(fp, to_write);
    fclose(fp);
}   


