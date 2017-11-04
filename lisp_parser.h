#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "lisp_lexer.h"

/* extern */ struct Environment;

typedef struct Node
{
	void * value;
	void * next;
} Node;


extern void * parse_value();
extern Node * parse_list();

extern void print_value(void * list);
extern void println_value(void * list);

