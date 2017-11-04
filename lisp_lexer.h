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
	LAMBDA,
	LIST,
	ENVIRONMENT,
	VARIABLE
} ValueType;

extern int get_non_whitespace_char();
extern char * parse_label (int c);

