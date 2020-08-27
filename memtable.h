#ifndef MEMTABLE_H
#define MEMTABLE_H

#include <stdbool.h>

#define DEL_MARKER "*-*"   // special marker, denoting a key was deleted

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

int insert(Memtable *memtable, int key, char *data);

int delete(Memtable *memtable, int key, bool hard_delete);

MNode* create_node(int key, char *data);

MNode* search(Memtable *memtable, int key);

void print_memtable(Memtable *memtable, char *print_type);

void clear_memtable(Memtable *memtable);

void delete_memtable(Memtable *memtable);

#endif
