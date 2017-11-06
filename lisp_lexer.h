#include <stdio.h>
#include <stdbool.h>
#include "buffered_read.h"
#include "parse_string.h"

extern int get_non_whitespace_char();
extern char * parse_label (int c);
extern void skip_line();
