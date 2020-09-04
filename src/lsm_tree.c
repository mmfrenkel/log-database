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
#include "index.h"

// prototypes for static functions here
static int execute_action(LSM_Tree *lsm_tree, Submission *submission);
static char* generate_new_segment_name();
static int update_index(Index *index, Memtable *memtable, char *filename);
static int add_key_to_index(Index *index, MNode *node, char *filename);


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

	char **segments = init_segment_list(MAX_SEGMENTS);
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

	Index *index = init_index(INDEX_SIZE);
	if (wal == NULL) {
		free(lsm_tree);
		free(memtable);
		free(segments);
		free(wal);
		return NULL;
	}

	lsm_tree->memtable = memtable;
	lsm_tree->segments = segments;
	lsm_tree->full_segments = 0;
	lsm_tree->wal = wal;
	lsm_tree->index = index;

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
	if (memtable_is_full(lsm_tree->memtable)) {
		if (ready_for_compaction(lsm_tree)) {
			int error = run_compaction(lsm_tree);
			if (error != 0) {
				shutdown_lsm_system(lsm_tree);
				die("Fatal Error: Compaction step failed! "
						"Please review logs for errors.\n");
			}
		}
		char *filename = send_memtable_to_segment(lsm_tree);
		if (!filename) {
			shutdown_lsm_system(lsm_tree);
			die("Fatal Error: Could not send memtable to segment.\n");
		}

		// update index before clearing memtable
		if (update_index(lsm_tree->index, lsm_tree->memtable, filename) != 0) {
			shutdown_lsm_system(lsm_tree);
			die("Fatal Error: Corrupted index.\n");
		}

		// clear the existing memtable; ready for new contents
		clear_memtable(lsm_tree->memtable);
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
		if (memtable_insert(lsm_tree->memtable, submission->key, submission->value) == 0) {
			lsm_tree->memtable->count_keys++;
		} else {
			printf("Insertion of new node failed.\n");
			return -1;
		}

	} else if (submission->action == SEARCH) {
		// search memtable first
		MNode *node = search_memtable(lsm_tree->memtable, submission->key);
		char *value;

		if (node == NULL) { // value wasn't in memtable, so look in segments
			value = lsm_tree_search_with_index(lsm_tree, submission->key);
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
		if (memtable_delete(lsm_tree->memtable, submission->key, false, TOMBSTONE) != 0) {
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

static char* generate_new_segment_name() {
	char *filename = (char*) malloc(FILENAME_SIZE * sizeof(char));
	if (filename == NULL) {
		printf("Failed to allocate memory for new filename\n");
		return NULL;
	}

	// time returns 10 digit number, plus underscore and another 2 digits
	// + file suffix (3 char)  + \0 = min 17 digits
	sprintf(filename, "%s%ld_%d.log", SEGMENT_LOCATION, time(NULL), rand() % 10);
	return filename;
}

/* Runs compaction of segments existing in LSM tree */
int run_compaction(LSM_Tree *lsm_tree) {
	printf("> LSM System Alert: Running compaction...\n");

	char *new_segment = generate_new_segment_name();
	if (!new_segment) {
		printf("Couldn't run compaction without a new segment.\n");
		return -1;
	}

	int error = compact_segments(lsm_tree->segments, MAX_SEGMENTS,
			new_segment, MAX_LINE_SIZE, TOMBSTONE);
	if (error) {
		printf("Error occurred while compacting segment files\n");
		return -1;
	}

	// assign new segment and clear existing segments
	free_segment_list(lsm_tree->segments, MAX_SEGMENTS);
	lsm_tree->segments = init_segment_list(MAX_SEGMENTS);
	*(lsm_tree->segments) = new_segment;
	lsm_tree->full_segments = 1;

	return 0;
}

/* Sends in-memory memtable (binary tree) to a segment file,
 * while also reseting the memtable and updating the record
 * of segment files available within the system. */
char* send_memtable_to_segment(LSM_Tree *lsm_tree) {
	char *new_segment = generate_new_segment_name();
	if (!new_segment) {
		printf("Couldn't send memtable to segment.\n");
		return NULL;
	}

	int error = memtable_to_segment(lsm_tree->memtable, new_segment);
	if (error) {
		printf("Failed at saving memtable to segment.\n");
		return NULL;
	}

	// make new segment the next segment in list
	*(lsm_tree->segments + lsm_tree->full_segments) = new_segment;
	lsm_tree->full_segments += 1;
	return new_segment;
}

/* Finds the value of a key using the LSM Tree Systems file system index. */
char* lsm_tree_search_with_index(LSM_Tree *lsm_tree, int key) {
	char *filename = index_lookup(lsm_tree->index, key);
	if (!filename) {
		return NULL;
	}
	return search_segment(filename, key, MAX_LINE_SIZE);
}

/* Searches existing segment files to see if the key exists.
 * Starts search with most recent segment (newest), but searches until found. */
char* lsm_tree_linear_search(LSM_Tree *lsm_tree, int key) {

	char **segment_files = lsm_tree->segments;
	int full_segments = lsm_tree->full_segments;

	if (full_segments == 0) {
		return NULL;
	}

	// full_segments -1 because segment 1 at index 0
	for (int i = full_segments - 1; i >= 0; i--) {
		char *value = search_segment(*(segment_files + i), key, MAX_LINE_SIZE);
		if (value != NULL) {
			return value;
		}
	}
	return NULL;
}

/* Prints the status of the LSM Tree system (i.e., keys in memtable,
 * and full segments */
void show_status(LSM_Tree *lsm_tree) {
	printf("\n> LSM Tree System Alert: Memtable currently holds %d keys, File system "
			"holds %d segment(s).\n", lsm_tree->memtable->count_keys,
			lsm_tree->full_segments);
}

/* Prints out all active segment files */
void print_active_segments(LSM_Tree *lsm_tree) {
	for (int i = 0; i < lsm_tree->full_segments; i++) {
		printf("%s\n", lsm_tree->segments[i]);
	}
}

// wrapper function for recursively adding memtable keys to index */
static int update_index(Index *index, Memtable *memtable, char *filename) {
	return add_key_to_index(index, memtable->root, filename);
}

static int add_key_to_index(Index *index, MNode *node, char *filename) {
	if (node) {
		if (index_insert(index, node->key, filename) != 0) {
			printf("Update to index failed. Index may be incomplete.\n");
			return -1;
		}
		add_key_to_index(index, node->left_child, filename);
		add_key_to_index(index, node->right_child, filename);
	}
	return 0;
}

/* Call to deallocate all memory for LSM tree system*/
void shutdown_lsm_system(LSM_Tree *lsm_tree) {
	// send what contents are left in memtable to disk
	if (lsm_tree->memtable->count_keys != 0) {

		if (serialize_memtable(lsm_tree->memtable, LATEST_MEMTABLE) != 0) {
			printf("Warning, memtable contents were not successfully saved.\n");
		}
	}

	fclose(lsm_tree->wal);
	delete_memtable(lsm_tree->memtable);
	free_segment_list(lsm_tree->segments, MAX_SEGMENTS);
	free(lsm_tree);
}
