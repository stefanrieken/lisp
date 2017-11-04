#include "lisp_parser.h"

typedef struct Variable {
	char * name;
	void * value;
	struct Variable * next;
} Variable;

typedef struct Environment {
	struct Environment * parent;
	Variable * variables;
} Environment;

extern void * eval (void * expression, Environment * environment);
extern Node * apply (Node * expression, Environment * environment);

