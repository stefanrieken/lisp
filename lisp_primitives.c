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

// Simple math functions. These are really not 'special'
// but they are 'primitive'; and this is a first implementation.
// 'intptr_t' is the 'int' that is of pointer width
// (so on 64-bit machines it is 64-bit instead of 32)
static inline intptr_t extract_op(Node * arg, Environment * env)
{
	return * (intptr_t *) eval(arg->value, env);
}

static inline intptr_t * envelop(intptr_t val)
{
	intptr_t * pointer = (intptr_t *) allocate(memory, 4, false);
	set_type(pointer, INT);
	(* pointer) = val;
	return pointer;
}

static void * add (Node * args, Environment * env)
{
	if (args == NULL || args->next == NULL) return NULL;
	return envelop(extract_op(args, env) + extract_op(args->next, env));
}

static void * subtract (Node * args, Environment * env)
{
	if (args == NULL || args->next == NULL) return NULL;
	return envelop(extract_op(args, env) - extract_op(args->next, env));
}

static void * multiply (Node * args, Environment * env)
{
	if (args == NULL || args->next == NULL) return NULL;
	return envelop(extract_op(args, env) * extract_op(args->next, env));
}

static void * divide (Node * args, Environment * env)
{
	if (args == NULL || args->next == NULL) return NULL;
	return envelop(extract_op(args, env) / extract_op(args->next, env));
}

static void * modulo (Node * args, Environment * env)
{
	if (args == NULL || args->next == NULL) return NULL;
	return envelop(extract_op(args, env) % extract_op(args->next, env));
}
// bit-meddling things
static void * bitset (Node * args, Environment * env)
{
	if (args == NULL || args->next == NULL) return NULL;
	return envelop(extract_op(args, env) | (1 << extract_op(args->next, env)));
}

static void * bitclear (Node * args, Environment * env)
{
	if (args == NULL || args->next == NULL) return NULL;
	return envelop(extract_op(args, env) & ~(1 << extract_op(args->next, env)));
}

static void * address (Node * args, Environment * env)
{
	intptr_t * address = allocate(memory, sizeof (void *), false);
	set_type(address, INT);
	void * evalresult = eval(args->value, env);
	(* address) = (intptr_t) (evalresult);
	return address;
}

static void * value_at (Node * args, Environment * env)
{
	if (args == NULL) return NULL;

	intptr_t * address = (intptr_t *) eval(args->value, env);
	return (void *) * address;
}

static void * gc_collect(Node * args, Environment * env) {
  char * buffer = malloc((tmmh_memsize(memory) + 1) / 4);

  tmmh_visualize(memory, buffer);
  printf("%s\n", buffer);

  tmmh_gc(memory, (void *){env}, 1);

  tmmh_visualize(memory, buffer);
  printf("%s\n", buffer);

  return NULL;
}

static inline void * typify(special_form form)
{
	special_form * type = new (special_form, PRIMITIVE);
	(* type) = form;
	return type;
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
