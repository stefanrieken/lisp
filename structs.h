
#ifndef STRUCTS_H
#define STRUCTS_H

#include <stdint.h>
#include <stdbool.h>

typedef enum ValueType {
	VTYPE_NONE,
	VTYPE_INT,
	VTYPE_STRING,
	VTYPE_ID, // Label
	VTYPE_LIST,
	VTYPE_LAMBDA,
	VTYPE_SPECIAL,
	VTYPE_PRIMITIVE,
	VTYPE_ENVIRONMENT,
	VTYPE_VARIABLE
} ValueType;


// The special / primitive callbacks suffer from recursive definition.
struct Node;
struct Environment;
union Element;
typedef union Element (* special_form) (struct Node * args, struct Environment * env);
typedef union Element (* primitive_form) (struct Node * args, struct Environment * env);

// Defining the node LHS and RHS as unions is a lot cleaner
// compared to casting void pointers all over the place.
typedef union Element {
  void * ptr;
  char * str;
  struct Node * node;
  intptr_t * intptr; // that's a pointer to a pointer-sized int...
  intptr_t as_int;
  special_form * special;
  primitive_form * primitive;
} Element;

typedef struct Variable {
	char * name;
	Element value;
	struct Variable * next;
} Variable;

typedef struct Environment {
	struct Environment * parent;
	Variable * variables;
} Environment;

typedef struct Node {
  Element value;
  Element next;
} Node;

extern Variable * add_variable(Environment * environment, char * name, Element value);
extern Variable * find_variable(Environment * environment, char * name, bool recurse);
extern Variable * set_variable(Environment * environment, char * name, Element value, bool recurse);

extern void * memory;
#endif
