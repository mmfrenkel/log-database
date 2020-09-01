#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include "lsm_tree.h"
#include "error.h"
#include "memtable.h"
#include "segment.h"
#include "wal.h"

// prototypes for static functions here
static char** init_segments();
static int execute_action(LSM_Tree *lsm_tree, Submission *submission);
static char* compact_segments(char **segment_files);
static char* merge_files(char *segment_file1, char *segment_file2);
static void merge_lines(char *line_a, char *line_b,
		                FILE *new_fp, bool *incr_a, bool *incr_b);
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

	Memtable *memtable = init_memtable();
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

	FILE *wal = init_wal(WRITE_AHEAD_LOG);
	if (wal == NULL) {
		free(lsm_tree);
		free(memtable);
		free(segments);
		return NULL;
	}

	lsm_tree->memtable = memtable;
	lsm_tree->segments = segments;
	lsm_tree->full_segments = 0;
	lsm_tree->wal = wal;

	return lsm_tree;
}

/* Handles the submission provided by user; returns 0 if all succeeds,
 * otherwise returns -1.  */
int handle_submission(LSM_Tree *lsm_tree, Submission *submission) {

	// start by writing directly to WAL
	int error = submission_to_wal(lsm_tree->wal, submission->action,
			submission->key, submission->value, MAX_LINE_SIZE, true);
	if (error) {
		shutdown_lsm_system(lsm_tree);
		die("Fatal Error: Submission to WAL Failed.\n");
	}

	// do the thing that the user actually requested
	error = execute_action(lsm_tree, submission);
	if (error != 0) {
		printf("Failed to execute requested user action.\n");
		return -1;
	}

	/* make sure the system sends memtable to segment
	 * and runs compaction without being asked. this should be
	 * in the background, rather than sequential in future */
	if (is_full(lsm_tree->memtable)) {
		if (ready_for_compaction(lsm_tree)) {
			int error = run_compaction(lsm_tree);
			if (error != 0) {
				shutdown_lsm_system(lsm_tree);
				die("Fatal Error: Compaction step failed! "
						"Please review logs for errors.\n");
			}
		}
		int error = send_memtable_to_segment(lsm_tree);
		if (error != 0) {
			shutdown_lsm_system(lsm_tree);
			die("Fatal Error: Could not send memtable to segment.\n");
		}
	}
	return 0;
}

/* Does the action that the user submitted. */
static int execute_action(LSM_Tree *lsm_tree, Submission *submission) {
	if (submission->action == ADD) {
		if (strcmp(submission->value, TOMBSTONE) == 0) {
			printf("Cannot insert new record with value equal to the "
				   "tombstone for this system (%s)\n", TOMBSTONE);
			return -1;
		}
		if (insert(lsm_tree->memtable, submission->key, submission->value) == 0) {
			lsm_tree->memtable->count_keys++;
		} else {
			printf("Insertion of new node failed.\n");
			return -1;
		}

	} else if (submission->action == SEARCH) {
		// search memtable first
		MNode *node = search(lsm_tree->memtable, submission->key);
		char *value;

		if (node == NULL) { // value wasn't in memtable, so look through the log files
			value = search_segments(lsm_tree->segments, lsm_tree->full_segments,
					submission->key);
		} else {
			value = node->data;
		}
		if (value != NULL) {
			printf("The value for key %d is %s.\n", submission->key, value);
		} else {
			printf("Key not found in LSM Tree system.\n");
		}

	} else if (submission->action == DELETE) {
		// always soft delete here; do not decrement keys in tree
		if (delete(lsm_tree->memtable, submission->key, false, TOMBSTONE) != 0) {
			printf("Deletion of key %d failed.\n", submission->key);
			return -1;
		}

	} else if (submission->action == FLUSH) {
		serialize_memtable(lsm_tree->memtable, "memtable.txt");

	} else if (submission->action == PRINT_MEMTABLE) {
		print_memtable(lsm_tree->memtable, "in_order_traversal");

	} else {
		printf("Could not process request in submission.\n");
		return -1;
	}
	return 0;
}

/* Determines if the LSM System has enough segments to
 * warrant compaction step  */
bool ready_for_compaction(LSM_Tree *lsm_tree) {
	if (lsm_tree->full_segments == MAX_SEGMENTS) {
		return true;
	}
	return false;
}

/* Runs compaction of segments existing in LSM tree */
int run_compaction(LSM_Tree *lsm_tree) {
	printf("> LSM System Alert: Running compaction...\n");

	char *new_segment = compact_segments(lsm_tree->segments);
	if (new_segment == NULL) {
		return -1;
	}

	// assign new segment and clear existing segments
	free_segments(lsm_tree->segments);
	lsm_tree->segments = init_segments();
	*(lsm_tree->segments) = new_segment;
	lsm_tree->full_segments = 1;

	return 0;
}

/* Sends in-memory memtable (binary tree) to a segment file,
 * while also reseting the memtable and updating the record
 * of segment files available within the system. */
