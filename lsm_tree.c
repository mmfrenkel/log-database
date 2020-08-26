#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include "lsm_tree.h"
#include "binary_tree.h"
#include "custom_io.h"

// prototypes for static functions here
static char** init_segments();
static char* compact(int num_segment_files, char **segment_files);
static char* merge_files(char *segment_file1, char *segment_file2);
static char* search_segments(char **segment_files, int full_segments, int key);
static char* search_segment_file(FILE *segment_ptr, int key);
static void free_segments(char **segments);

/* Creates an LSM Tree for the program to use, initializing
 * everything properly */
LSM_Tree* init_lsm_tree() {
	LSM_Tree *lsm_tree = (LSM_Tree*) malloc(sizeof(LSM_Tree));
	if (lsm_tree == NULL) {
		printf("Allocation of memory for LSM Tree failed.\n");
		return NULL;
	}

	Binary_Tree *memtable = init_binary_tree();
	if (memtable == NULL) {
		free(lsm_tree);
		return NULL;
	}

	char **segments = init_segments();
	if (segments == NULL) {
		free(lsm_tree);
		free(memtable);
		return NULL;
	}

	lsm_tree->memtable = memtable;
	lsm_tree->segments = segments;
	lsm_tree->full_segments = 0;
	return lsm_tree;
}

/* Handles the submission provided by user; returns 0 if all succeeds,
 * otherwise returns -1.  */
int handle_submission(LSM_Tree *lsm_tree, Submission *submission) {

	printf("\n> LSM System Alert: Memtable currently holds %d keys, File system holds %d segments.\n",
			lsm_tree->memtable->count_keys, lsm_tree->full_segments);

	if (submission->action == ADD) {
		if (insert(lsm_tree->memtable, submission->key, submission->value) == 0) {
			lsm_tree->memtable->count_keys++;
		} else {
			printf("Insertion of new node failed.\n");
			return -1;
		}

	} else if (submission->action == SEARCH) {
		// search memtable first
		BT_Node *node = search(lsm_tree->memtable, submission->key);
		char *value;
		if (node == NULL) {
			// value wasn't in memtable, so look through the log files
			value = search_segments(lsm_tree->segments, lsm_tree->full_segments,
					submission->key);
		} else {
			value = node->data;
		}

		if (value != NULL) {
			printf("Value for key %d is %s", submission->key, value);
		} else {
			printf("Key not found in LSM Tree system.\n");
		}

	} else if (submission->action == DELETE) {
		// always soft delete here; do not decrement keys in tree
		if (delete(lsm_tree->memtable, submission->key, false) == 0) {
			lsm_tree->memtable->count_keys--;
		} else {
			printf("Deletion of key, value pair with key %d failed.\n",
					submission->key);
			return -1;
		}

	} else if (submission->action == SAVE) {
		serialize_tree(lsm_tree->memtable, "binary_tree.txt");

	} else if (submission->action == PRINT_MEMTABLE) {
		print_tree(lsm_tree->memtable, "in_order_traversal");

	} else {
		printf("Could not process request in submission.\n");
		return -1;
	}

	// this should be in the background, rather than sequential in future
	return run_compaction(lsm_tree);
}

int run_compaction(LSM_Tree *lsm_tree) {

	// Send keys, values in memtable to new segment
	if (lsm_tree->memtable->count_keys == MAX_KEYS_IN_TREE) {

		// Compact existing files, if necessary, to produce 1 segment file
		if (lsm_tree->full_segments == MAX_SEGMENTS) {
			char *new_segment = compact(MAX_SEGMENTS, lsm_tree->segments);
			if (new_segment == NULL) {
				printf("Compaction step failed!\n");
				return -1;
			}

			// assign new segment and clear existing segments
			*(lsm_tree->segments) = new_segment;
			for (int i = 1; i < MAX_SEGMENTS; i++) {
				free(*(lsm_tree->segments + i));
				*(lsm_tree->segments + i) = NULL;
			}
			lsm_tree->full_segments = 1;
		}

		// now send memtable to segment
		char *filename = tree_to_sorted_strings_table(lsm_tree->memtable);
		if (filename == NULL) {
			printf("Failed at saving memtable to segment.\n");
			return -1;
		}

		*(lsm_tree->segments + lsm_tree->full_segments) = filename;
		lsm_tree->full_segments += 1;
		clear_tree(lsm_tree->memtable);
	}
	return 0;
}

/* Call to deallocate all memory for LSM tree system*/
void shutdown_lsm_system(LSM_Tree *lsm_tree) {
	delete_tree(lsm_tree->memtable);
	free_segments(lsm_tree->segments);
	free(lsm_tree);
}

static char* compact(int num_segment_files, char **segment_files) {
	if (segment_files == NULL) {
		printf("Segment files for compaction not provided\n");
		return NULL;
	}
	if (num_segment_files < 2) {
		printf("Need at least two segment files for compaction.\n");
		return NULL;
	}

	char *segment_file_a = *(segment_files);
	char *new_segment, *segment_file_b;

	for (int i = 1; i < num_segment_files; i++) {
		segment_file_b = *(segment_files + i);
		new_segment = merge_files(segment_file_a, segment_file_b);

		// old segment files no longer needed if new one was
		// successfully created
		if (delete_file(segment_file_a) != 0
				|| delete_file(segment_file_b) != 0) {
			printf("Failed to delete old segment files!\n");
			free(new_segment);
			return NULL;
		}

		// new segment will then be merged with consecutive other files
		segment_file_a = new_segment;
	}
	return new_segment;
}

