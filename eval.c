#include <string.h>
#include <stdio.h>

#include "eval.h"
#include "../tmmh/tmmh.h"

#define new(Type, TYPE) (Type *) allocate_type (sizeof(Type), TYPE)

static inline void * allocate_type(int size, int type)
{
	void * bla = allocate(memory, size, false);
	set_type(bla, type);
	return bla;
}

// 'label' == variable | function
static void * find_label(char * name, Environment * environment)
{
	Variable * var = environment->variables;
	while (var != NULL)
	{
		if (strcmp(name, var->name) == 0) {
			return var->value;
		}
		var = var->next;
	}

	if (environment->parent != NULL) {
		return find_label(name, environment->parent);
	}
	return NULL;
}

static Node * eval_and_chain(Node * arg_exps, Environment * lambda_env)
{
	if (arg_exps == NULL) return NULL;
	Node * node = new(Node, VTYPE_LIST);
	node->value = eval(arg_exps->value, lambda_env);
	node->next = eval_and_chain(arg_exps->next, lambda_env);
	return node;
}

// args contains the *expressions that result in the arg values after evaluation*
static Environment * extract_args(Node * arg_names, Node * arg_exps, Environment * lambda_env)
{
	Environment * function_env = new(Environment, VTYPE_ENVIRONMENT);
	function_env->parent = lambda_env;
	function_env->variables = NULL;

	while (arg_names != NULL && arg_exps != NULL && get_type(arg_names) == VTYPE_LIST)
	{
		if (get_type(arg_names->value) != VTYPE_ID) return NULL;
		add_variable(function_env, (char *) arg_names->value, eval(arg_exps->value, lambda_env));

		arg_names = arg_names->next;
		arg_exps = arg_exps->next;
	}
	if (arg_names != NULL && get_type(arg_names) == VTYPE_ID)
	{
		// &rest remainder var in cdr
		add_variable(function_env, (char *) arg_names, eval_and_chain(arg_exps, lambda_env));
	}

	return function_env;
}

void * eval (void * expression, Environment * environment)
{
	// Idea: only apply expression when we can match it to a function,
	// otherwise leave / return it unused

	int type = get_type(expression);
	if (type < VTYPE_ID) return expression;
	if (type == VTYPE_ID) return find_label(expression, environment);

	return apply(expression, environment);
}

void * apply (Node * expression, Environment * environment)
{
	if (expression == NULL) return expression;

	// Function definition. Eval'ing this to function selection by expression: ((foo x) y)
	void * function = eval(expression->value, environment); // find_label(name, environment);

	if (function == NULL) {
		printf ("can't find %s\n", (char *) expression->value); // TODO this isn't always just an ID!
		return NULL;
	}
	// -arg expressions
	Node * arg_exps = expression->next;

	int function_type = get_type(function);

	// once more, gracefully handle data being invoked
	if (function_type == VTYPE_ID) {
		// most likely scenario:
		// - we had an expression for a function definition: ((foo x) y)
		// - this eval'ed (above) to an ID
		return find_label((char *) function, environment);
	}
	if (function_type < VTYPE_ID) return function;

	// TODO 1) does special belong in 'apply'? 2) here? 3) should it and lambda have type marker at same level?
	if (function_type == VTYPE_SPECIAL) {
		special_form * form = (special_form *) function;
		return (* form)(arg_exps, environment);
	} else if (function_type == VTYPE_PRIMITIVE) {
		// as above, but "as a service" we now pre-evaluate all args
		primitive_form * form = (primitive_form *) function;
		return (* form)(eval_and_chain(arg_exps, environment), environment);
	}
	// else - lambda
	Node * f = (Node *) function;
	if (f->value == NULL || get_type(f->value) != VTYPE_LAMBDA) return NULL;

	// - environment
	Environment * lambda_env = (Environment *) f->value;

	// - arg names
	Node * arg_names_list = f->next;
	if (arg_names_list == NULL) {
		return NULL;
	}
	Node * arg_names = arg_names_list->value;
	// - body
	Node * body_forms = arg_names_list->next;
	if (body_forms != NULL && get_type(body_forms->value) == VTYPE_STRING)
		body_forms = body_forms->next; // skip def comment

	Environment * function_env = extract_args(arg_names, arg_exps, lambda_env);
	Node * result = NULL;
	while (body_forms != NULL)
	{
		//if (get_type(body_forms->value) != LIST) return NULL;
		result = eval(body_forms->value, function_env);
		body_forms = body_forms->next;
	}

	return result;
}
