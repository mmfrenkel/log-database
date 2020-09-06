#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "index.h"


/* Call to create a new empty Index() */
Index* init_index(int size) {
	Index *index = (Index*) malloc(sizeof(Index));
	if (index == NULL) {
		printf("Failed to allocate memory for index\n");
		return NULL;
	}

	// create contents and set all pointers to null
	Entry **contents = (Entry **) malloc(size * sizeof(Entry*));
	if (contents == NULL) {
		printf("Failed to allocate memory for index contents.\n");
		free(index);
		return NULL;
	}

	for (int i = 0; i < size; i++) {
		*(contents + i) = NULL;
	}
	index->capacity = size;
	index->positions_filled = 0;
	index->contents = contents;
	return index;
}

/* Inserts a new entry into hash table; where there are
 * collisions, this program uses chaining. */
int index_insert(Index *index, int key, char *value) {

	/* hash position where key should be */
	int position = hash(key, index->capacity);

	// see if the key is already in the hash map
	Entry *in_slot = *(index->contents + position);
	Entry *trav = in_slot;
	if (trav) {
		while (trav) {
			if (trav->key == key) {
				trav->value = value;
				return 0;
			}
			trav = trav->next;
		}
	}

	Entry *new_entry = (Entry*) malloc(sizeof(Entry));
	if (new_entry == NULL) {
		printf("Failed to allocate memory for new hash table entry.\n");
		return -1;
	}
	new_entry->key = key;
	new_entry->value = value;
	new_entry->next = NULL;

	// we didn't find the right key so add to chain, if there is one
	if (in_slot)
		new_entry->next = in_slot;

	// add the new entry to the index
	*(index->contents + position) = new_entry;

	// update number positions filled for accurate loading
	if (!in_slot)
		index->positions_filled++;
	return 0;
}

/* finds the value associated with a key in hash table,
 * in this case, the filename of segment where key is stored */
char* index_lookup(Index *index, int key) {

	int position = hash(key, index->capacity);
	Entry *entries = *(index->contents + position);

	while (entries) {
		if (entries->key == key) {
			return entries->value;
		}
		entries = entries->next;
	}
	// if we get to here, key is not in table
	printf("Key is not currently in index.\n");
	return NULL;
}

int index_remove(Index *index, int key) {
	int position = hash(key, index->capacity);
	Entry *entries = *(index->contents + position);
	Entry *trail = NULL;

	while (entries) {
		if (entries->key == key) {
			if (trail)
				trail->next = NULL;
			if (entries->next)
				trail->next = entries->next;

			*(index->contents + position) = trail;
			free(entries);
			return 0;
		}
		trail = entries;
		entries = entries->next;
	}

	// if we get to here, key is not in table so cannot delete it
	printf("Key to remove (%d) not in index", key);
	return -1;
}

bool index_is_full(Index *index) {
	if (index->positions_filled == index->capacity) {
		return true;
	} else if (index->positions_filled / index->capacity > LOAD_FACTOR) {
		printf("Warning: Index is getting full.\n");
	}
	return false;
}

/* Hash the key value, using the division method. It
 * is simple and fast, though unsophisticated. */
int hash(int key, int m) {
	if (key < 0)
		return -1 * (key % m);

	return key % m;
}
