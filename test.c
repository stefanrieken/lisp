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
  _LABEL,
  INTEGER,
  NATIVE,
  PRIMITIVE
} BasicType;

#define mask(val, m) (((intptr_t) val) & m)
void * memory;

void assert(char * in_words, intptr_t what, intptr_t who) {
	if(what != who) {
		printf("Expected %ld for %s got %ld\n", what, in_words, who);
		exit(-1);
	}
}

void expect(char * what, char * who) {
	if(!strcmp(what, who) == 0) {
		printf("Expected %s got %s\n", what, who);
		exit(-1);
	}
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

	printf("Parsing test file\n");

	FILE * file = fopen("testdata.scm", "r");
	read_from(file);
	
	Node * parsed = (Node *) parse_value(file);
	// peel apart entirely!
	assert("Node", VTYPE_LIST, get_type(parsed));
	assert("label", VTYPE_ID, get_type(parsed->value));
	expect("if", parsed->value);
	Node * cond = ((Node *) parsed->next)->value;
	assert("Node", VTYPE_LIST, get_type(cond));
	assert("label", VTYPE_ID, get_type(cond->value));
	expect("eq", cond->value);
	assert("int", VTYPE_INT, get_type(((Node *) cond->next)->value));
	Node * lambda1 = ((Node *) ((Node *) ((Node *) parsed->next)->next)->value)->value;
	assert("Node", VTYPE_LIST, get_type(lambda1));
	assert("label", VTYPE_ID, get_type(lambda1->value));
	expect("lambda", lambda1->value);
	Node * args = ((Node *) lambda1->next)->value;
	assert("Node", VTYPE_LIST, get_type(args));
	assert("label", VTYPE_ID, get_type(args->value));
	expect("x", args->value);
	Node * body = ((Node *) ((Node *) lambda1->next)->next)->value;
	assert("Node", VTYPE_LIST, get_type(body));
	assert("label", VTYPE_ID, get_type(body->value));
	expect("list", body->value);
	Node * str = body->next;
	assert("string", VTYPE_STRING, get_type(str->value));
	expect("This test is", str->value);
	Node * var = str->next;
	assert("label", VTYPE_ID, get_type(var->value));
	expect("x", var->value);
	
	printf("Calling 'transform' (work in progress)\n");
	//Environment * transform_env = (Environment *) allocate(memory, sizeof(Environment), false);

	Node * post = transform(parsed, root_env, root_env);
	assert("native pointer", NATIVE, (intptr_t) mask(post->value, 0b11));
	assert("then block", VTYPE_LIST, get_type((void *) mask(post->value, ~0b11)));
	// Right now we don't pre-translate the 'then' block, so it is little use to analyse it
		Node * thenblock = ((Node*)mask(post->value, ~0b11));
		assert("native pointer", NATIVE, mask(thenblock->value, 0b11));
		assert("string", VTYPE_STRING, get_type((void*) mask(thenblock->value, ~0b11)));
		expect("successful!", (char *) mask(thenblock->value, ~0b11));
		Node * lambda2 = thenblock->next;
		// TODO: (refine and) test the closure sub-expression
		Node * apply1 = lambda2->next;
		assert("apply", 0, (intptr_t) apply1->value);
		assert("block done", 0, (intptr_t) apply1->next);
	Node * val1 = post->next;
	assert("int", INTEGER, mask(val1->value, 0b11));
	assert("22", 2, ((intptr_t) val1->value) >> 2);
	Node * val2 = val1->next;
	assert("int", 0b01, mask(val2->value, 0b11));
	assert("2", 2, ((intptr_t) val2->value) >> 2);
	Node * eq = val2->next;
	assert("primitive", PRIMITIVE, mask(eq->value, 0b11));
	// TODO this is interesting: apparently we do wrap the callbacks inside a tmmh pointer
	// This extra indirection may not be required after 'transform' has egalized the difference
	// between special and primitive functions (by (not) transforming their arguments)
	assert("special", VTYPE_SPECIAL, get_type((void*)mask(eq->value, ~0b11)));
	Node * apply2 = eq->next;
	assert("apply", 0, (intptr_t) apply2->value);
	Node * if2 = apply2->next;
	assert("primitive", PRIMITIVE, mask(if2->value, 0b11));
	assert("special", VTYPE_SPECIAL, get_type((void*) mask(if2->value, ~0b11))); // TODO same comment as before
	Node * apply3 = if2->next;
	assert("apply", 0, (intptr_t) apply3->value);
	assert("done", 0, (intptr_t) (((Node*)apply3)->next)); // TODO should be apply3->next
	

	printf("Running 'eval'\n");
	println_value(eval(parsed, root_env));
	
	fclose(file);
}

