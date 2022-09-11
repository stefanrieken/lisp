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
#include "../tmmh/tmmh.h"

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
	Environment * transform_env = (Environment *) allocate(memory, sizeof(Environment), false);
	transform(parsed, transform_env, transform_env);

	printf("Running 'eval'\n");
	println_value(eval(parsed, root_env));
	
	fclose(file);
}

