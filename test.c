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

void assert(char * what, bool whatwhat) {
	if(!whatwhat) {
		printf("Expected %s\n", what);
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
	assert("Node", get_type(parsed) == LIST);
	assert("label", get_type(parsed->value) == ID);
	expect("if", parsed->value);
	Node * cond = ((Node *) parsed->next)->value;
	assert("Node", get_type(cond) == LIST);
	assert("label", get_type(cond->value) == ID);
	expect("eq", cond->value);
	assert("int", get_type(((Node *) cond->next)->value) == INT);
	Node * lambda = ((Node *) ((Node *) ((Node *) parsed->next)->next)->value)->value;
	assert("Node", get_type(lambda) == LIST);
	assert("label", get_type(lambda->value) == ID);
	expect("lambda", lambda->value);
	Node * args = ((Node *) lambda->next)->value;
	assert("Node", get_type(args) == LIST);
	assert("label", get_type(args->value) == ID);
	expect("x", args->value);
	Node * body = ((Node *) ((Node *) lambda->next)->next)->value;
	assert("Node", get_type(body) == LIST);
	assert("label", get_type(body->value) == ID);
	expect("list", body->value);
	Node * str = body->next;
	assert("string", get_type(str->value) == STRING);
	expect("This test is", str->value);
	Node * var = str->next;
	assert("label", get_type(var->value) == ID);
	expect("x", var->value);
	
	printf("Calling 'transform' (work in progress)\n");
	//Environment * transform_env = (Environment *) allocate(memory, sizeof(Environment), false);

	Node * post = transform(parsed, root_env, root_env);
	assert("native pointer", (intptr_t) mask(post->value, 0b11) == 0b10);
	assert("string", get_type((void *) mask(post->value, ~0b11)));
	expect("successful!", (char*) mask(post->value, ~0b11));
	Node * lambda2 = post->next;
	// TODO: fix+test lambda transformation
	//Node * apply1 = post->next;
	// TODO: fix+test insertion of 'apply' (or combination of call+apply)
	Node * val1 = lambda2->next; // TODO should by apply1->next
	assert("int", mask(val1->value, 0b11) == 0b01);
	assert("2", ((intptr_t) val1->value) >> 2);
	Node * val2 = val1->next;
	assert("int", mask(val2->value, 0b11) == 0b01);
	assert("2", ((intptr_t) val2->value) >> 2);
	Node * eq = val2->next;
	assert("primitive", mask(eq->value, 0b11) == 0b11);
	// TODO this is interesting: apparently we do wrap the callbacks inside a tmmh pointer
	// This extra indirection may not be required after 'transform' has egalized the difference
	// between special and primitive functions (by (not) transforming their arguments)
	assert("special", get_type((void*)mask(eq->value, ~0b11)) == SPECIAL);
	//Node * apply2 = eq->next;
	// TODO: fix+test insertion of 'apply' (or combination of call+apply)
	Node * if2 = eq->next; // TODO should be apply2->next
	assert("primitive", mask(if2->value, 0b11) == 0b11);
	assert("special", get_type((void*) mask(if2->value, ~0b11)) == SPECIAL); // TODO same comment as before
	//Node * apply3 = eq->next;
	// TODO: fix+test insertion of 'apply' (or combination of call+apply)
	assert("done", if2->next == NULL); // TODO should be apply3->next
	

	printf("Running 'eval'\n");
	println_value(eval(parsed, root_env));
	
	fclose(file);
}

