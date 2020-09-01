#ifndef MEMTABLE_H
#define MEMTABLE_H

#include <stdbool.h>

#define MAX_KEYS_IN_TREE 3     // max # keys held in tree before flush to segment
#define NULL_MARKER -1

typedef struct memtable_node {
	int key;
	char *data;
	struct memtable_node *left_child;
	struct memtable_node *right_child;
} MNode;

typedef struct binary_tree {
	MNode *root;
	int count_keys;
} Memtable;

Memtable* init_memtable();

bool is_full(Memtable *memtable);

int insert(Memtable *memtable, int key, char *data);

int delete(Memtable *memtable, int key, bool hard_delete, char *tombstone);

MNode* create_node(int key, char *data);

MNode* search(Memtable *memtable, int key);

void print_memtable(Memtable *memtable, char *print_type);

void clear_memtable(Memtable *memtable);

void delete_memtable(Memtable *memtable);

int serialize_memtable(Memtable *memtable, char *filename);

MNode* deserialize_memtable(FILE *fp, int buffer_size);

#endif
