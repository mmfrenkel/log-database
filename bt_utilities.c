#include <stdio.h>
#include <stdlib.h>

void die(char *msg) {
	fprintf(stderr, "%s", msg);
	exit(1);
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

