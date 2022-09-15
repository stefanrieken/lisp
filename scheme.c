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

static Element iff(Node * arg, Environment * env)
{
	if (arg == NULL || arg->next.ptr == NULL) return (Element) NULL;

	// This is the actual if-statement evaluation and if-else block selection
	Node * expr = (eval(arg->value, env).ptr != NULL) ? arg->next.node : arg->next.node->next.node;

	if (expr != NULL) {
		return eval(expr->value, env);
	}

	return (Element) NULL;
}

// TODO check if there are subtle differences between McCarthy and Scheme here
static Element lambda(Node * arg, Environment * environment)
{
 	Node * node = new(Node, VTYPE_LIST);
 	node->value.ptr = environment;
 	set_type(node->value.ptr, VTYPE_LAMBDA);
 	node->next.node = arg;
 	return (Element) node;
}

static Element set(Node * arg, Environment * environment)
{
 	if (arg == NULL) return (Element) NULL;
 	Element name = eval(arg->value, environment);
 	if (name.ptr == NULL) return (Element) NULL;
 	if (get_type(name.ptr) != VTYPE_ID) return (Element) NULL;

 	if (arg->next.node == NULL) return (Element) NULL;
 	Node * val = (Node *) arg->next.node;
 	Element value = eval(val->value, environment);
 	add_variable(environment, name.str, value);
 	return value;
}

static Element quote (Node * args, Environment * environment)
{
 	return args->value;
}

// The above may be a theoretical minimum, but hey,
// let's throw in at least a few more practical ones.

// This is McCarthy's eq; there's all kinds of equivalence
// checks in LISP and Scheme of which I have yet to make a
// selection.

// Only on atoms!
static Element eq(Node * lhs, Environment * env)
{
  if (lhs == NULL || lhs->value.ptr == NULL) return (Element) NULL;
  Node * rhs = lhs->next.node;
 	if (rhs == NULL || rhs->value.ptr == NULL) return (Element) NULL;

  Element lhsval = eval(lhs->value, env);
  Element rhsval = eval(rhs->value, env);

  if (get_type(lhsval.ptr) >= VTYPE_LIST) return (Element) NULL;
  if (get_type(rhsval.ptr) >= VTYPE_LIST) return (Element) NULL;

  if (get_type(lhsval.ptr) != get_type(rhsval.ptr)) return (Element) NULL;
  if ((get_type(lhsval.ptr) == VTYPE_STRING || get_type(lhsval.ptr) == VTYPE_ID)
      && streq(lhsval.str, rhsval.str))
    return (Element) lhsval; // 'true'
  else if (*(lhsval.intptr) == *(rhsval.intptr))
    return (Element) lhsval; // 'true'

  return (Element) NULL; // 'false'
}

// This one is not usually quoted as a required primitive
// but it makes producing a list with evaluated contents
// a lot easier compared to repeated cons'ing.
static Element list(Node * args, Environment * environment)
{
	if (args == NULL || args->value.ptr == NULL) return (Element) NULL;
	Node * result = new(Node, VTYPE_LIST);
	result->value = eval(args->value, environment);
	if (args->next.ptr == NULL || get_type(args->next.ptr) != VTYPE_LIST) {
		result->next.ptr = args->next.ptr;
	} else {
		result->next = list(args->next.node, environment);
	}
	return (Element) result;
}

// So let's throw in 'his' atom as well for now.
static Element atom(Node * arg, Environment * env)
{
	if (arg == NULL || get_type(eval(arg->value, env).ptr) >= VTYPE_LIST) return (Element) NULL;
	return arg->value;
}

static inline Element typify(special_form form)
{
 	special_form * type = new (special_form, VTYPE_SPECIAL);
 	(* type) = form;
 	return (Element) type;
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
