#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "structs.h"
#include "parse.h"
#include "print.h"
#include "lisp_primitives.h"
#include "special.h"
#include "eval.h"
#include "transform.h"
#include "../tmmh/tmmh.h"

// copied from wonky.h
typedef enum BasicType {
  LABEL,
  INTEGER,
  NATIVE,
  PRIMITIVE
} BasicType;

#define mask(val, m) (((intptr_t) val) & m)

#define DATA(val) (((intptr_t) val.as_int) & ~0b11)
#define BTYPE(val) (((intptr_t) val.as_int) & 0b11)

void * memory;

int indent = 0;

void assert(char * in_words, intptr_t what, intptr_t who) {
	if(what != who) {
		printf("Expected %ld for %s got %ld\n", what, in_words, who);
		exit(-1);
	}
	else
	{
		printf("%s", in_words);
	}
}

void expect(char * what, char * who) {
	if(strcmp(what, who) != 0) {
		printf("Expected %s got %s\n", what, who);
		exit(-1);
	}
	else
	{
		printf("%s", what);
	}
}

void add_indent()
{
	printf("\n");
	indent++;
	for (int i=0;i<indent;i++)printf("\t");
}
void remove_indent()
{
	printf("\n");
	indent--;
	for (int i=0;i<indent;i++)printf("\t");
}


