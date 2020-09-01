#include <stdio.h> 
#include <stdlib.h>
#include "error.h"
#include "user_io.h"
#include "lsm_tree.h"
#include "segment.h"

int main(int argc, char *argv[]) {
	printf("Database System Started!\n");
	LSM_Tree *lsm_tree = init_lsm_tree();

	while (1) {
		Submission *user_submission = next_submission();

		if (user_submission == NULL) {
			printf("There was an issue with your submission, try again.\n");
		} else if (user_submission->action == EXIT) {
			printf("Shutting down...\n");
			shutdown_lsm_system(lsm_tree);
			exit(0);
		} else {
			int error = handle_submission(lsm_tree, user_submission);
			if (error) {
				printf("LSM Tree system returned error %d", error);
			}
		}
		show_status(lsm_tree);
	}
}
