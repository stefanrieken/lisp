#include <string.h>
#include <stdio.h>

#include "eval.h"
#include "../tmmh/tmmh.h"

#define new(Type, TYPE) (Type *) allocate_type (sizeof(Type), TYPE)

static inline void * allocate_type(int size, int type)
{
	void * bla = allocate(size, false);
	set_type(bla, type);
	return bla;
}

// 'label' == variable | function
static void * find_label(char * name, Environment * environment)
{
	Variable * var = environment->variables;
	while (var != NULL)
	{
		if (strcmp(name, var->name) == 0) return var->value;
		var = var->next;
	}

	if (environment->parent != NULL) return find_label(name, environment->parent);
	return NULL;
}

// args contains the *expressions that result in the arg values after evaluation*
static Environment * extract_args(Node * arg_names, Node * arg_exps, Environment * lambda_env)
{
	Environment * function_env = new(Environment, ENVIRONMENT);
	function_env->parent = lambda_env;
	function_env->variables = NULL;

	while (arg_names != NULL)
	{
		// TODO use add_variable

		if (get_type(arg_names->value) != ID) return NULL;
		Variable * arg = new(Variable,VARIABLE);
		arg->name = (char *) arg_names->value;
		arg->value = eval(arg_exps->value, lambda_env);
		// reverse insert is easier
		arg->next = function_env->variables;
		function_env->variables = arg;

		arg_names = arg_names->next;
		arg_exps = arg_exps->next;
	}

	return function_env;
}

void * eval (void * expression, Environment * environment)
{
	// Idea: only apply expression when we can match it to a function,
	// otherwise leave / return it unused

	int type = get_type(expression);
	if (type < ID) return expression;
	if (type == ID) return find_label(expression, environment);

	return apply(expression, environment);
}

void * apply (Node * expression, Environment * environment)
{
	if (expression == NULL || get_type(expression->value) != ID) return expression;

	// Analyze the call
	// -name
	char * name = (char *) expression->value;
	// -arg expressions
	Node * arg_exps = expression->next;

	// Analyze the function definition
	Node * function = find_label(name, environment);
	if (function == NULL) {
		printf ("can't find %s\n", name);
		return NULL;
	}
	// TODO 1) does special belong in 'apply'? 2) here? 3) should it and lambda have type marker at same level?
	if (get_type(function) == SPECIAL) {
		special_form * form = (special_form *) function;
		return (* form)(arg_exps, environment);
	}
	// else - lambda
	if (function->value == NULL || get_type(function->value) != LAMBDA) return NULL;

	// - environment
	Environment * lambda_env = (Environment *) function->value;
	// - arg names
	Node * arg_names_list = function->next;
	if (arg_names_list == NULL || get_type(arg_names_list->value) != LIST) return NULL;
	Node * arg_names = arg_names_list->value;
	// - body
	Node * body_forms = arg_names_list->next;
	if (body_forms != NULL && get_type(body_forms->value) == STRING)
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

