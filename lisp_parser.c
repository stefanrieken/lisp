// This is an implementation of 'cons cells'.
// Interesting fact: the original IBM 704 had 36-bit words,
// divided into a 15-bit 'address register', another 15-bit 'decrement register'
// and a 6-bit 'tag'. This just happened to be the ideal storage for a 'cons cell'.
// On modern computers, the struct is awkwardly mis-aligned.
// There are different solutions for it:
// - storing the type with the value, diminishing pointer / integer expressiveness
// - dropping the 'cons cells' structure altogether
//   (see e.g. https://clojure.org/reference/sequences)
// This implementation explores the traditional approach, ignoring optimization.

#include "lisp_parser.h"

#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "lisp_lexer.h"
#include "../tmmh/tmmh.h"

#define new(Type, TYPE) (Type *) allocate_type (sizeof(Type), TYPE)

static inline void * allocate_type(int size, int type)
{
	void * bla = allocate(memory, size, false);
	set_type(bla, type);
	return bla;
}

void * parse_value()
{
	int ch = get_non_whitespace_char();
	if (ch == ';') {
		skip_line();
		ch = get_non_whitespace_char();
	}

	if (ch == '"')
		return parse_string();
	else if (ch == '\'')
		return parse_quote();
	else if (ch == '(')
		return parse_list();
	else if (ch != -1) {
		return parse_label_or_number(ch, 10);
	}
	return NULL;
}

Node * parse_quote()
{
	Node * node = new (Node, LIST);
	void * quote = allocate_type(6, ID);
	strcpy (quote, "quote");
	node->value = quote;

	Node * next = new (Node, LIST);
	next->next = NULL;
	next->value = parse_value();
	node->next = next;

	return node;
}

Node * parse_list()
{
	// '(' is already read here
	int ch = get_non_whitespace_char();
	if (ch == ')') return NULL; // empty list
	buffer_return(ch);

	Node * pair = new (Node, LIST);

	pair->value = parse_value();
	ch = get_non_whitespace_char();
	if (ch == -1) return NULL;

	if (ch == '.') {
		pair->next = parse_value();
		int ch = get_non_whitespace_char();
		if (ch != ')') return NULL; // TODO say parse error
		return pair;
	} else if (ch == ')') {
		pair->next = NULL;
		return pair;
	} else {
		buffer_return(ch);
		pair->next = parse_list();
		return pair;
	}
}

void print_list_body(Node * list)
{
	print_value(list->value);
	if (list->next != NULL) {
		if (get_type(list->next) == LIST) {
			printf(" ");
			print_list_body((Node *) list->next);
		} else {
			printf(" . ");
			print_value(list->next);
		}
	}
}

void print_list(Node * list)
{
	printf ("(");
	print_list_body(list);
	printf (")");
}


void print_value(void * value)
{
	if (value == NULL) {
		printf("nil");
		return;
	}

	int type = get_type(value);

	if (type == LIST) print_list((Node *) value);
	else if (type == INT) printf("%d", * ((int32_t *) value));
	else if (type == ID) printf("%s", (char *) value);
	else if (type == STRING) printf("\"%s\"", (char *) value);
	else if (type == LAMBDA) printf("lambda");
	else if (type == SPECIAL) printf("<special>");
  else if (type == PRIMITIVE) printf("<fn-primitive>");
	else printf("unknown type %d %s", type, (char *) value);
}

void println_value(void * value)
{
	print_value(value);
	printf("\n");
}
