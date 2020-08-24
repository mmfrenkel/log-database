#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lsm_tree.h"
#include "user_io.h"

Submission* next_submission(void) {
	print_user_options();

	// ask user for category of submission
	char user_value[LEN_OPTIONS];
	get_user_input(user_value, LEN_OPTIONS, "Select an action: ");
	int user_selection = atoi(user_value);

	if (user_selection > NUM_OPTIONS || user_selection <= 0) {
		printf("Users can only selection actions labeled 1 - %d", NUM_OPTIONS);
		return NULL;
	}

	Submission *user_submission = (Submission*) malloc(sizeof(Submission));
	if (user_submission == NULL) {
		printf("Allocation of memory for new user submission failed\n");
		return NULL;
	}

	user_submission->action = user_selection;
	user_submission->key = get_key();

	if (user_selection == 1) {
		user_submission->value = get_value();
	} else {
		user_submission->value = NULL;
	}
	return user_submission;
}

/* Print out user options */
void print_user_options() {
	printf("\n-----------------------------------------\n");
	printf("Available Actions are below:\n");
	printf(" 1. ADD New Key, Value Pair\n");
	printf(" 2. SEARCH with key to find value\n");
	printf(" 3. DELETE Key, Value Pair Based on Key\n");
	printf(" 4. SAVE to Disk\n");
	printf(" 5. Exit\n");
	printf("\n-----------------------------------------\n");
}

/* Get a key from a user */
int get_key() {
	char key_value[MAX_LEN_KEYS];
	get_user_input(key_value, MAX_LEN_KEYS, "Provide a key (numeric, > 0):");
	int key = atoi(key_value);

	if (key == 0 || key < 0) {
		printf("Please provide a numeric-only key value >0");
	}
	return key;
}

/* Get a value from a user */
char* get_value() {
	char *value = (char*) malloc(sizeof(char) * MAX_LEN_DATA);
	get_user_input(value, MAX_LEN_DATA,
			"Provide some data for submitted key (<%d characters)");

	return value;
}

char* get_user_input(char buf[], int buf_size, char *print_message) {
	char *p;

	if (strlen(print_message) > 0) {
		printf("%s\n", print_message);
	}

	if (fgets(buf, buf_size, stdin) != NULL) {
		/* remove newline character, if necessary */
		if ((p = strchr(buf, '\n')) != NULL)
			*p = '\0';
	}
	return NULL;
}
