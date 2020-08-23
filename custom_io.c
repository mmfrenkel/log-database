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

char* compact(int num_segment_files, char **segment_files) {
	if (segment_files == NULL) {
		die("Segment files for compaction not provided\n");
	}
	if (num_segment_files < 2) {
		die("Need at least two segment files for compaction.\n");
	}

	char *segment_file1 = segment_files[0];
	char *new_segment, *segment_file2;
	for (int i = 1; i < num_segment_files; i++) {
		segment_file2 = segment_files[i];
		new_segment = do_compaction(segment_file1, segment_file2);
		delete_file(segment_file1);
		delete_file(segment_file2);
		segment_file1 = new_segment;
	}
	return new_segment;
}

/* Used to perform compaction step of two segment files. This method is
 * necessary for cleaning up old segment files and keeping read I/O from
 * getting out of control. Merges old segments together into new segments.*/
static char* do_compaction(char *segment_file1, char *segment_file2) {
	// open files for segments of interest
	FILE *segment_ptr1 = fopen(segment_file1, "r");
	FILE *segment_ptr2 = fopen(segment_file2, "r");

	// create a new segment file
	char *new_segment = (char*) malloc(sizeof(char) * strlen(segment_file1));
	sprintf(new_segment, "log_%ld.txt", time(NULL));
	FILE *new_fp = fopen(new_segment, "w");

	// setup for merge loop
	char line_seg1[MAX_LINE_SIZE];
	char line_seg2[MAX_LINE_SIZE];
	int keep_merging = 1;
	int incr_ptr1 = 1, incr_ptr2 = 1;
	char *read_result1 = NULL, *read_result2 = NULL;

	while (keep_merging) {
		if (incr_ptr1)
			read_result1 = fgets(line_seg1, MAX_LINE_SIZE, segment_ptr1);
		if (incr_ptr2)
			read_result2 = fgets(line_seg2, MAX_LINE_SIZE, segment_ptr2);

		if (read_result1 == NULL && read_result2 == NULL) {
			// both files are empty; so ready to be done
			keep_merging = 0;
		} else if (read_result1 == NULL || read_result2 == NULL) {
			// add any remaining keys in remaining file to new file
			FILE *temp_ptr = read_result1 != NULL ? segment_ptr1 : segment_ptr2;
			char *temp_line = read_result1 != NULL ? line_seg1 : line_seg2;
			do {
				fputs(temp_line, new_fp);
			} while (fgets(temp_line, MAX_LINE_SIZE, temp_ptr) != NULL);
			keep_merging = 0;
		} else {
			/* merge these two files; figure out which should go into the file 
			 * note that segmentfile 2 should be "younger" or "newer" than 
			 * segment file 1 so we use that for more up to date data */
			int key_1 = atoi(strtok(line_seg1, ","));
			int key_2 = atoi(strtok(line_seg2, ","));
			if (key_1 == key_2) {
				fputs(line_seg2, new_fp);
				incr_ptr1 = 1;
				incr_ptr2 = 1;
			} else if (key_1 > key_2) {
				fputs(line_seg2, new_fp);
				incr_ptr1 = 0;
				incr_ptr2 = 1;
			} else {  // key_2 > key_1
				fputs(line_seg1, new_fp);
				incr_ptr1 = 1;
				incr_ptr2 = 0;
			}
		}
	}
	// close fp for new file
	fclose(new_fp);
	fclose(segment_ptr1);
	fclose(segment_ptr2);
	return new_segment;
}

/* Writes a Tree to a Sorted Strings Table 
 * (key-value pairs in which keys are in sorted order*/
void tree_to_sorted_strings_table(Binary_Tree *tree, char *filename) {
	FILE *fp;
	fp = fopen(filename, "w");
	inorder_to_file(tree->root, fp);
	fclose(fp);
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

