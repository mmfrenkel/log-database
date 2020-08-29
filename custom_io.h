#ifndef CUSTOM_IO_H
#define CUSTOM_IO_H

#include <stdio.h>
#include "memtable.h"

#define MARKER -1
#define BUF_DATA_SIZE 50

int serialize_memtable(Memtable *memtable, char *filename);

MNode* deserialize_preorder(FILE *fp);

void kv_to_wal(char *wal, int key, char *value);

char* memtable_to_sorted_strings_table(Memtable *memtable);

char* readline_from_file(char *line, int buf_size, FILE *fp, bool rm_newline);

void delete_file_contents(char *filename);

int delete_file(char *filename);

#endif
