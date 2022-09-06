/**
 * Defines a set of special forms loosely styled after Scheme.
 */
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include "structs.h"
#include "../tmmh/tmmh.h"
#include "eval.h"

#define new(Type, TYPE) (Type *) allocate_type (sizeof(Type), TYPE)

static inline void * allocate_type(int size, int type)
{
	void * bla = allocate(memory, size, false);
	set_type(bla, type);
	return bla;
}

static inline bool streq(char * str1, char * str2)
{
	return strcmp(str1, str2) == 0;
}

static void * iff(Node * arg, Environment * env)
{
	if (arg == NULL || arg->next == NULL) return NULL;

	// This is the actual if-statement evaluation and if-else block selection
	Node * expr = (eval(arg->value, env) != NULL) ? arg->next : ((Node *) arg->next)->next;
	
	if (expr != NULL) {
		return eval(expr->value, env);
	}

	return NULL;
}

// TODO check if there are subtle differences between McCarthy and Scheme here
static void * lambda(Node * arg, Environment * environment)
{
 	Node * node = new(Node, LIST);
 	node->value = environment;
 	set_type(node->value, LAMBDA);
 	node->next = arg;
 	return node;
}

static void * set(Node * arg, Environment * environment)
{
 	if (arg == NULL) return NULL;
 	void * name = eval(arg->value, environment);
 	if (name == NULL) return NULL;
 	if (get_type(name) != ID) return NULL;

 	if (arg->next == NULL) return NULL;
 	Node * val = (Node *) arg->next;
 	void * value = eval(val->value, environment);
 	add_variable(environment, name, value);
 	return value;
}

static void * quote (Node * args, Environment * environment)
{
 	return args->value;
}

// The above may be a theoretical minimum, but hey,
// let's throw in at least a few more practical ones.

// This is McCarthy's eq; there's all kinds of equivalence
// checks in LISP and Scheme of which I have yet to make a
// selection.

// Only on atoms!
static void * eq(Node * lhs, Environment * env)
{
  if (lhs == NULL || lhs->value == NULL) return NULL;
  Node * rhs = lhs->next;
 	if (rhs == NULL || rhs->value == NULL) return NULL;

  void * lhsval = eval(lhs->value, env);
  void * rhsval = eval(rhs->value, env);

  if (get_type(lhsval) >= LIST) return NULL;
  if (get_type(rhsval) >= LIST) return NULL;

  if (get_type(lhsval) != get_type(rhsval)) return NULL;
  if ((get_type(lhsval) == STRING || get_type(lhsval) == ID)
      && streq((char *) lhs->value, (char *) rhsval))
    return lhsval; // 'true'
  else if ((*(intptr_t*) lhsval) == (*(intptr_t*) rhsval))
    return lhsval; // 'true'

  return NULL; // 'false'
}

// This one is not usually quoted as a required primitive
// but it makes producing a list with evaluated contents 
// a lot easier compared to repeated cons'ing.
static void * list(Node * args, Environment * environment)
{
	if (args == NULL || args->value == NULL) return NULL;
	Node * result = new(Node, LIST);
	result->value = eval(args->value, environment);
	if (args->next == NULL || get_type(args->next) != LIST) {
		result->next = args->next;
	} else {
		result->next = list(args->next, environment);
	}
	return result;
}

// So let's throw in 'his' atom as well for now.
static void * atom(Node * arg, Environment * env)
{
	if (arg == NULL || get_type(eval(arg->value, env)) >= LIST) return NULL;
	return arg->value;
}

static inline void * typify(special_form form)
{
 	special_form * type = new (special_form, SPECIAL);
 	(* type) = form;
 	return type;
}

void register_specials(Environment * env)
{
 	// Scheme's bare minimum (and then some)
 	add_variable(env, "quote", typify(quote));
 	add_variable(env, "atom", typify(atom));
 	add_variable(env, "eq", typify(eq));
 	add_variable(env, "if", typify(iff));
 	add_variable(env, "lambda", typify(lambda));
 	add_variable(env, "set!", typify(set));
 	add_variable(env, "list", typify(list));
}
