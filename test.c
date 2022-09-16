#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "structs.h"
#include "parse.h"
#include "print.h"
#include "lisp_primitives.h"
#include "special.h"
#include "transform.h"
#include "../tmmh/tmmh.h"
#include "eval.h"

#define mask(val, m) (((intptr_t) val) & m)

#define DATA(val) (((intptr_t) val.as_int) & ~0b11)
#define BTYPE(val) (((intptr_t) val.as_int) & 0b11)

// Copied over from wonky.h
typedef enum BasicType {
  LABEL,
  INTEGER,
  NATIVE,
  PRIMITIVE
} BasicType;

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

	printf("\n\nRunning (plain) 'eval':\n\n");
	println_value(eval((Element) parsed, root_env));

	fclose(file);
}
