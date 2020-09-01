#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include "wal.h"


/* Initializes the WAL log*/
FILE * init_wal(char *filename) {
	FILE *wal;

	if ((wal = fopen(filename, "a")) == NULL) {
		printf("Failed to open WAL log.\n");
		return NULL;
	}
	return wal;
}

/* Writes key, value pair to write ahead log */
int submission_to_wal(FILE *wal, int action, int key,
		              char *value, int max_line_size, bool flush_immediately) {

	char to_write[max_line_size];
	sprintf(to_write, "%d - %d:%d,%s", (int) time(0), action, key, value);
	fprintf(wal, "%s\n", to_write);

	// if user specifies, flush immediately to disk
	if (flush_immediately) {
		int error = fflush(wal);
		if (error) {
			printf("Could not flush WAL to disk (Error: %d).\n", error);
			return error;
		}
	}
	return 0;
}

