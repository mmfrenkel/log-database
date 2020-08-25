#ifndef LSM_TREE_H
#define LSM_TREE_H

#include "binary_tree.h"

#define NUM_OPTIONS 6          // number of actions LSM tree can do
#define STR_BUF 5              // leave plenty of room for options
#define MAX_LEN_KEYS 10        // max size of key (10 digits)
#define MAX_LEN_DATA 50        // max length of data for value in database
#define MAX_KEYS_IN_TREE 10    // max # keys held in tree before flush to segment
#define MAX_SEGMENTS 2         // max # full segments before compaction
#define FILENAME_SIZE 20       // file name size
#define MAX_LINE_SIZE 60       // max number of characters in a single line of a segment file

enum available_actions {
	ADD = 1,
	SEARCH = 2,
	DELETE = 3,
	SAVE = 4,
	PRINT_MEMTABLE = 5,
	EXIT = 6
};

typedef struct user_submission {
	enum available_actions action;
	int key;
	char *value;
} Submission;

typedef struct lsm_tree_system {
	Binary_Tree *memtable;
	char **segments;
	int full_segments;
} LSM_Tree;

LSM_Tree* init_lsm_tree();

int handle_submission(LSM_Tree *lsm_tree, Submission *submission);

int run_compaction(LSM_Tree *lsm_tree);

void shutdown_lsm_system(LSM_Tree *lsm_tree);

#endif

