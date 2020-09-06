#ifndef CUSTOM_INDEX_H
#define CUSTOM_INDEX_H

#include <stdio.h>
#include <stdbool.h>

#define LOAD_FACTOR 0.8

typedef struct table_entry {
	int key;
	char *value;
	struct table_entry *next;
} Entry;

typedef struct table {
	int capacity;
	int positions_filled;
	struct table_entry **contents;
} Index;

Index* init_index(int size);

int index_insert(Index *index, int key, char *value);

char* index_lookup(Index *index, int key);

int index_remove(Index *index, int key);

bool index_is_full(Index *index);

int hash(int key, int m);

#endif
