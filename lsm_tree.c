#include <stdio.h>
#include <stdlib.h>
#include "lsm_tree.h"
#include "binary_tree.h"


// prototypes for static functions here
static char* compact(int num_segment_files, char **segment_files);
static char* merge_files(char *segment_file1, char *segment_file2);


/* Creates an LSM Tree for the program to use, initializing
 * everything properly */
LSM_Tree* init_lsm_tree() {
	LSM_Tree *lsm_tree = (LSM_Tree*) malloc(sizeof(LSM_Tree));
	if (lsm_tree == NULL) {
		printf("Allocation of memory for LSM Tree failed.\n");
		return NULL;
	}

	Binary_Tree *memtable = (Binary_Tree*) malloc(sizeof(Binary_Tree));
	if (memtable == NULL) {
		printf("Allocation of memory for memtable failed.\n");
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


/* Handles the submission provided by user */
int handle_submission(LSM_Tree *lsm_tree, Submission *submission) {

	if (submission->action == ADD) {
		if (insert(lsm_tree->memtable, submission->key, submission->value) == 0) {
			lsm_tree->memtable->count_keys++;
		} else {
			printf("Insertion of new node failed.\n");
			return -1;
		}

	} else if (submission->action == SEARCH) {
		BT_Node *node = search(lsm_tree->memtable, submission->key);
		printf("Value for key %d is %s", node->key, node->data);

	} else if (submission->action == DELETE) {
		if (delete(lsm_tree->memtable, submission->key) == 0) {
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

	return run_compaction(lsm_tree);  // this should be in the background -- TO DO
}

int run_compaction(LSM_Tree *lsm_tree){

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

static char* compact(int num_segment_files, char **segment_files) {
	if (segment_files == NULL) {
		printf("Segment files for compaction not provided\n");
		return NULL;
	}
	if (num_segment_files < 2) {
		printf("Need at least two segment files for compaction.\n");
		return NULL;
	}

	char *segment_file1 = *(segment_files);
	char *new_segment, *segment_file2;

	for (int i = 1; i < num_segment_files; i++) {
		segment_file2 = *(segment_files + i);
		new_segment = merge_files(segment_file1, segment_file2);

		// old segment files no longer needed if new one was
		// successfully created
		delete_file(segment_file1);
		delete_file(segment_file2);

		// new segment will then be merged with consecutive other files
		segment_file1 = new_segment;
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
	int keep_merging = 1;
	int incr_ptr_a = 1, incr_ptr_b = 1;
	char *read_result_a = NULL, *read_result_b = NULL;

	// run the merge loop
	while (keep_merging) {
		if (incr_ptr_a) {
			read_result_a = fgets(line_seg_a, MAX_LINE_SIZE, seg_ptr_a);
		}
		if (incr_ptr_b) {
			read_result_b = fgets(line_seg_b, MAX_LINE_SIZE, seg_ptr_b);
		}
		if (read_result_a == NULL && read_result_b == NULL) {
			// both files are empty; so ready to be done
			keep_merging = 0;

		} else if (read_result_a == NULL || read_result_b == NULL) {
			// add any remaining keys in remaining file to new file
			FILE *temp_ptr = read_result_a != NULL ? seg_ptr_a : seg_ptr_b;
			char *temp_line = read_result_b != NULL ? line_seg_a : line_seg_b;
			do {
				fputs(temp_line, new_fp);
			} while (fgets(temp_line, MAX_LINE_SIZE, temp_ptr) != NULL);
			keep_merging = 0;
		} else {
			/* merge these two files; figure out which should go into the file
			 * note that segment file 2 should be "younger" or "newer" than
			 * segment file 1 so we use that for more up to date data */
			int key_1 = atoi(strtok(line_seg_a, ","));
			int key_2 = atoi(strtok(line_seg_b, ","));
			if (key_1 == key_2) {
				fputs(line_seg_a, new_fp);
				incr_ptr_a = 1;
				incr_ptr_b = 1;
			} else if (key_1 > key_2) {
				fputs(line_seg_b, new_fp);
				incr_ptr_a = 0;
				incr_ptr_b = 1;
			} else {  // key_2 > key_1
				fputs(line_seg_a, new_fp);
				incr_ptr_a = 1;
				incr_ptr_b = 0;
			}
		}
	}

	// close file pointers for new file and the two segment files
	fclose(new_fp);
	fclose(seg_ptr_a);
	fclose(seg_ptr_b);
	return new_segment;
}

char** init_segments() {
	char **segments = (char**) malloc(MAX_SEGMENTS * sizeof(char*));
	if (segments == NULL) {
		printf("Allocation of memory for segment files failed.\n");
		return NULL;
	}

	for (int i = 0; i < MAX_SEGMENTS; i++)
		*(segments + i) = NULL;
	return segments;
}

void free_segments(char **segments) {
	for (int i = 0; i < MAX_SEGMENTS; i++) {
		if (*(segments + i) != NULL) {
			free(*(segments + i));
		}
	}
	free(segments);
}

void shutdown_lsm_system(LSM_Tree *lsm_tree) {
	delete_tree(lsm_tree->memtable);
	free_segments(lsm_tree->segments);
}

