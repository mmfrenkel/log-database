#ifndef CUSTOM_IO_H
#define CUSTOM_IO_H

#include <stdio.h>

#include "memtable.h"

char** init_segment_list(int num_segments);

void free_segment_list(char **segments, int num_segments);

int serialize_memtable(Memtable *memtable, char *filename);

int compact_segments(char **segment_files, int num_segments,
		char *new_segment_name, int line_size, char *tombstone);

char* search_segment(char *filename, int key, int line_size);

MNode* deserialize_preorder(FILE *fp, int buffer_size);

int memtable_to_segment(Memtable *memtable, char *filename);

char* readline_from_segment(char *line, int buf_size, FILE *fp,
		bool rm_newline);

int delete_segment(char *filename);

#endif
