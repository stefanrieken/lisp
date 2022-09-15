#include "structs.h"
#include <stddef.h>
#include <string.h>
#include "../tmmh/tmmh.h"

#define new(Type, TYPE) (Type *) allocate_type (sizeof(Type), TYPE)

static inline void * allocate_type(int size, int type)
{
	void * bla = allocate(memory, size, false);
	set_type(bla, type);
	return bla;
}

Variable * add_variable(Environment * environment, char * name, Element value)
{
	Variable * variable = new(Variable, VTYPE_VARIABLE);
	variable->name = name;
	variable->value = value;
	variable->next = NULL;

	if(environment->variables != NULL)
	{
		Variable * prev = environment->variables;
		while (prev->next != NULL) prev = prev->next;
		prev->next = variable;
	}
	else
		environment->variables = variable;
	return variable;
}

Variable * find_variable(Environment * environment, char * name, bool recurse)
{
	Variable * var = environment->variables;
	while (var != NULL && strcmp(var->name, name) != 0) {
		var = var->next;
	}

	if (var != NULL) {
		return var;
	} else if (recurse && environment->parent != NULL) {
		return find_variable(environment->parent, name, recurse);
	}
	// else
	return NULL;
}

Variable * set_variable(Environment * environment, char * name, Element value, bool recurse)
{
	Variable * var = find_variable(environment, name, recurse);
	if (var == NULL) return NULL;
	var->value = value;
	return var;
}
