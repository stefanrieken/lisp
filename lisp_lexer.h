#include <stdio.h>
#include <stdbool.h>
#include "buffered_read.h"
#include "parse_string.h"

typedef enum value_type
{
	UNTYPED,
	INT,
	STRING,
	ID,
	LIST,
	ENVIRONMENT
} ValueType;

extern char * parse_zero_ending_word();
extern void discard_initial_opening_bracket();
