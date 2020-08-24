#ifndef CUSTOM_IO_H
#define CUSTOM_IO_H

#include <stdio.h>
#include "binary_tree.h"

#define MARKER -1
#define BUF_DATA_SIZE 50

void serialize_tree(Binary_Tree *tree, char *filename);

BT_Node* deserialize_preorder(FILE *fp);

void kv_to_wal(char *wal, int key, char *value);

char* tree_to_sorted_strings_table(Binary_Tree *tree);

void delete_file_contents(char *filename);

void delete_file(char *filename);

#endif
