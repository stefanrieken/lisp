#include "lisp_parser.h"

typedef struct Variable {
	char * name;
	Node * value;
	struct Variable * next;
} Variable;

typedef struct Environment {
	struct Environment * parent;
	Variable * variables;
} Environment;

extern Node * eval (Node * expression, Environment * environment);
extern Node * apply (Node * expression, Environment * environment);

