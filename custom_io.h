#ifndef CUSTOM_IO_H
#define CUSTOM_IO_H

#include <stdio.h>
#include "binary_tree.h"

#define MARKER -1
#define BUF_DATA_SIZE 50

void save_tree_to_file(Binary_Tree *tree, char *filename);

void serialize_preorder(BT_Node *root, FILE *fp);

BT_Node* deserialize_preorder(FILE *fp);

void kv_to_wal(char *wal, int key, char *value);

void delete_file_contents(char *filename);

void add_to_file(char *filename, char *to_write);

#endif