int send_memtable_to_segment(LSM_Tree *lsm_tree) {
	char *filename = memtable_to_segment(lsm_tree->memtable);
	if (filename == NULL) {
		printf("Failed at saving memtable to segment.\n");
		return -1;
	}

	// make new segment the next segment in list
	*(lsm_tree->segments + lsm_tree->full_segments) = filename;
	lsm_tree->full_segments += 1;

	// clear the existing memtable; ready for new contents
	clear_memtable(lsm_tree->memtable);
	return 0;
}

/* Prints the status of the LSM Tree system (i.e., keys in memtable,
 * and full segments */
void show_status(LSM_Tree *lsm_tree) {
	printf("\n> LSM Tree System Alert: Memtable currently holds %d keys, File system "
			"holds %d segments.\n", lsm_tree->memtable->count_keys,
			lsm_tree->full_segments);
}

void print_active_segments(LSM_Tree *lsm_tree) {
	for (int i = 0; i < lsm_tree->full_segments; i++) {
		printf("%s\n", lsm_tree->segments[i]);
	}
}

/* Call to deallocate all memory for LSM tree system*/
void shutdown_lsm_system(LSM_Tree *lsm_tree) {
	// send what contents are left in memtable to disk
	if (lsm_tree->memtable->count_keys != 0) {
		if (serialize_memtable(lsm_tree->memtable, "latest_memtable_state.log") != 0) {
			printf("Warning, memtable contents were not successfully saved.\n");
		}
	}

	fclose(lsm_tree->wal);

	delete_memtable(lsm_tree->memtable);

	free_segments(lsm_tree->segments);

	free(lsm_tree);
}

/* Takes a list of segment file names and compacts two at
 * a time, sequentially; deletes files no longer needed */
static char* compact_segments(char **segment_files) {
	char *segment_a = *(segment_files);
	char *new_segment, *segment_b;

	for (int i = 1; i < MAX_SEGMENTS; i++) {
		segment_b = *(segment_files + i);

		new_segment = merge_files(segment_a, segment_b);
		if (new_segment == NULL) {
			return NULL;
		}

		// if existing file segments fail to delete, this is big problem for disk space.
		if (delete_segment(segment_a) != 0 || delete_segment(segment_b) != 0) {
			printf("Failed to delete old segment files");
			delete_segment(new_segment);
			free(new_segment);
			return NULL;
		}

		// new segment will then be merged with consecutive other files
		segment_a = new_segment;
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
	char *new_segment = (char*) malloc(sizeof(char) * FILENAME_SIZE);
	if (new_segment == NULL) {
		printf("Could not allocate memory for new segment name.\n");
		return NULL;
	}

	sprintf(new_segment, "%ld_%d.log", time(NULL), rand() % 10);
	FILE *new_fp = fopen(new_segment, "w");

	// setup for merge loop
	char line_a[MAX_LINE_SIZE];
	char line_b[MAX_LINE_SIZE];
	bool keep_merging = true;
	bool incr_ptr_a = true, incr_ptr_b = true;

	// run the merge loop
	while (keep_merging) {
		if (incr_ptr_a) {
			memset(line_a, '\0', MAX_LINE_SIZE);
			readline_from_segment(line_a, MAX_LINE_SIZE, seg_ptr_a, true);
		}
		if (incr_ptr_b) {
			memset(line_b, '\0', MAX_LINE_SIZE);
			readline_from_segment(line_b, MAX_LINE_SIZE, seg_ptr_b, true);
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
			} while (fgets(temp_line, MAX_LINE_SIZE, temp_ptr) != NULL);

			keep_merging = false;

		} else
			merge_lines(line_a, line_b, new_fp, &incr_ptr_a, &incr_ptr_b);
	}
	// fclose() returns 0 if closing file is successful
	if (fclose(new_fp) || fclose(seg_ptr_a) || fclose(seg_ptr_b)) {
		printf("Failed to close one or more of the compacting files.\n");
		free(new_segment);
		return NULL;
	}
	return new_segment;
}

/* Helper function for merging two files together by adding lines from either file a
 * or file b to the new file, then assigning values to determine whether the
 * calling function should increment a or b */
static void merge_lines(char *line_a, char *line_b, FILE *new_fp, bool *incr_a, bool *incr_b) {
	//  need to make copy of lines because strtok() alters char array
	char line_a_copy[MAX_LINE_SIZE], line_b_copy[MAX_LINE_SIZE];
	strcpy(line_a_copy, line_a);
	strcpy(line_b_copy, line_b);

	int key_a = atoi(strtok(line_a_copy, ","));
	char *value_a = strtok(NULL, ",");

	int key_b = atoi(strtok(line_b_copy, ","));
	char *value_b = strtok(NULL, ",");

	if (key_a == key_b) {
		if (strcmp(value_b, TOMBSTONE) != 0) {
			fputs(line_b, new_fp);
			fputs("\n", new_fp);
		}
		*incr_a = true;
		*incr_b = true;
	} else if (key_a > key_b) {
		if ((strcmp(value_b, TOMBSTONE) != 0)) {
			fputs(line_b, new_fp);
			fputs("\n", new_fp);
		}
		*incr_a = false;
		*incr_b = true;
	} else {  // key_a < key_b
		if (strcmp(value_a, TOMBSTONE) != 0) {
			fputs(line_a, new_fp);
			fputs("\n", new_fp);
		}
		*incr_a = true;
		*incr_b = false;
	}
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

