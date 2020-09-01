#ifndef CUSTOM_WAL_H
#define CUSTOM_WAL_H

FILE * init_wal(char *filename);

int submission_to_wal(FILE *wal, int action, int key,
		              char *value, int max_line_size, bool flush_immediately);

#endif
