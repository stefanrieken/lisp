#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
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

// Simple math functions.

// 'intptr_t' is the 'int' that is of pointer width
// (so on 64-bit machines it is 64-bit instead of 32)
#define AS_INT(x) (*(x->value.intptr))

static inline Element envelop(intptr_t val)
{
	intptr_t * pointer = (intptr_t *) allocate(memory, 4, false);
	set_type(pointer, VTYPE_INT);
	(* pointer) = val;
	return (Element) pointer;
}

static Element add (Node * args, Environment * env)
{
	if (args == NULL || args->next.ptr == NULL) return (Element) NULL;
	return envelop(AS_INT(args) + AS_INT(args->next.node));
}

static Element subtract (Node * args, Environment * env)
{
	if (args == NULL || args->next.ptr == NULL) return (Element) NULL;
	return envelop(AS_INT(args) - AS_INT(args->next.node));
}

static Element multiply (Node * args, Environment * env)
{
	if (args == NULL || args->next.ptr == NULL) return (Element) NULL;
	return envelop(AS_INT(args) * AS_INT(args->next.node));
}

static Element divide (Node * args, Environment * env)
{
	if (args == NULL || args->next.ptr == NULL) return (Element) NULL;
	return envelop(AS_INT(args) / AS_INT(args->next.node));
}

static Element modulo (Node * args, Environment * env)
{
	if (args == NULL || args->next.ptr == NULL) return (Element) NULL;
	return envelop(AS_INT(args) % AS_INT(args->next.node));
}
// bit-meddling things
static Element bitset (Node * args, Environment * env)
{
	if (args == NULL || args->next.ptr == NULL) return (Element) NULL;
	return envelop(AS_INT(args) | (1 << AS_INT(args->next.node)));
}

static Element bitclear (Node * args, Environment * env)
{
	if (args == NULL || args->next.ptr == NULL) return (Element) NULL;
	return envelop(AS_INT(args) & ~(1 << AS_INT(args->next.node)));
}

static Element address (Node * args, Environment * env)
{
	intptr_t * address = allocate(memory, sizeof (void *), false);
	set_type(address, VTYPE_INT);
	(* address) = (intptr_t) (args->value.ptr);
	return (Element) address;
}

static Element value_at (Node * args, Environment * env)
{
	if (args == NULL) return (Element) NULL;
	return (Element) AS_INT(args);
}

static Element gc_collect(Node * args, Environment * env) {
	char * buffer = malloc((tmmh_memsize(memory) + 1) / 4);

	tmmh_visualize(memory, buffer);
	printf("%s\n", buffer);

	tmmh_gc(memory, (void *){env}, 1);

	tmmh_visualize(memory, buffer);
	printf("%s\n", buffer);

	return (Element) NULL;
}

Element typify(special_form form)
{
	special_form * type = new (special_form, VTYPE_PRIMITIVE);
	(* type) = form;
	return (Element) type;
}

void register_primitives(Environment * env)
{
	// Integer functions
	add_variable(env, "+", typify(add));
	add_variable(env, "-", typify(subtract));
	add_variable(env, "*", typify(multiply));
	add_variable(env, "/", typify(divide));
	add_variable(env, "%", typify(modulo));
	add_variable(env, "bitset", typify(bitset));
	add_variable(env, "bitclear", typify(bitclear));
	add_variable(env, "address", typify(address));
	add_variable(env, "value-at", typify(value_at));
	add_variable(env, "gc-collect", typify(gc_collect));
}
