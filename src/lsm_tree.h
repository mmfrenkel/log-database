#ifndef LSM_TREE_H
#define LSM_TREE_H

#include <stdio.h>
#include "memtable.h"
#include "index.h"

#define NUM_OPTIONS 6          		// number of actions LSM tree can do
#define STR_BUF 5              		// leave plenty of room for options
#define MAX_LEN_KEYS 10        		// max size of key (10 digits)
#define MAX_LEN_DATA 50        		// max length of data for value in database
#define MAX_SEGMENTS 2         		// max # full segments before compaction
#define FILENAME_SIZE 20       		// file name size
#define MAX_LINE_SIZE 100      		// max number of characters in a single line of a segment file
#define TOMBSTONE "*-*"        		// special marker, denoting a key was deleted
#define WRITE_AHEAD_LOG "wal.log"   // name of write-ahead-log
#define INDEX_SIZE 91               // size of index (hash map)

enum available_actions {
	ADD = 1, SEARCH = 2, DELETE = 3, FLUSH = 4, PRINT_MEMTABLE = 5, EXIT = 6
};

typedef struct user_submission {
	enum available_actions action;
	int key;
	char *value;
} Submission;

typedef struct lsm_tree_system {
	Memtable *memtable;
	char **segments;
	int full_segments;
	FILE *wal;
	Index *index;
} LSM_Tree;

LSM_Tree* init_lsm_tree();

int handle_submission(LSM_Tree *lsm_tree, Submission *submission);

bool ready_for_compaction(LSM_Tree *lsm_tree);

int run_compaction(LSM_Tree *lsm_tree);

char* send_memtable_to_segment(LSM_Tree *lsm_tree);

char* lsm_tree_search_with_index(LSM_Tree *lsm_tree, int key);

char* lsm_tree_linear_search(LSM_Tree *lsm_tree, int key);

void print_active_segments(LSM_Tree *lsm_tree);

void show_status(LSM_Tree *lsm_tree);

void shutdown_lsm_system(LSM_Tree *lsm_tree);

#endif

