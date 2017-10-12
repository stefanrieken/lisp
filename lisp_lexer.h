#include <stdio.h>
#include <stdbool.h>
#include "buffered_read.h"
#include "parse_string.h"

extern int parse_word(char * buffer, int buffer_length);
extern int parse_zero_ending_word(char * buffer, int buffer_length);
extern void discard_initial_opening_bracket();