/* Used to perform compaction step of two segment files. This method is
 * necessary for cleaning up old segment files and keeping read I/O from
 * getting out of control. Merges old segments together into new segments.*/
static char* merge_files(char *filename_a, char *filename_b) {
	// open files for segments of interest
	FILE *seg_ptr_a = fopen(filename_a, "r");
	FILE *seg_ptr_b = fopen(filename_b, "r");

	// create a new segment file
	char *new_segment = (char*) malloc(sizeof(char) * strlen(filename_a));
	sprintf(new_segment, "log_%ld.txt", time(NULL));
	FILE *new_fp = fopen(new_segment, "w");

	// setup for merge loop
	char line_seg_a[MAX_LINE_SIZE], line_seg_b[MAX_LINE_SIZE];
	bool keep_merging = true;
	bool incr_ptr_a = true, incr_ptr_b = true;
	char *read_result_a = NULL, *read_result_b = NULL;

	// run the merge loop
	while (keep_merging) {
		if (incr_ptr_a)
			read_result_a = fgets(line_seg_a, MAX_LINE_SIZE, seg_ptr_a);

		if (incr_ptr_b)
			read_result_b = fgets(line_seg_b, MAX_LINE_SIZE, seg_ptr_b);

		if (read_result_a == NULL && read_result_b == NULL)
			keep_merging = false;
		else if (read_result_a == NULL || read_result_b == NULL) {

			// add any remaining keys in remaining file to new file
			FILE *temp_ptr = read_result_a != NULL ? seg_ptr_a : seg_ptr_b;
			char *temp_line = read_result_b != NULL ? line_seg_a : line_seg_b;
			do {
				fputs(temp_line, new_fp);
			} while (fgets(temp_line, MAX_LINE_SIZE, temp_ptr) != NULL);

			keep_merging = 0;
		} else {
			/* merge these two files; figure out which should go into the file
			 * note that segment file b should be "younger" or "newer" than
			 * segment file a so we use that for more up to date data. If a segment
			 * currently has a recent "delete" marker, don't add the key to the newly
			 * created segment.*/
			int key_a = atoi(strtok(line_seg_a, ","));
			char *value_a = strtok(NULL, ",");

			int key_b = atoi(strtok(line_seg_b, ","));
			char *value_b = strtok(NULL, ",");

			if (key_a == key_b) {
				if (strcmp(value_b, DEL_MARKER) != 0)
					fputs(line_seg_b, new_fp);
				incr_ptr_a = true;
				incr_ptr_b = true;
			} else if (key_a > key_b) {
				if ((strcmp(value_b, DEL_MARKER) != 0))
					fputs(line_seg_b, new_fp);
				incr_ptr_a = false;
				incr_ptr_b = true;
			} else {  // key_a < key_b
				if (strcmp(value_a, DEL_MARKER) != 0)
					fputs(line_seg_a, new_fp);
				incr_ptr_a = true;
				incr_ptr_b = false;
			}
		}
	}
	// close file pointers for new file and the two segment files
	fclose(new_fp);
	fclose(seg_ptr_a);
	fclose(seg_ptr_b);
	return new_segment;
}

/* Searches existing segment files to see if the key exists.
 * Starts search with most recent segment (newest) */
static char* search_segments(char **segment_files, int full_segments, int key) {
	if (full_segments == 0) {
		return NULL;
	}

	for (int i = full_segments; i >= 0; i--) {

		FILE *segment_ptr = fopen(*(segment_files + i), "r");
		char *value = search_segment_file(segment_ptr, key);

		fclose(segment_ptr);
		if (value != NULL) {
			return value;
		}
	}
	return NULL;
}

/* Searches through a segment, if key in segment then return value;
 * uses recursion to look at last line of file first (note that files
 * are assumed to be small and can fit in stack memory. */
static char* search_segment_file(FILE *segment_ptr, int key) {
	char line[MAX_LINE_SIZE];

	if (fgets(line, MAX_LINE_SIZE, segment_ptr) != NULL) {
		char *value = search_segment_file(segment_ptr, key);
		if (value != NULL) {
			return value;
		}

		if (atoi(strtok(line, ",")) == key) {
			return strtok(NULL, ",");
		}
	}
	return NULL;
}

/* Creates an array of strings to hold segment file names */
static char** init_segments() {
	char **segments = (char**) malloc(MAX_SEGMENTS * sizeof(char*));
	if (segments == NULL) {
		printf("Allocation of memory for segment files failed.\n");
		return NULL;
	}

	for (int i = 0; i < MAX_SEGMENTS; i++)
		*(segments + i) = NULL;
	return segments;
}

/* Frees memory associated with segment file names */
static void free_segments(char **segments) {
	for (int i = 0; i < MAX_SEGMENTS; i++) {
		if (*(segments + i) != NULL) {
			free(*(segments + i));
		}
	}
	free(segments);
}

