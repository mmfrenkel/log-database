#include <stdio.h>
#include <stdlib.h>
#include "binary_tree.h"

#define USER_SUBMISSION_SIZE 1


int main(int argc, char *argv[]) {

	printf("Database System Started!\n");

	Binary_Tree *btree = (Binary_Tree *) malloc(sizeof(Binary_Tree));
	char user_submission;
	int count_keys_in_tree = 0;

	while(1) {

		printf("Please select an action\n");
		printf(" 1. Add New Key, Value Pair\n");
		printf(" 2. Print all Key, Value Pairs\n");
		printf(" 3. Delete Key, Value Pair Based on Key\n");
		printf(" 4. Flush to Disk\n");

		scanf("%c", &user_submission);

		switch (user_submission) {
		case '1':
			insert(btree, 1, "2");
			count_keys_in_tree++;
			break;
		}
	}

	delete_tree(btree);
}
