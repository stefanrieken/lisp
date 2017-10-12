#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "lisp_lexer.h"

typedef enum value_type
{
	LIST,
	STRING
} ValueType;

typedef struct Node
{
	union
	{
		char * string_value;
		struct Node * list_value;
	};
	struct Node * next;
	ValueType value_type;
} Node;

#define BUFFER_SIZE 255
char buffer[BUFFER_SIZE];

extern Node * parse_list();
extern void print_list(Node * list);

