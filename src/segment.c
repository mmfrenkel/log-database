#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "segment.h"
#include "memtable.h"
#include "error.h"

/* prototypes for static functions */
static void inorder_to_file(MNode *root, FILE *fp); // @suppress("Unused function declaration")
static int merge_segments(char *filename_a, char *filename_b,
						  char *new_segment_name, int line_size, char *tombstone);
static void merge_lines(char *line_a, char *line_b, FILE *new_fp,
						bool *incr_a, bool *incr_b, int line_size, char *tombstone);
static char* do_search_segment(FILE *segment_ptr, int key, int line_size);


/* Creates an array of strings to hold segment file names */
char** init_segment_list(int num_segments) {
	char **segments = (char**) malloc(num_segments * sizeof(char*));
	if (segments == NULL) {
		printf("Allocation of memory for segment files failed.\n");
		return NULL;
	}

	for (int i = 0; i < num_segments; i++)
		*(segments + i) = NULL;
	return segments;
}
/* Frees memory associated with segment file names */
void free_segment_list(char **segments, int num_segments) {
	for (int i = 0; i < num_segments; i++) {
		if (*(segments + i) != NULL) {
			free(*(segments + i));
		}
	}
	free(segments);
}

/* Writes a Memtable to a Sorted Strings Table
 * (key-value pairs in which keys are in sorted order*/
int memtable_to_segment(Memtable *memtable, char *filename) {
	FILE *fp;
	if ((fp = fopen(filename, "w")) == NULL) {
		printf("Failed to save memtable to segment.\n");
		return -1;
	}
	inorder_to_file(memtable->root, fp);

	fclose(fp);
	return 0;
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

/* Takes a list of segment file names and compacts two at
 * a time, sequentially; deletes files no longer needed */
int compact_segments(char **segment_files, int num_segments,
					 char *new_segment_name, int line_size, char *tombstone) {

	char *segment_a = *(segment_files);
	char *segment_b;

	for (int i = 1; i < num_segments; i++) {
		segment_b = *(segment_files + i);

		// will merge segs a and b into new_segment
		int error = merge_segments(segment_a, segment_b, new_segment_name,
				line_size, tombstone);
		if (error) {
			printf("An error occurred on compacting segments.\n");
			return -1;
		}

		// if existing file segments fail to delete, this is big problem for disk space.
		if (delete_segment(segment_a) != 0
				|| delete_segment(segment_b) != 0) {
			printf("Failed to delete old segment files");
			delete_segment(new_segment_name);
			free(new_segment_name);
			return -1;
		}

		// new segment will then be merged with consecutive other files
		segment_a = new_segment_name;
	}
	return 0;
}

/* Public wrapper function for searching a file specified by filename */
char* search_segment(char *filename, int key, int line_size) {
	FILE *fp;
	if (!(fp = fopen(filename, "r"))) {
		printf("Could not open up segment: %s", filename);
		return NULL;
	}
	char *found = do_search_segment(fp, key, line_size);
	fclose(fp);

	return found;
}

/* Searches through a segment, if key in segment then return value;
 * uses recursion to look at last line of file first (note that files
 * are assumed to be small and can fit in stack memory. */
static char* do_search_segment(FILE *segment_ptr, int key, int line_size) {
	char line[line_size];

	if (fgets(line, line_size, segment_ptr) != NULL) {
		char *value = do_search_segment(segment_ptr, key, line_size);
		if (value != NULL) {
			return value;
		}

		// split line to get key value, then check if it's the key
		if (atoi(strtok(line, ",")) == key) {
			char *value = strtok(NULL, ",");

			// remove new line character from value before returning
			char *p;
			if ((p = strchr(value, '\n')) != NULL)
				*p = '\0';

			return value;
		}
	}
	return NULL;
}

/* Used to perform compaction step of two segment files. This method is
 * necessary for cleaning up old segment files and keeping read I/O from
 * getting out of control. Merges old segments together into new segments.*/
static int merge_segments(char *filename_a, char *filename_b,
						  char *new_segment_name, int line_size, char *tombstone) {

	FILE *seg_ptr_a;
	FILE *seg_ptr_b;
	FILE *new_fp;

	// open files for segments of interest
	if (!(seg_ptr_a = fopen(filename_a, "r"))) {
		printf("Could not open up segment: %s", filename_a);
		return -1;
	}
	if (!(seg_ptr_b = fopen(filename_b, "r"))) {
		printf("Could not open up segment: %s", filename_b);
		return -1;
	}
	if (!(new_fp = fopen(new_segment_name, "w"))) {
		printf("Could not open up new segment: %s", new_segment_name);
		return -1;
	}

	// setup for merge loop
	char line_a[line_size];
	char line_b[line_size];
	bool keep_merging = true;
	bool incr_ptr_a = true, incr_ptr_b = true;

	// run the merge loop
	while (keep_merging) {
		if (incr_ptr_a) {
			memset(line_a, '\0', line_size);
			readline_from_segment(line_a, line_size, seg_ptr_a, true);
		}
		if (incr_ptr_b) {
			memset(line_b, '\0', line_size);
			readline_from_segment(line_b, line_size, seg_ptr_b, true);
		}

		if (!strlen(line_a) && !strlen(line_b))
			keep_merging = false;

		else if (!strlen(line_a) || !strlen(line_b)) {
			// add any remaining keys in remaining file to new file
			FILE *temp_ptr = strlen(line_a) ? seg_ptr_a : seg_ptr_b;
			char *temp_line = strlen(line_a) ? line_a : line_b;
			do {
				fputs(temp_line, new_fp);
				fputs("\n", new_fp);
			} while (fgets(temp_line, line_size, temp_ptr) != NULL);

			keep_merging = false;

		} else
			merge_lines(line_a, line_b, new_fp, &incr_ptr_a, &incr_ptr_b,
					line_size, tombstone);
	}
	// fclose() returns 0 if closing file is successful
	if (fclose(new_fp) || fclose(seg_ptr_a) || fclose(seg_ptr_b)) {
		printf("Failed to close one or more of the compacting files.\n");
		return -1;
	}
	return 0;
}

/* Helper function for merging two files together by adding lines from either file a
 * or file b to the new file, then assigning values to determine whether the
 * calling function should increment a or b */
static void merge_lines(char *line_a, char *line_b, FILE *new_fp,
						bool *incr_a, bool *incr_b, int line_size, char *tombstone) {

	//  need to make copy of lines because strtok() alters char array
	char line_a_copy[line_size], line_b_copy[line_size];
	strcpy(line_a_copy, line_a);
	strcpy(line_b_copy, line_b);

	int key_a = atoi(strtok(line_a_copy, ","));
	char *value_a = strtok(NULL, ",");

	int key_b = atoi(strtok(line_b_copy, ","));
	char *value_b = strtok(NULL, ",");

	if (key_a == key_b) {
		if (strcmp(value_b, tombstone) != 0) {
			fputs(line_b, new_fp);
			fputs("\n", new_fp);
		}
		*incr_a = true;
		*incr_b = true;
	} else if (key_a > key_b) {
		if ((strcmp(value_b, tombstone) != 0)) {
			fputs(line_b, new_fp);
			fputs("\n", new_fp);
		}
		*incr_a = false;
		*incr_b = true;
	} else {  // key_a < key_b
		if (strcmp(value_a, tombstone) != 0) {
			fputs(line_a, new_fp);
			fputs("\n", new_fp);
		}
		*incr_a = true;
		*incr_b = false;
	}
}

