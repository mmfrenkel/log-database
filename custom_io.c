#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "memtable.h"
#include "custom_io.h"
#include "error.h"

#define MARKER -1
#define MAX_LINE_SIZE 100
#define BUF_DATA_SIZE 50

/* Length is based on structure of filename below... time returns 10 digit number,
 * plus underscore and another 2 digits  + file suffix (3 char)  + \0 = 17 chars */
#define FILENAME_SIZE 20

/* prototypes for static functions */
static void serialize_preorder(MNode *root, FILE *fp);
static void add_to_file(char *filename, char *to_write);
static void inorder_to_file(MNode *root, FILE *fp);

/* Writes a memtable (binary tree) to file */
void serialize_memtable(Memtable *memtable, char *filename) {
	FILE *fp;
	fp = fopen(filename, "w");
	serialize_preorder(memtable->root, fp);
	fclose(fp);
}

/* Writes an entire memtable (binary tree) to file, preorder */
static void serialize_preorder(MNode *root, FILE *fp) {
	if (root == NULL) {
		fprintf(fp, "%d,%d\n", MARKER, MARKER);
		return;
	}

	fprintf(fp, "%d,%s\n", root->key, root->data);
	serialize_preorder(root->left_child, fp);
	serialize_preorder(root->right_child, fp);
}

/* Reads an entire memtable (binary tree) from file, expects preorder layout */
MNode* deserialize_preorder(FILE *fp) {
	int key;
	char buf[BUF_DATA_SIZE];

	if (!fscanf(fp, "%d,%s", &key, buf) || key == MARKER) {
		return NULL;
	}

	printf("Read in: %d, %s", key, buf);
	MNode *root = create_node(key, buf);
	root->left_child = deserialize_preorder(fp);
	root->right_child = deserialize_preorder(fp);

	return root;
}

/* Writes a Memtable to a Sorted Strings Table
 * (key-value pairs in which keys are in sorted order*/
char* memtable_to_sorted_strings_table(Memtable *memtable) {
	char *filename = (char*) malloc(sizeof(char) * FILENAME_SIZE);
	if (filename == NULL) {
		printf("Failed to allocate memory for filename\n");
		return NULL;
	}

	// time returns 10 digit number, plus underscore and another 2 digits
	// + file suffix (3 char)  + \0 = min 17 digits
	sprintf(filename, "%ld_%d.log", time(NULL), rand() % 10);

	FILE *fp;
	fp = fopen(filename, "w");
	inorder_to_file(memtable->root, fp);
	fclose(fp);

	return filename;
}

/* Takes a memtable (tree) root, traverses tree "inorder" in
 * order to add data, ordered by key */
static void inorder_to_file(MNode *root, FILE *fp) {
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

/* Reads line from an open file, placing it into the string pointed to
 * by 'line'; note that the new line character may be
 * removed, if specified. */
char* readline_from_file(char *line, int buf_size, FILE *fp, bool rm_newline) {
	fgets(line, buf_size, fp);

	if (rm_newline) {
		char *p;
		if ((p = strchr(line, '\n')) != NULL)
			*p = '\0';
	}
	return line;
}

/* Deletes contents of a file identified by filename,
 * but keeps the file itself */
void delete_file_contents(char *filename) {
	FILE *fp;
	fp = fopen(filename, "w");
	fclose(fp);
}

/* Permanently deletes entire file */
int delete_file(char *filename) {
	int del = remove(filename);
	if (del) {
		printf("Compacted segment file did not delete properly");
		return -1;
	}
	return 0;
}

