#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "binary_tree.h"
#include "custom_io.h"
#include "bt_utilities.h"

#define LEN_OPTIONS 1
#define MAX_LEN_KEYS 10
#define MAX_LEN_DATA 50


/* Insert a Key Value Pair Case */
void case_one(Binary_Tree *btree) {

	char key_value[MAX_LEN_KEYS];
	get_user_input(key_value, MAX_LEN_KEYS, "Provide a key (numeric, >0):");
	int key = atoi(key_value);

	if (key == 0) {
		printf("Please provide a numeric-only key value >0");
	}

	char data[MAX_LEN_DATA];
	get_user_input(data, MAX_LEN_DATA, "Provide some data for this key:");

	insert(btree, key, data);
	btree->count_keys++;
}

/* Delete a Key-Value Pair Case */
void case_three(Binary_Tree *btree){

	char key_value[MAX_LEN_KEYS];
	get_user_input(key_value, MAX_LEN_KEYS, "Provide a key to delete (numeric, >0):");
	int key = atoi(key_value);

	if (key == 0) {
		printf("Please provide a numeric-only key value >0");
	}
	delete(btree, key);
}


int main(int argc, char *argv[]) {

	printf("Database System Started!\n");

	Binary_Tree *btree = (Binary_Tree *) malloc(sizeof(Binary_Tree));
	if (btree == NULL) {
		die("Allocation of memory for Binary Tree failed.");
	}
	btree->root = NULL;
	btree->count_keys = 0;

	char user_submission[10];
	int user_selection = 0;

	while(1) {

        printf("\n-----------------------------------------\n");
		printf("Available Actions are below:\n");
		printf(" 1. Add New Key, Value Pair\n");
		printf(" 2. Print all Key, Value Pairs\n");
		printf(" 3. Delete Key, Value Pair Based on Key\n");
		printf(" 4. Flush to Disk\n");
		printf(" 5. Exit\n");
        printf("\n-----------------------------------------\n");

        get_user_input(user_submission, 10, "Select an action:");
		user_selection = atoi(user_submission);

		switch (user_selection) {
		case 1:
			case_one(btree);
			print_tree(btree, "in_order_traversal");
			break;

		case 2:
			print_tree(btree, "in_order_traversal");
			break;

		case 3:
			case_three(btree);
			print_tree(btree, "in_order_traversal");
			break;

		case 4:
			save_tree_to_file(btree, "binary_tree.txt");
			break;

		case 5:
			printf("Exiting...\n");
			exit(1);

		default:
			printf("Please submit a number 1-5 (no spare characters).\n");
			break;
		}
	}

	delete_tree(btree);
}

