#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "segment.h"
#include "memtable.h"
#include "error.h"

/* Length is based on structure of filename below... time returns 10 digit number,
 * plus underscore and another 2 digits  + file suffix (3 char)  + \0 = 17 chars */
#define FILENAME_SIZE 20

/* prototypes for static functions */
static void inorder_to_file(MNode *root, FILE *fp);


/* Writes a Memtable to a Sorted Strings Table
 * (key-value pairs in which keys are in sorted order*/
char* memtable_to_segment(Memtable *memtable) {
	char *filename = (char*) malloc(sizeof(char) * FILENAME_SIZE);
	if (filename == NULL) {
		printf("Failed to allocate memory for filename\n");
		return NULL;
	}

	// time returns 10 digit number, plus underscore and another 2 digits
	// + file suffix (3 char)  + \0 = min 17 digits
	sprintf(filename, "%ld_%d.log", time(NULL), rand() % 10);

	FILE *fp;
	if ((fp = fopen(filename, "w")) == NULL) {
		printf("Failed to save memtable to segment.\n");
		return NULL;
	}
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

/* Reads line from an open file, placing it into the string pointed to
 * by 'line'; note that the new line character may be  removed, if specified. */
char* readline_from_segment(char *line, int buf_size, FILE *fp, bool rm_newline) {
	fgets(line, buf_size, fp);

	if (rm_newline) {
		char *p;
		if ((p = strchr(line, '\n')) != NULL)
			*p = '\0';
	}
	return line;
}

/* Permanently deletes entire file */
int delete_segment(char *filename) {
	int del = remove(filename);
	if (del) {
		printf("Compacted segment file did not delete properly");
		return -1;
	}
	return 0;
}

