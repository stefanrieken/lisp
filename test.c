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

#define DATA(val) (((intptr_t) val) & ~0b11)
#define BTYPE(val) (((intptr_t) val) & 0b11)

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
	if(!strcmp(what, who) == 0) {
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
	assert("label:", VTYPE_ID, get_type(parsed->value));
	expect("if", parsed->value);
		add_indent();
		Node * cond = parsed->next;
		assert("(", VTYPE_LIST, get_type(cond->value));
		Node * eqv = ((Node *) cond->value);
		assert("label:", VTYPE_ID, get_type(eqv->value));
		expect("eq", eqv->value);
		Node * int1 = eqv->next;
		assert(" int:", VTYPE_INT, get_type(int1->value));
		assert("2", 2, *((intptr_t*) int1->value));
		Node * int2 = int1->next;
		assert(" int:", VTYPE_INT, get_type(int2->value));
		assert("2", 2, *((intptr_t*) int2->value));
		assert(")", 0, (intptr_t) int2->next);
		Node * thenn = cond->next;
		assert(" (", VTYPE_LIST, get_type(thenn->value));
		Node * thenbl = thenn->value;
		Node * lambdalabel = thenbl->value;
		assert("label:", VTYPE_ID, get_type(lambdalabel->value));
		expect("lambda", lambdalabel->value);
			add_indent();
			Node * args = ((Node *) lambdalabel->next)->value;
			assert("(", VTYPE_LIST, get_type(args));
			assert("label:", VTYPE_ID, get_type(args->value));
			expect("x", args->value);
			assert(") ", 0, (intptr_t) args->next);
			Node * body = ((Node *) ((Node *) lambdalabel->next)->next)->value;
			assert("(", VTYPE_LIST, get_type(body));
			assert("label:", VTYPE_ID, get_type(body->value));
			expect("list", body->value);
			Node * str = body->next;
			assert(" string:", VTYPE_STRING, get_type(str->value));
			expect("This test is", str->value);
			Node * var = str->next;
			assert(" label:", VTYPE_ID, get_type(var->value));
			expect("x", var->value);
			assert(")", 0, (intptr_t) var->next);
			remove_indent();
		assert(")", 0, (intptr_t) thenn->next);
		remove_indent();
	Node * successtr = thenbl->next;
	assert("string:", VTYPE_STRING, get_type(successtr->value));
	expect("successful!", successtr->value);
	assert(")", 0, (intptr_t) successtr->next);
	remove_indent();
	
	printf("\n\nCalling 'transform' (work in progress)\n\n");
	//Environment * transform_env = (Environment *) allocate(memory, sizeof(Environment), false);

	indent = 0;

	add_indent();
	Node * post = transform(parsed, root_env, root_env);
	assert("root:(", VTYPE_LIST, get_type((void *) mask(post, ~0b11)));
		add_indent();
		assert("native:", NATIVE, (intptr_t) mask(post->value, 0b11));
		assert("(", VTYPE_LIST, get_type((void *) mask(post->value, ~0b11)));
		Node * thenblock = ((Node*)mask(post->value, ~0b11));
		assert("native:", NATIVE, mask(thenblock->value, 0b11));
		assert("string:", VTYPE_STRING, get_type((void*) mask(thenblock->value, ~0b11)));
		expect("successful!", (char *) mask(thenblock->value, ~0b11));
			add_indent();
			Node * lambda2 = thenblock->next;
			assert("native:", NATIVE, BTYPE(lambda2->value));
			assert("(", VTYPE_LIST, get_type((void*) DATA(lambda2->value)));
			Node * closure = (Node*) DATA(lambda2->value);
			assert("native:", NATIVE, BTYPE(closure->value));
			assert("environment", VTYPE_ENVIRONMENT, get_type((void*) DATA(closure->value)));
			Node * x = closure->next;
			assert(" label:", LABEL, BTYPE(x->value));
			expect("x", (char*) DATA(x->value));
			Node * string = x->next;
			assert(" native:", NATIVE, BTYPE(string->value));
			assert("string:", VTYPE_STRING, get_type((void*) DATA(string->value)));
			expect("This test is", (char*) DATA(string->value));
			Node * liszt = string->next;
			assert(" primitive:list", PRIMITIVE, BTYPE(liszt->value));
			Node * apply = liszt->next;
			assert(" <apply>", 0, (intptr_t) apply->value);
			Node * next = apply->next; // TODO this second apply seems excessive?
			assert(" <apply>", 0, (intptr_t) next->value);
			assert(")", 0, (intptr_t) next->next);
			remove_indent();
		Node * apply1 = lambda2->next;
		assert(" <apply>", 0, (intptr_t) apply1->value);
		assert(")", 0, (intptr_t) apply1->next);
		remove_indent();
	Node * val1 = post->next;
	assert("int:", INTEGER, mask(val1->value, 0b11));
	assert("2", 2, ((intptr_t) val1->value) >> 2);
	Node * val2 = val1->next;
	assert(" int:", 0b01, mask(val2->value, 0b11));
	assert("2", 2, ((intptr_t) val2->value) >> 2);
	Node * eq = val2->next;
	assert(" primitive:", PRIMITIVE, mask(eq->value, 0b11));
	// TODO this is interesting: apparently we do wrap the callbacks inside a tmmh pointer
	// This extra indirection may not be required after 'transform' has egalized the difference
	// between special and primitive functions (by (not) transforming their arguments)
	assert("special:eq", VTYPE_SPECIAL, get_type((void*)mask(eq->value, ~0b11)));
	Node * apply2 = eq->next;
	assert(" <apply>", 0, (intptr_t) apply2->value);
	Node * if2 = apply2->next;
	assert(" primitive:", PRIMITIVE, mask(if2->value, 0b11));
	assert("special:if", VTYPE_SPECIAL, get_type((void*) mask(if2->value, ~0b11))); // TODO same comment as before
	Node * apply3 = if2->next;
	assert(" <apply>", 0, (intptr_t) apply3->value);
	assert(")", 0, (intptr_t) (((Node*)apply3)->next)); // TODO should be apply3->next
	remove_indent();

	printf("\n\nRunning 'eval':\n\n");
	println_value(eval(parsed, root_env));
	
	fclose(file);
}

