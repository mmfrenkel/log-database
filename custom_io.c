#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "binary_tree.h"
#include "custom_io.h"
#include "error.h"

#define MARKER -1
#define MAX_LINE_SIZE 100
#define BUF_DATA_SIZE 50
#define FILENAME_SIZE 20       // file name size

/* prototypes for static functions */
static void serialize_preorder(BT_Node *root, FILE *fp);
static void add_to_file(char *filename, char *to_write);
static void inorder_to_file(BT_Node *root, FILE *fp);
static char* do_compaction(char *segment_file1, char *segment_file2);
static void delete_file(char *filename);

/* Writes a binary tree to file */
void serialize_tree(Binary_Tree *tree, char *filename) {
	FILE *fp;
	fp = fopen(filename, "w");
	serialize_preorder(tree->root, fp);
	fclose(fp);
}

/* Writes an entire binary tree to file, preorder */
static void serialize_preorder(BT_Node *root, FILE *fp) {
	if (root == NULL) {
		fprintf(fp, "%d,%d\n", MARKER, MARKER);
		return;
	}

	fprintf(fp, "%d,%s\n", root->key, root->data);
	serialize_preorder(root->left_child, fp);
	serialize_preorder(root->right_child, fp);
}

/* Reads an entire binary tree to file, expects preorder layout */
BT_Node* deserialize_preorder(FILE *fp) {
	int key;
	char buf[BUF_DATA_SIZE];

	if (!fscanf(fp, "%d,%s", &key, buf) || key == MARKER) {
		return NULL;
	}

	printf("Read in: %d, %s", key, buf);
	BT_Node *root = create_bt_node(key, buf);
	root->left_child = deserialize_preorder(fp);
	root->right_child = deserialize_preorder(fp);

	return root;
}

/* Writes a Tree to a Sorted Strings Table 
 * (key-value pairs in which keys are in sorted order*/
char* tree_to_sorted_strings_table(Binary_Tree *tree) {
	char *filename = (char*) malloc(sizeof(char));
	if (filename == NULL) {
		printf("Failed to allocate memory for filename\n");
		return NULL;
	}

	sprintf(filename, "%s_%ld.txt", "log", time(NULL));

	FILE *fp;
	fp = fopen(filename, "w");
	inorder_to_file(tree->root, fp);
	fclose(fp);

	return filename;
}

/* Takes a tree root, traverses tree "inorder" in order to add
 * tree data, ordered by key */
static void inorder_to_file(BT_Node *root, FILE *fp) {
	// we've traversed past a root
	if (root == NULL) {
		return;
	}
	inorder_to_file(root->left_child, fp);
	fprintf(fp, "%d,%s\n", root->key, root->data);
	inorder_to_file(root->right_child, fp);
}

/* Writes key, value pair to write ahead log */
void kv_to_wal(char *wal, int key, char *value) {
	char to_write[MAX_LINE_SIZE];
	sprintf(to_write, "%d,%s", key, value);
	add_to_file(wal, to_write);
}

/* Append to file */
static void add_to_file(char *filename, char *to_write) {
	FILE *fp;
	fp = fopen(filename, "a");

	fprintf(fp, "%s\n", to_write);
	fclose(fp);
}

/* Deletes contents of a file identified by filename,
 * but keeps the file itself */
void delete_file_contents(char *filename) {
	FILE *fp;
	fp = fopen(filename, "w");
	fclose(fp);
}

/* Permanently deletes entire file */
static void delete_file(char *filename) {
	int del = remove(filename);
	if (del)
		die("Compacted segment file did not delete properly");
}

