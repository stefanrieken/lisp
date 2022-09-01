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

/**
 * These are McCarthy's special forms.
 */

// We use the old-fashioned definition:
// false == nil == empty list == null (?? !!)
// true == anything else, including the argument as passed
// N.B. of course this leaves ample room for null pointers
// if we do not secure other methods to recognize this special value
static void * atom(Node * arg, Environment * env)
{
	if (arg == NULL || get_type(eval(arg->value, env)) >= LIST) return NULL;
	return arg->value;
}

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

static void * car(Node * arg, Environment * env)
{
 	if (arg == NULL) return NULL;
 	Node * val = (Node *) eval(arg->value, env);
 	return val->value;
}

static void * cdr(Node * arg, Environment * env)
{
 	if (arg == NULL) return NULL;
 	Node * val = (Node *) eval(arg->value, env);
 	return val->next;
}

static void * cons(Node * args, Environment * environment)
{
 	if (args == NULL) return NULL;
 	if (args->next == NULL || get_type(args->next) != LIST) return NULL;
 	Node * next = (Node *) args->next;
 	Node * result = new (Node, LIST);
 	result->value = eval(args->value, environment);
 	result->next = eval(next->value, environment);
 	return result;
}

static void * cond(Node * arg, Environment * env)
{
	if (arg == NULL || get_type(arg->value) != LIST) return NULL;

	while (arg != NULL) {
	 	Node * cond = arg->value;
	 	if (cond== NULL || get_type(cond->value) != LIST) return NULL;

		if (eval(cond->value, env) != NULL && cond->next != NULL) {
			Node * expr = cond->next;
			Node * result = eval(expr->value, env);
			return result;
		}
		arg = arg->next;
	}
	return NULL;
}

static void * lambda(Node * arg, Environment * environment)
{
 	Node * node = new(Node, LIST);
 	node->value = environment;
 	set_type(node->value, LAMBDA);
 	node->next = arg;
 	return node;
}

static void * label(Node * arg, Environment * environment)
{
 	if (arg == NULL || get_type(arg->value) != ID) return NULL;
 	if (arg->next == NULL) return NULL;
 	Node * val = (Node *) arg->next;
 	void * value = eval(val->value, environment);
 	add_variable(environment, arg->value, value);
 	return value;
}

// This one is not usually quoted as a required primitive
// but I can't for the life of me imagine how otherwise to
// produce a list with evaluated contents with only the other
// primitives mentioned above.
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

static void * quote (Node * args, Environment * environment)
{
 	return args->value;
}

static inline void * typify(special_form form)
{
 	special_form * type = new (special_form, SPECIAL);
 	(* type) = form;
 	return type;
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
