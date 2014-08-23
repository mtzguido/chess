#ifndef __OPTS_H
#define __OPTS_H

#include "common.h"

#ifdef FIXOPTS
static int parse_opt(int argc, char **argv) { return 1; }
#else
int parse_opt(int argc, char **argv);
#endif

void print_help(char *progname);

#endif
