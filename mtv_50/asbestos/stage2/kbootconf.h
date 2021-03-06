/*  main.c - kboot.conf parsing

Copyright (C) 2010  Hector Martin "marcan" <hector@marcansoft.com>

This code is licensed to you under the terms of the GNU GPL, version 2;
see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
*/

#ifndef KBOOTCONF_H
#define KBOOTCONF_H

#include "types.h"
#include "kernel.h"

#define MAX_KBOOTCONF_SIZE 16384
#define MAX_KERNELS 64

struct kbootkernel {
	char *label;
	char *kernel;
	char *initrd;
	char *root;
	char *parameters;
};

struct kbootconf {
	int timeout;
	char *msgfile;
	int default_idx;
	int num_kernels;
	struct kbootkernel kernels[MAX_KERNELS];
};

extern char conf_buf[MAX_KBOOTCONF_SIZE];

extern struct kbootconf conf;

int kbootconf_parse(void);

#endif