extern void * list(Node * arg, Environment * env);
int main()
{
	pif pifs[] = {pif_none, pif_none, pif_none, pif_none, pif_none, pif_none, pif_none, pif_none};

	memory = tmmh_init(5000, pifs);
	Environment * root_env = (Environment *) allocate(memory, sizeof(Environment), false);

	root_env->parent = NULL;
	root_env->variables = NULL;

	register_specials(root_env);
	register_primitives(root_env);

	printf("Parsing test file:\n\n");

	FILE * file = fopen("testdata.scm", "r");
	read_from(file);

	indent = 0;

	add_indent();
	Node * parsed = (Node *) parse_value(file);
	// peel apart entirely!
	assert("(", VTYPE_LIST, get_type(parsed));
	assert("label:", VTYPE_ID, get_type(parsed->value.ptr));
	expect("if", parsed->value.str);
		add_indent();
		Node * cond = parsed->next.node;
		assert("(", VTYPE_LIST, get_type(cond->value.ptr));
		Node * eqv = cond->value.node;
		assert("label:", VTYPE_ID, get_type(eqv->value.ptr));
		expect("eq", eqv->value.str);
		Node * int1 = eqv->next.node;
		assert(" int:", VTYPE_INT, get_type(int1->value.ptr));
		assert("2", 2, *(int1->value.intptr));
		Node * int2 = int1->next.node;
		assert(" int:", VTYPE_INT, get_type(int2->value.ptr));
		assert("2", 2, *(int2->value.intptr));
		assert(")", 0, int2->next.as_int);
		Node * thenn = cond->next.node;
		assert(" (", VTYPE_LIST, get_type(thenn->value.ptr));
		Node * thenbl = thenn->value.node;
		Node * lambdalabel = thenbl->value.node;
		assert("label:", VTYPE_ID, get_type(lambdalabel->value.ptr));
		expect("lambda", lambdalabel->value.str);
			add_indent();
			Node * args = lambdalabel->next.node->value.node;
			assert("(", VTYPE_LIST, get_type(args));
			assert("label:", VTYPE_ID, get_type(args->value.ptr));
			expect("x", args->value.str);
			assert(") ", 0, (intptr_t) args->next.as_int);
			Node * body = lambdalabel->next.node->next.node->value.node;
			assert("(", VTYPE_LIST, get_type(body));
			assert("label:", VTYPE_ID, get_type(body->value.ptr));
			expect("list", body->value.str);
			Node * str = body->next.node;
			assert(" string:", VTYPE_STRING, get_type(str->value.ptr));
			expect("This test is", str->value.str);
			Node * var = str->next.node;
			assert(" label:", VTYPE_ID, get_type(var->value.ptr));
			expect("x", var->value.str);
			assert(")", 0, (intptr_t) var->next.as_int);
			remove_indent();
		assert(")", 0, (intptr_t) thenn->next.as_int);
		remove_indent();
	Node * successtr = thenbl->next.node;
	assert("string:", VTYPE_STRING, get_type(successtr->value.ptr));
	expect("successful!", successtr->value.str);
	assert(")", 0, (intptr_t) successtr->next.as_int);
	remove_indent();

	printf("\n\nCalling 'transform' (work in progress)\n\n");
	//Environment * transform_env = (Environment *) allocate(memory, sizeof(Environment), false);

	indent = 0;

	add_indent();
	Node * post = transform(parsed, root_env, root_env);
	assert("root:(", VTYPE_LIST, get_type((void *) mask(post, ~0b11)));
		add_indent();
		assert("native:", NATIVE, (intptr_t) mask(post->value.as_int, 0b11));
		assert("(", VTYPE_LIST, get_type((void *) mask(post->value.as_int, ~0b11)));
		Node * thenblock = ((Node*)mask(post->value.as_int, ~0b11));
		assert("native:", NATIVE, mask(thenblock->value.as_int, 0b11));
		assert("string:", VTYPE_STRING, get_type((void*) mask(thenblock->value.as_int, ~0b11)));
		expect("successful!", (char *) mask(thenblock->value.as_int, ~0b11));
			add_indent();
			Node * lambda2 = thenblock->next.node;
			assert("native:", NATIVE, BTYPE(lambda2->value));
			assert("(", VTYPE_LIST, get_type((void*) DATA(lambda2->value)));
			Node * closure = (Node*) DATA(lambda2->value);
			assert("native:", NATIVE, BTYPE(closure->value));
			assert("environment", VTYPE_ENVIRONMENT, get_type((void*) DATA(closure->value)));
			Node * x = closure->next.node;
			assert(" label:", LABEL, BTYPE(x->value));
			expect("x", (char*) DATA(x->value));
			Node * string = x->next.node;
			assert(" native:", NATIVE, BTYPE(string->value));
			assert("string:", VTYPE_STRING, get_type((void*) DATA(string->value)));
			expect("This test is", (char*) DATA(string->value));
			Node * liszt = string->next.node;
			assert(" primitive:list", PRIMITIVE, BTYPE(liszt->value));
			Node * apply = liszt->next.node;
			assert(" <apply>", 0, apply->value.as_int);
			assert(")", 0, apply->next.as_int);
			remove_indent();
		Node * apply1 = lambda2->next.node;
		assert(" <apply>", 0, apply1->value.as_int);
		assert(")", 0, apply1->next.as_int);
		remove_indent();
	Node * val1 = post->next.node;
	assert("int:", INTEGER, mask(val1->value.as_int, 0b11));
	assert("2", 2, (val1->value.as_int >> 2));
	Node * val2 = val1->next.node;
	assert(" int:", 0b01, mask(val2->value.as_int, 0b11));
	assert("2", 2, (val2->value.as_int >> 2));
	Node * eq = val2->next.node;
	assert(" primitive:", PRIMITIVE, mask(eq->value.as_int, 0b11));
	// TODO this is interesting: apparently we do wrap the callbacks inside a tmmh pointer
	// This extra indirection may not be required after 'transform' has egalized the difference
	// between special and primitive functions (by (not) transforming their arguments)
	assert("special:eq", VTYPE_SPECIAL, get_type((void*)mask(eq->value.as_int, ~0b11)));
	Node * apply2 = eq->next.node;
	assert(" <apply>", 0, apply2->value.as_int);
	Node * if2 = apply2->next.node;
	assert(" primitive:", PRIMITIVE, mask(if2->value.as_int, 0b11));
	assert("special:if", VTYPE_SPECIAL, get_type((void*) mask(if2->value.as_int, ~0b11))); // TODO same comment as before
	Node * apply3 = if2->next.node;
	assert(" <apply>", 0, apply3->value.as_int);
	assert(")", 0, apply3->next.as_int);
	remove_indent();

	printf("\n\nRunning 'eval':\n\n");
	println_value(eval((Element) parsed, root_env));

	fclose(file);
}
