#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lsm_tree.h"
#include "user_io.h"

static void get_user_input(char *buf, int buf_size, char *print_message);

/* Retrieve the next LSM Tree System submission from a user,
 * getting input from the user where necessary and returning a
 * Submission struct that contains the action item and data. */
Submission* next_submission(void) {
	print_user_options();

	// ask user for category of submission
	char user_value[STR_BUF];

	get_user_input(user_value, STR_BUF, "Select an action: ");
	int user_selection = atoi(user_value);

	if (user_selection > NUM_OPTIONS || user_selection <= 0) {
		printf("Users can only selection actions labeled 1 - %d.\n", NUM_OPTIONS);
		return NULL;
	}

	Submission *user_submission = (Submission*) malloc(sizeof(Submission));
	if (user_submission == NULL) {
		printf("Allocation of memory for new user submission failed\n");
		return NULL;
	}

	user_submission->action = user_selection;

	if (user_selection == 1 || user_selection == 2 || user_selection == 3) {
		int key = get_key();
		if (key <= 0)
			return NULL;
		user_submission->key = key;
	} else {
		user_submission->key = 0;
	}

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
	printf(" 5. PRINT Memtable\n");
	printf(" 6. EXIT \n");
	printf("-----------------------------------------\n");
}

/* Get a key from a user */
int get_key() {
	char key_value[MAX_LEN_KEYS];
	get_user_input(key_value, MAX_LEN_KEYS, "Provide a key (numeric, > 0):");
	int key = atoi(key_value);

	if (key == 0 || key < 0) {
		printf("Please provide a numeric-only key value >0.\n");
		return -1;
	}
	return key;
}

/* Get a value from a user */
char* get_value() {
	char *value = (char*) malloc(sizeof(char) * MAX_LEN_DATA);
	char *substring = "Provide some data for submitted key (<%d characters):";
	char print_string[strlen(substring) + STR_BUF];  // need way to handle this better

	sprintf(print_string, substring, MAX_LEN_DATA);

	get_user_input(value, MAX_LEN_DATA, print_string);
	return value;
}

static void get_user_input(char *buf, int buf_size, char *print_message) {
	char *p;

	if (strlen(print_message) > 0) {
		printf("%s\n", print_message);
	}

	if (fgets(buf, buf_size, stdin) != NULL) {
		/* remove newline character, if necessary */
		if ((p = strchr(buf, '\n')) != NULL)
			*p = '\0';
	}
}

