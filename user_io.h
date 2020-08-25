#ifndef BT_USER_IO_H
#define BT_USER_IO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lsm_tree.h"

void print_user_options();

Submission* next_submission(void);

int get_key();

char* get_value();

#endif

