#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
#include <time.h>
#include "binary_tree.h"
#include "custom_io.h"
#include "error.h"
#include "user_io.h"

#define LEN_OPTIONS 1
#define MAX_LEN_KEYS 10
#define MAX_LEN_DATA 50
#define MAX_KEYS_IN_TREE 10
#define MAX_SEGMENTS 2
#define FILENAME_SIZE 20

/* Print out user options */
void print_user_options() {
	printf("\n-----------------------------------------\n");
	printf("Available Actions are below:\n");
	printf(" 1. Add New Key, Value Pair\n");
	printf(" 2. Print all Key, Value Pairs\n");
	printf(" 3. Delete Key, Value Pair Based on Key\n");
	printf(" 4. Flush to Disk\n");
	printf(" 5. Exit\n");
	printf("\n-----------------------------------------\n");
}

/* Insert a Key Value Pair Case */
void case_one(Binary_Tree *memtable) {

	char key_value[MAX_LEN_KEYS];
	get_user_input(key_value, MAX_LEN_KEYS, "Provide a key (numeric, >0):");
	int key = atoi(key_value);

	if (key == 0) {
		printf("Please provide a numeric-only key value >0");
	}

	char data[MAX_LEN_DATA];
	get_user_input(data, MAX_LEN_DATA, "Provide some data for this key:");

	if (insert(memtable, key, data) == 0) {
		memtable->count_keys++;
	} else {
		printf("Insertion of new node failed.\n");
	}
}

/* Delete a Key-Value Pair Case */
void case_three(Binary_Tree *memtable) {

	char key_value[MAX_LEN_KEYS];
	get_user_input(key_value, MAX_LEN_KEYS,
			"Provide a key to delete (numeric, >0):");

	int key = atoi(key_value);
	if (key == 0) {
		printf("Please provide a numeric-only key value >0");
	}

	if (delete(memtable, key) == 0) {
		memtable->count_keys--;
	} else {
		printf("Deletion of key, value pair with key %d failed.\n", key);
	}
}

char** init_segments() {
	char **segments = (char**) malloc(MAX_SEGMENTS * sizeof(char*));
	for (int i = 0; i < MAX_SEGMENTS; i++)
		*(segments + i) = NULL;
	return segments;
}

void free_segments(char **segments) {
	for (int i = 0; i < MAX_SEGMENTS; i++)
		free(*(segments + i));
	free(segments);
}

int main(int argc, char *argv[]) {

	printf("Database System Started!\n");
	int keys_in_memory = 0;
	int full_segments = 0;
	char **segments = init_segments();

	/* This memtable (in this case, a binary tree) will hold 
	 * user data in memory until flush to log */
	Binary_Tree *memtable = (Binary_Tree*) malloc(sizeof(Binary_Tree));
	if (memtable == NULL) {
		die("Allocation of memory for memtable failed.");
	}
	memtable->root = NULL;
	memtable->count_keys = 0;

	char user_submission[LEN_OPTIONS];
	int user_selection = 0;

	while (1) {
		print_user_options();
		get_user_input(user_submission, LEN_OPTIONS, "Select an action:");
		user_selection = atoi(user_submission);

		switch (user_selection) {
		case 1:
			case_one(memtable);
			print_tree(memtable, "in_order_traversal");
			keys_in_memory += 1;
			break;
		case 2:
			print_tree(memtable, "in_order_traversal");
			break;
		case 3:
			case_three(memtable);
			print_tree(memtable, "in_order_traversal");
			keys_in_memory -= 1;
			break;
		case 4:
			serialize_tree(memtable, "binary_tree.txt");
			break;
		case 5:
			printf("Exiting...\n");
			exit(1);
		default:
			printf("Please submit a number 1-5 (no spare characters).\n");
			break;
		}

		// Compact existing files, if necessary
		if (keys_in_memory == MAX_KEYS_IN_TREE) {
			if (full_segments == MAX_SEGMENTS) {
				// run compaction of existing files	to produce single one 
				*(segments) = compact(MAX_SEGMENTS, segments);
				for (int i = 1; i < MAX_SEGMENTS; i++) {
					free(*(segments + i));
					*(segments + i) = NULL;
				}
				full_segments = 1;
			}
			char *filename = (char*) malloc(sizeof(char) * FILENAME_SIZE);
			sprintf(filename, "%s_%ld.txt", "log", time(NULL));
			tree_to_sorted_strings_table(memtable, filename);

			*(segments + full_segments) = filename;
			full_segments += 1;
			clear_tree(memtable);
			keys_in_memory = 0;
		}
	}
	// I'm finished!
	delete_tree(memtable);
	free_segments(segments);
}

