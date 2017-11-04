#include "eval.h"
#include "../tmmh/tmmh.h"

#define new(Type, TYPE) (Type *) allocate_type (sizeof(Type), TYPE)

static inline void * allocate_type(int size, int type)
{
	void * bla = allocate(size, false);
	set_type(bla, type);
	return bla;
}

static inline bool streq(char * str1, char * str2)
{
	return strcmp(str1, str2) == 0;
}

// 'label' == variable | function
static Node * find_label(char * name, Environment * environment)
{
	Variable * var = environment->variables;
	while (var != NULL)
	{
		if (streq(name, var->name)) return var->value;
		var = var->next;
	}
	return NULL;
}

static Node * find_function(char * name, Environment * environment)
{
	Node * val = find_label(name, environment);
	if (val != NULL && get_type(val->value) == LAMBDA) return val;
	else return NULL;
}

// We use the old-fashioned definition:
// false == nil == empty list == null (?? !!)
// true == anything else, including the argument as passed
// N.B. of course this leaves ample room for null pointers
// if we do not secure other methods to recognize this special value
static void * atom(Node * arg)
{
	if (arg == NULL || get_type(arg->value) >= LIST) return NULL;
	return arg->value;
}

// Only on atoms!
// TODO: 1) check for type 2) return 'true' if both values are null (~= empty list)
static void * eq(Node * lhs)
{
	if (lhs == NULL || get_type(lhs->value) >= LIST) return NULL;

	Node * rhs = lhs->next;
	if (rhs == NULL || get_type(rhs->value) >= LIST) return NULL;

	if (streq((char *) lhs->value, (char *) rhs->value)) return lhs; // 'true'
	return NULL; // 'false'
}

static void * car(Node * arg)
{
	if (arg == NULL || get_type(arg->value) != LIST) return NULL;
	Node * val = (Node *) arg->value;
	return val->value;
}

static void * cdr(Node * arg)
{
	if (arg == NULL || get_type(arg->value) != LIST) return NULL;
	Node * val = (Node *) arg->value;
	return val->next;
}

static Node * cons(Node * args, Environment * environment)
{
	if (args == NULL) return NULL;
	if (args->next == NULL || get_type(args->next) != LIST) return NULL;
	Node * next = (Node *) args->next;
	Node * result = new (Node, LIST);
	result->value = eval(args->value, environment);
	result->next = eval(next->value, environment);
	return result;
}

static Node * cond(Node * arg)
{
	// TODO
	return arg;
}

static Node * lambda(Node * arg, Environment * environment)
{
	Node * node = new(Node, LIST);
	node->value = environment;
	set_type(node->value, LAMBDA);
	node->next = arg;
	return node;
}

static Node * label(Node * arg, Environment * environment)
{
	if (arg == NULL || get_type(arg->value) != ID) return NULL;
	if (arg->next == NULL) return NULL;
	Node * val = (Node *) arg->next;

	Variable * variable = new(Variable, VARIABLE);
	variable->name = arg->value;
	variable->value = eval(val->value, environment);
	variable->next = NULL;

	if(environment->variables != NULL)
	{
		Variable * prev = environment->variables;
		while (prev->next != NULL) prev = prev->next;
		prev->next = variable;
	}
	else
		environment->variables = variable;
	return arg;
}

// This one is not usually quoted as a required primitive
// but I can't for the life of me imagine how otherwise to
// produce a list with evaluated contents with only the other
// primitives mentioned above.
static Node * list(Node * args, Environment * environment)
{
	if (args == NULL) return NULL;
	Node * result = new(Node, LIST);
	result->value = eval(args->value, environment);
	result->next = list(args->next, environment);

	return result;
}

// args contains the *expressions that result in the arg values after evaluation*
static Environment * extract_args(Node * arg_names, Node * arg_exps, Environment * lambda_env)
{
	Environment * function_env = new(Environment, ENVIRONMENT);
	function_env->parent = lambda_env;
	function_env->variables = NULL;

	while (arg_names != NULL)
	{
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

	Node * node = (Node *) expression;
	char * value = (char *) node->value;
	Node * args = node->next;

	if (streq(value, "atom")) return atom(args);
	else if (streq(value, "eq")) return eq(args);
	else if (streq(value, "car")) return car(args);
	else if (streq(value, "cdr")) return cdr(args);
	else if (streq(value, "cons")) return cons(args, environment);
	else if (streq(value, "quote")) return args->value; // duh!
	else if (streq(value, "cond")) return cond(args);
	else if (streq(value, "lambda")) return lambda(args, environment);
	else if (streq(value, "label")) return label(args, environment);
	else if (streq(value, "list")) return list(args, environment);
	else return apply(expression, environment);
}

Node * apply (Node * expression, Environment * environment)
{
	if (expression == NULL || get_type(expression->value) != ID) return expression;

	// Analyze the call
	// -name
	char * name = (char *) expression->value;
	// -arg expressions
	Node * arg_exps = expression->next;

	// Analyze the function definition
	Node * function = find_function(name, environment);
	// - environment
	if (function == NULL || get_type(function->value) != LAMBDA) return NULL;
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
		if (get_type(body_forms->value) != LIST) return NULL;
		result = eval(body_forms->value, function_env);
		body_forms = body_forms->next;
	}

	return result;
}

