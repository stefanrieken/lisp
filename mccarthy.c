/**
 * Defines a set of special forms loosely styled after McCarthy's 1960 LISP paper.
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


// We use the old-fashioned definition:
// false == nil == empty list == null (?? !!)
// true == anything else, including the argument as passed
// N.B. of course this leaves ample room for null pointers
// if we do not secure other methods to recognize this special value
static Element atom(Node * arg, Environment * env)
{
	if (arg == NULL || get_type(eval(arg->value, env).ptr) >= VTYPE_LIST) return (Element) NULL;
	return arg->value;
}

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
    return lhsval; // 'true'
  else if (*(lhsval.intptr) == *(rhsval.intptr))
    return lhsval; // 'true'

  return (Element) NULL; // 'false'
}

static Element car(Node * arg, Environment * env)
{
 	if (arg == NULL) return (Element) NULL;
 	Node * val = eval(arg->value, env).node;
 	return val->value;
}

static Element cdr(Node * arg, Environment * env)
{
 	if (arg == NULL) return (Element) NULL;
 	Node * val = eval(arg->value, env).node;
 	return val->next;
}

static Element cons(Node * args, Environment * environment)
{
 	if (args == NULL) return (Element) NULL;
 	if (args->next.ptr == NULL || get_type(args->next.ptr) != VTYPE_LIST) return (Element) NULL;
 	Node * next = args->next.node;
 	Node * result = new (Node, VTYPE_LIST);
 	result->value = eval(args->value, environment);
 	result->next = eval(next->value, environment);
 	return (Element) result;
}

static Element cond(Node * arg, Environment * env)
{
	if (arg == NULL || get_type(arg->value.ptr) != VTYPE_LIST) return (Element) NULL;

	while (arg != NULL) {
	 	Node * cond = arg->value.node;
	 	if (cond== NULL) return (Element) NULL;

		// We first check the boolean outcome, where true is defined as "not null";
		// Then we just check if the associated code block is actually there :)
		if (eval(cond->value, env).ptr != NULL && cond->next.ptr != NULL) {
			Node * expr = cond->next.node;
			return eval(expr->value, env);
		}
		arg = arg->next.node;
	}
	return (Element) NULL;
}

static Element lambda(Node * arg, Environment * environment)
{
 	Node * node = new(Node, VTYPE_LIST);
 	node->value.ptr = environment;
 	set_type(node->value.ptr, VTYPE_LAMBDA);
 	node->next.node = arg;
 	return (Element) node;
}

static Element label(Node * arg, Environment * environment)
{
 	if (arg == NULL || get_type(arg->value.ptr) != VTYPE_ID) return (Element) NULL;
 	if (arg->next.ptr == NULL) return (Element) NULL;
 	Node * val = arg->next.node;
 	Element value = eval(val->value, environment);
 	add_variable(environment, arg->value.str, value);
 	return (Element) value;
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

static Element quote (Node * args, Environment * environment)
{
 	return args->value;
}

static inline Element typify(special_form form)
{
 	special_form * type = new (special_form, VTYPE_SPECIAL);
 	(* type) = form;
 	return (Element) type;
}

void register_specials(Environment * env)
{
 	// The McCarthy functions
 	add_variable(env, "quote", typify(quote));
 	add_variable(env, "atom", typify(atom));
 	add_variable(env, "eq", typify(eq));
 	add_variable(env, "cond", typify(cond));
 	add_variable(env, "car", typify(car));
 	add_variable(env, "cdr", typify(cdr));
 	add_variable(env, "cons", typify(cons));
 	add_variable(env, "lambda", typify(lambda));
 	add_variable(env, "label", typify(label));
 	add_variable(env, "list", typify(list));
}
