#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "structs.h"
#include "parse.h"
#include "print.h"
#include "prims.h"
#include "../wonky/wonky.h"
#include "transform.h"
#include "../tmmh/tmmh.h"


#define mask(val, m) (((intptr_t) val) & m)

#define DATA(val) (((intptr_t) val.as_int) & ~0b11)
#define BTYPE(val) (((intptr_t) val.as_int) & 0b11)

int count(Node * nodes)
{
  if(nodes->next.ptr == NULL) return 1;
  else return(count(nodes->next.ptr)) +1;
}

void fill(Element elements[], Node * list)
{
  if(list != NULL)
  {
    elements[0] = list->value;
    fill(elements+1, list->next.node);
  }
}

State * to_state(Node * nodes, Environment * env)
{
  int size = count(nodes);

  Element * code = malloc(sizeof(Element) * size); // TODO use tmmh
  fill(code, nodes);

  State * state = malloc(sizeof(State)); // TODO use tmmh
  state->at = 0;
  state->code = code;
  state->env = env;
  state->stack = new_stack();
  state->code_size = size;
  return state;
}

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


//extern bool eq(State * state);
//extern bool iff(State * state);

int main()
{
	pif pifs[] = {pif_none, pif_none, pif_none, pif_none, pif_none, pif_none, pif_none, pif_none};

	memory = tmmh_init(5000, pifs);

	printf("\n\nTesting BasicData alignment:\n\n");
	Element element;
	assert("struct size=4", 4, sizeof(BasicData)); // N.b. is smaller than Element on 64-bit systems!
	element.basic.label.index=9213;
	element.basic.label.args=42;
	element.basic.label.type=0b11;
	assert("9213", element.basic.label.index, element.as_int >> 8);
	assert("42", element.basic.label.args, (element.as_int >> 2) & 0b111111);
	assert("0b11", element.basic.label.type, element.as_int & 0b11);
	//
	element.basic.value.as_int = 4767;
	assert("4767", element.basic.value.as_int, element.as_int >> 2);

	printf("\n\nParsing test file:\n\n");

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
  Environment * root_env = (Environment *) allocate(memory, sizeof(Environment), false);

	root_env->parent = NULL;
	root_env->variables = NULL;

	register_prims(root_env);

	indent = 0;

	add_indent();
	Node * post = transform(parsed, root_env);
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
	Node * equ = val2->next.node;
	assert(" primitive:", PRIMITIVE, BTYPE(equ->value));
//  printf("eq: %p %p %p\n", equ->value.ptr, (void*) DATA(equ->value), eq);
//	assert("eq", (intptr_t) eq & ~0b11, DATA(equ->value)); // TODO why are prims pointers unaligned?
	Node * apply2 = equ->next.node;
	assert(" <apply>", 0, apply2->value.as_int);
	Node * if2 = apply2->next.node;
	assert(" primitive:", PRIMITIVE, BTYPE(if2->value));
//	assert("if", (intptr_t) iff & ~0b11, DATA(if2->value)); // TODO why are prims pointers unaligned?
	Node * apply3 = if2->next.node;
	assert(" <apply>", 0, apply3->value.as_int);
	assert(")", 0, apply3->next.as_int);
	remove_indent();

	printf("\n\nConverting to State\n\n");
	State * state = to_state(post, root_env);

	printf("\n\nRunning (wonky) 'eval':\n\n");
	// TODO reasons why this presently still crashes:
	// 1) The 'wonky' eval() assumes a PrimitiveCallback where we provide (PrimitiveCallback *) at best, BUT:
	// 2) The primitives we provide ('eq' to begin with) are NOT expecting a State parameter, or Basic Type'd values
	//    (In other words, there's still lots of stuff to be done here. We could postpone providing number of args
	//     if we temporarily hack 'list' to fit the needs of our test, but there's that as well.)
	// 3) Lambda's, lets, etc. are not presently compiled to States, only transformed.
	//    So there's lots of garbage going on at this point as well.
	eval(state);

	fclose(file);
}
