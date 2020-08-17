#include <stdio.h>
#include <stdlib.h>
#include "binary_tree.h"

#define MAX_LEN_KEYS 10;


void die(char *msg) {
	printf("%s", msg);
	exit(1);
}

char *get_user_input(int buffer_size, char* print_message) {
	char buf[buffer_size];
	char *p;

	if (strlen(print_message) > 0) {
		printf("%s\n", print_message);
	}

	if (fgets(buf, buffer_size, stdin) != NULL) {

		/* remove newline character, if necessary */
		if ((p = strchr(buf, '\n')) != NULL)
			*p = '\0';

		return buf;
	}
	return NULL;
}

/* Insert a Key Value Pair Case */
void case_one(Binary_Tree *btree) {
	char *key_value = get_user_input(10, "Provide a key (numeric, >0):");
	int key = atoi(key_value);

	if (key == 0) {
		printf("Please provide a numeric-only key value >0");
	}

	char *data = get_user_input(50, "Provide some data for this key:");

	insert(btree, key, data);
	btree->count_keys++;
}

/* Delete a Key-Value Pair Case */
void case_three(Binary_Tree *btree){
	char *key_value = get_user_input(10, "Provide a key to delete (numeric, >0):");
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

	char user_submission;
	int count_keys_in_tree = 0;

	while(1) {

		printf("Available Actions are below:\n");
		printf(" 1. Add New Key, Value Pair\n");
		printf(" 2. Print all Key, Value Pairs\n");
		printf(" 3. Delete Key, Value Pair Based on Key\n");
		printf(" 4. Flush to Disk\n");
		printf(" 5. Exit\n");

		char *user_submission = get_user_input(1, "Select an action:");

		switch (user_submission) {
		case '1':
			case_one(btree);
			print_tree(btree, "in_order_traversal");
			break;

		case '2':
			print_tree(btree, "in_order_traversal");
			break;

		case '3':
			case_three(btree);
			print_tree(btree, "in_order_traversal");
			break;

		case '4':
			continue;

		case '5':
			printf("Exiting...\n");
			exit(1);
		}
	}

	delete_tree(btree);
}

