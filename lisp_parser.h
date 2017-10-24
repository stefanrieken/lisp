#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "lisp_lexer.h"

/* extern */ struct Environment;

typedef struct Node
{
	void * value;
	struct Node * next; // TODO in pure LISP this may also be any type
} Node;

extern Node * parse_list();
extern void print_list(Node * list);
extern void println_list(Node * list);
