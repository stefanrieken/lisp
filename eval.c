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
static Element find_label(char * name, Environment * environment)
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
	return (Element) NULL;
}

static Node * eval_and_chain(Node * arg_exps, Environment * lambda_env)
{
	if (arg_exps == NULL) return NULL;
	Node * node = new(Node, VTYPE_LIST);
	node->value = eval(arg_exps->value, lambda_env);
	node->next.node = eval_and_chain(arg_exps->next.node, lambda_env);
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
		if (get_type(arg_names->value.ptr) != VTYPE_ID) return NULL;
		add_variable(function_env, arg_names->value.str, eval(arg_exps->value, lambda_env));

		arg_names = arg_names->next.node;
		arg_exps = arg_exps->next.node;
	}
	if (arg_names != NULL && get_type(arg_names) == VTYPE_ID)
	{
		// &rest remainder var in cdr
		add_variable(function_env, (char *) arg_names, (Element) eval_and_chain(arg_exps, lambda_env));
	}

	return function_env;
}

Element eval (Element expression, Environment * environment)
{
	// Idea: only apply expression when we can match it to a function,
	// otherwise leave / return it unused

	int type = get_type(expression.ptr);
	if (type < VTYPE_ID) return expression;
	if (type == VTYPE_ID) return find_label(expression.str, environment);

	return apply(expression.node, environment);
}

Element apply (Node * expression, Environment * environment)
{
	if (expression == NULL) return (Element) expression;

	// Function definition. Eval'ing this to function selection by expression: ((foo x) y)
	Element function = eval(expression->value, environment); // find_label(name, environment);

	if (function.ptr == NULL) {
		printf ("can't find %s\n", (char *) expression->value.str); // TODO this isn't always just an ID!
		return (Element) NULL;
	}
	// -arg expressions
	Node * arg_exps = expression->next.node;

	int function_type = get_type(function.ptr);

	// once more, gracefully handle data being invoked
	if (function_type == VTYPE_ID) {
		// most likely scenario:
		// - we had an expression for a function definition: ((foo x) y)
		// - this eval'ed (above) to an ID
		return find_label(function.str, environment);
	}
	if (function_type < VTYPE_ID) return function;

	// TODO 1) does special belong in 'apply'? 2) here? 3) should it and lambda have type marker at same level?
	if (function_type == VTYPE_SPECIAL) {
		return (*(function.special))(arg_exps, environment);
	} else if (function_type == VTYPE_PRIMITIVE) {
		// as above, but "as a service" we now pre-evaluate all args
		return (*(function.primitive))(eval_and_chain(arg_exps, environment), environment);
	}
	// else - lambda
	Node * f = function.node;
	if (f->value.ptr == NULL || get_type(f->value.ptr) != VTYPE_LAMBDA) return (Element) NULL;

	// - environment
	Environment * lambda_env = (Environment *) f->value.ptr;

	// - arg names
	Node * arg_names_list = f->next.node;
	if (arg_names_list == NULL) {
		return (Element) NULL;
	}
	Node * arg_names = arg_names_list->value.node;
	// - body
	Node * body_forms = arg_names_list->next.node;
	if (body_forms != NULL && get_type(body_forms->value.ptr) == VTYPE_STRING)
		body_forms = body_forms->next.node; // skip def comment

	Environment * function_env = extract_args(arg_names, arg_exps, lambda_env);
	Node * result = NULL;
	while (body_forms != NULL)
	{
		//if (get_type(body_forms->value) != LIST) return NULL;
		result = eval(body_forms->value, function_env).node;
		body_forms = body_forms->next.node;
	}

	return (Element) result;
}
