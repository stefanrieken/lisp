
#ifndef STRUCTS_H
#define STRUCTS_H

#include <stdbool.h>

typedef enum value_type {
	UNTYPED,
	INT,
	STRING,
	ID, // Label
	LIST,
	LAMBDA,
	SPECIAL,
	PRIMITIVE,
	ENVIRONMENT,
	VARIABLE
} ValueType;

typedef struct Variable {
	char * name;
	void * value;
	struct Variable * next;
} Variable;

typedef struct Environment {
	struct Environment * parent;
	Variable * variables;
} Environment;

typedef struct Node {
	void * value;
	void * next;
} Node;

typedef void * (* special_form) (Node * args, Environment * env);
typedef void * (* primitive_form) (Node * args, Environment * env);

extern Variable * add_variable(Environment * environment, char * name, void * value);
extern Variable * find_variable(Environment * environment, char * name, bool recurse);
extern Variable * set_variable(Environment * environment, char * name, void * value, bool recurse);

extern void * memory;
#endif
