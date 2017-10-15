#include "eval.h"

#define new(bla) (bla *) malloc(sizeof(bla))

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
	if (val != NULL && val->value_type == ENVIRONMENT) return val;
	else return NULL;
}

// Mm, I feel that this should be inside 'eval'
// NB: we work on the node's single value so we have to sever it loose from any 'next'
Node * resolve_or_eval(Node * expression, Environment * environment)
{
	// here we get into some trouble because we can't yet discern
	// strings from words, so we just try to lookup all words
	if (expression->value_type == STRING)
	{
		Node * label_value = find_label(expression->string_value, environment);

		if (label_value != NULL) return label_value;
		else
		{
			Node * result = new (Node);
			memcpy(result, expression, sizeof(Node));
			result->next = NULL;
			return result;
		}
	}
	else return eval(expression->list_value, environment);
}

// We use the old-fashioned definition:
// false == nil == empty list == null (?? !!)
// true == anything else, including the argument as passed
// N.B. of course this leaves ample room for null pointers
// if we do not secure other methods to recognize this special value
static Node * atom(Node * arg)
{
	if (arg == NULL || arg->value_type >= LIST) return NULL;
	return arg;
}

// Only on atoms!
static Node * eq(Node * lhs)
{
	if (lhs == NULL || lhs->value_type >= LIST) return NULL;

	Node * rhs = lhs->next;
	if (rhs == NULL || rhs->value_type >= LIST) return NULL;

	if (streq(lhs->string_value, rhs->string_value)) return lhs; // 'true'
	return NULL; // 'false'
}

static Node * car(Node * arg)
{
	if (arg == NULL || arg->value_type != LIST) return NULL;
	Node * val = arg->list_value;

	Node * node = new(Node);
	memcpy(node, val, sizeof(Node));
	node->next = NULL;
	return node;
}

static Node * cdr(Node * arg)
{
	if (arg == NULL || arg->value_type != LIST) return NULL;
	Node * val = arg->list_value;
	return val->next;
}

static Node * cons(Node * car, Environment * environment)
{
	if (car == NULL) return NULL;
	Node * cdr = car->next;
	if (cdr == NULL) return NULL;
	Node * node = resolve_or_eval(car, environment);
	if (cdr->list_value == NULL) return node; // this un-quoted form is technically wrong
	Node * next = resolve_or_eval(cdr, environment);
	if (next != NULL && (next->value_type != LIST || next->list_value != NULL)) node->next = next;
	return node;
}

static Node * cond(Node * arg)
{
	// TODO
	return arg;
}

static Node * lambda(Node * arg, Environment * environment)
{
	Node * node = new(Node);
	node->value_type = ENVIRONMENT;
	node->environment = environment;
	node->next = arg;
	return node;
}

static Node * label(Node * arg, Environment * environment)
{
	if (arg == NULL || arg->value_type != STRING) return NULL;

	Variable * variable = new(Variable);
	variable->name = arg->string_value;
	variable->value = resolve_or_eval(arg->next, environment);

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
	Node * first_list_item = NULL;
	Node * last_list_item = NULL;

	while (args != NULL)
	{
		Node * result = resolve_or_eval(args, environment);
		Node * list_item = new(Node);
		memcpy(list_item, result, sizeof(Node));
		result->next = NULL;

		if (first_list_item == NULL) first_list_item = list_item;
		if (last_list_item == NULL) last_list_item = list_item;
		else last_list_item->next = list_item;

		// New list item could be many nodes long
		while (last_list_item->next != NULL)
			last_list_item = last_list_item->next;

		args = args->next;
	}

	return first_list_item;
}

// args contains the *expressions that result in the arg values after evaluation*
static Environment * extract_args(Node * arg_names, Node * arg_exps, Environment * lambda_env)
{
	Environment * function_env = new(Environment);
	function_env->parent = lambda_env;

	while (arg_names != NULL)
	{
		if (arg_names->value_type != STRING) return NULL;
		Variable * arg = new(Variable);
		arg->name = arg_names->string_value;
		arg->value = resolve_or_eval(arg_exps, lambda_env);

		// reverse insert is easier
		arg->next = function_env->variables;
		function_env->variables = arg;

		arg_names = arg_names->next;
		arg_exps = arg_exps->next;
	}

	return function_env;
}

Node * eval (Node * expression, Environment * environment)
{
	// Idea: only apply expression when we can match it to a function,
	// otherwise leave / return it unused
	if (expression->value_type != STRING) return expression;

	char * value = expression->string_value;
	Node * args = expression->next;

	if (streq(value, "atom")) return atom(args);
	else if (streq(value, "eq")) return eq(args);
	else if (streq(value, "car")) return car(args);
	else if (streq(value, "cdr")) return cdr(args);
	else if (streq(value, "cons")) return cons(args, environment);
	else if (streq(value, "quote")) return args; // duh!
	else if (streq(value, "cond")) return cond(args);
	else if (streq(value, "lambda")) return lambda(args, environment);
	else if (streq(value, "label")) return label(args, environment);
	else if (streq(value, "list")) return list(args, environment);
	else return apply(expression, environment);
}

Node * apply (Node * expression, Environment * environment)
{
	if (expression == NULL || expression->value_type != STRING) return NULL;

	// Analyze the call
	// -name
	char * name = expression->string_value;
	// -arg expressions
	Node * arg_exps = expression->next;

	// Analyze the function definition
	Node * function = find_function(name, environment);
	// - environment
	if (function == NULL || function->value_type != ENVIRONMENT) return NULL;
	Environment * lambda_env = function->environment;
	// - arg names
	Node * arg_names_list = function->next;
	if (arg_names_list == NULL || arg_names_list->value_type != LIST) return NULL;
	Node * arg_names = arg_names_list->list_value;
	// - body
	Node * body_forms = arg_names_list->next;
	if (body_forms != NULL && body_forms->value_type == STRING)
		body_forms = body_forms->next; // skip def comment


	Environment * function_env = extract_args(arg_names, arg_exps, lambda_env);

	Node * result = NULL;
	while (body_forms != NULL)
	{
		if (body_forms->value_type != LIST) return NULL;
		result = eval(body_forms->list_value, function_env);
		body_forms = body_forms->next;
	}

	return result;
}

