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
	assert("Node", LIST, get_type(parsed));
	assert("label", ID, get_type(parsed->value));
	expect("if", parsed->value);
	Node * cond = ((Node *) parsed->next)->value;
	assert("Node", LIST, get_type(cond));
	assert("label", ID, get_type(cond->value));
	expect("eq", cond->value);
	assert("int", INT, get_type(((Node *) cond->next)->value));
	Node * lambda = ((Node *) ((Node *) ((Node *) parsed->next)->next)->value)->value;
	assert("Node", LIST, get_type(lambda));
	assert("label", ID, get_type(lambda->value));
	expect("lambda", lambda->value);
	Node * args = ((Node *) lambda->next)->value;
	assert("Node", LIST, get_type(args));
	assert("label", ID, get_type(args->value));
	expect("x", args->value);
	Node * body = ((Node *) ((Node *) lambda->next)->next)->value;
	assert("Node", LIST, get_type(body));
	assert("label", ID, get_type(body->value));
	expect("list", body->value);
	Node * str = body->next;
	assert("string", STRING, get_type(str->value));
	expect("This test is", str->value);
	Node * var = str->next;
	assert("label", ID, get_type(var->value));
	expect("x", var->value);
	
	printf("Calling 'transform' (work in progress)\n");
	//Environment * transform_env = (Environment *) allocate(memory, sizeof(Environment), false);

	Node * post = transform(parsed, root_env, root_env);
	assert("native pointer", 0b10, (intptr_t) mask(post->value, 0b11));
	assert("then block", LIST, get_type((void *) mask(post->value, ~0b11)));
	// Right now we don't pre-translate the 'then' block, so it is little use to analyse it
/*
		Node * thenblock = ((Node*)mask(post->value, ~0b11))->value;
        	expect("successful!", (char*) mask(thenblock->value, ~0b11));
		Node * lambda2 = thenblock->next;
		// TODO: fix+test lambda transformation
		Node * apply1 = lambda2->next;
		assert("apply", apply1->value == 0);
		assert("block done", apply1->next == NULL);
		*/
	// Uhh... the 'eval' block is also not pre-translated, because nothing for 'special' is...
	Node * val1 = post->next;
	assert("int", 0b01, mask(val1->value, 0b11));
	assert("22", 2, ((intptr_t) val1->value) >> 2);
	Node * val2 = val1->next;
	assert("int", 0b01, mask(val2->value, 0b11));
	assert("2", 2, ((intptr_t) val2->value) >> 2);
	Node * eq = val2->next;
	assert("primitive", 0b11, mask(eq->value, 0b11));
	// TODO this is interesting: apparently we do wrap the callbacks inside a tmmh pointer
	// This extra indirection may not be required after 'transform' has egalized the difference
	// between special and primitive functions (by (not) transforming their arguments)
	assert("special", SPECIAL, get_type((void*)mask(eq->value, ~0b11)));
	Node * apply2 = eq->next;
	assert("apply", 0, (intptr_t) apply2->value);
	Node * if2 = apply2->next;
	assert("primitivez", 0b11, mask(if2->value, 0b11));
	assert("special", SPECIAL, get_type((void*) mask(if2->value, ~0b11))); // TODO same comment as before
	Node * apply3 = if2->next;
	assert("apply", 0, (intptr_t) apply3->value);
	assert("done", 0, (intptr_t) (((Node*)apply3)->next)); // TODO should be apply3->next
	

	printf("Running 'eval'\n");
	println_value(eval(parsed, root_env));
	
	fclose(file);
}

