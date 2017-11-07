#include <stdio.h>
#include <stdbool.h>
#include "buffered_read.h"
#include "parse_string.h"

extern int get_non_whitespace_char();
extern void * parse_label_or_number (int c, int radix);
extern void skip_line();
