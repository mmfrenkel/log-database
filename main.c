#include <stdio.h> 
#include <stdlib.h>
#include "custom_io.h"
#include "error.h"
#include "user_io.h"
#include "lsm_tree.h"

int main(int argc, char *argv[]) {
	printf("Database System Started!\n");

	LSM_Tree *lsm_tree = init_lsm_tree();

	while (1) {
		Submission *user_submission = next_submission();

		if (user_submission == NULL) {
			printf("There was an issue with your submission, try again.\n");
		} else if (user_submission->action == EXIT) {
			shutdown_lsm_system(lsm_tree);
			exit(0);
		} else {
			int err = handle_submission(lsm_tree, user_submission);
			if (err) {
				printf("LSM Tree system returned error %d");
			}
		}
	}
}
