#ifndef CUSTOM_IO_H
#define CUSTOM_IO_H

#include <stdio.h>

#include "memtable.h"

int serialize_memtable(Memtable *memtable, char *filename);

MNode* deserialize_preorder(FILE *fp, int buffer_size);

char* memtable_to_segment(Memtable *memtable);

char* readline_from_segment(char *line, int buf_size, FILE *fp, bool rm_newline);

int delete_segment(char *filename);

#endif
