#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../tmmh/tmmh.h"

#include "structs.h"
#include "parse_string.h"
#include "parse.h"

#define new(Type, TYPE) (Type *) allocate_type (sizeof(Type), TYPE)

int char_buffer = -2;

int buffered_read()
{
	int c;
	if (char_buffer != -2)
	{
		c = char_buffer;
		char_buffer = -2;
	}
	else c = getchar();

	return c;
}

void buffer_return(int c)
{
	char_buffer = c;
}

static inline bool is_whitespace (int c)
{
	return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

static inline bool is_bracket (int c)
{
	return c == '(' || c == ')' || c == '{' || c == '}' || c == '[' || c == ']';
}

int get_non_whitespace_char()
{
	int c;
	do
	{
		c = buffered_read();
	}
	while (c != -1 && is_whitespace(c));

	return c;
}

void skip_line()
{
	int c;
	do
	{
		c = buffered_read();
	}
	while (c != -1 && c != '\n');
}

char * parse_label(int c)
{
	int size = 1;
	char * result = (char *) allocate(memory, size, false);

	do
	{
		result[size-1] = (char) c;
		result = reallocate(memory, result, ++size, false);
		c = buffered_read();
	}
	while (c != -1 && !is_bracket(c) && !is_whitespace(c));

	if (c != -1) buffer_return(c);

	result[size-1] = 0;
	set_type(result, ID);
	return result;
}

// 'intptr_t' is the 'int' that is of pointer width
// (so on 64-bit machines it is 64-bit instead of 32)
void * parse_label_or_number (int c, int radix)
{
	char * str = parse_label(c);
	intptr_t result = 0;
	int sign = 1;

	int i=0;
	if (str[i] == '-')
	{
		sign = -1;
		i++;
	}
	while (str[i] != 0)
	{
		int intval = str[i++] - '0';
		if (intval < 0 || intval >= radix) return str; // give up
		result = (result * radix) + intval;
	}

	intptr_t * pointer = allocate(memory, 4, false);
	set_type(pointer, INT);
	(* pointer) = result * sign;

	return pointer;
}

static inline void * allocate_type(int size, int type)
{
	void * bla = allocate(memory, size, false);
	set_type(bla, type);
	return bla;
}

void * parse_value()
{
	int ch = get_non_whitespace_char();
	while (ch == ';') {
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

