#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../tmmh/tmmh.h"

#include "structs.h"
#include "parse.h"

#define new(Type, TYPE) (Type *) allocate_type (sizeof(Type), TYPE)

/**
 * Support for a 1 character read-ahead buffer.
 *
 * This is possible because the syntax uses no multiple-character keywords.
 * The alternative is to parse around the character read.
 * (We do parse it forward sometimes, but not back.)
 */

FILE * _file;

int char_buffer = -2;

void read_from(FILE * file)
{
	_file = file;
}

int buffered_read()
{
	int c;
	if (char_buffer != -2)
	{
		c = char_buffer;
		char_buffer = -2;
	}
	else c = fgetc(_file);

	return c;
}

void buffer_return(int c)
{
	char_buffer = c;
}

/**
 * Category checks.
 */

static inline bool is_whitespace (int c)
{
	return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

static inline bool is_opening_bracket (int c)
{
	return c == '(' || c == '{'  || c == '[';
}

static inline bool is_closing_bracket (int c)
{
	return c == ')' || c == '}' || c == ']';
}

/**
 * Skip-ahead functions.
 */

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

/**
 * Parse a double-quote delimited string, allowing for common C-style escape codes.
 */
char * escapes = "nrtf";
char * replacements = "\n\r\t\f";
int escapes_length = 4;

static inline int replace_escapes(int c)
{
	for (int i=0; i<escapes_length; i++)
		if (escapes[i] == c) return replacements[i];

	return c;
}

// assumes the starting '"' was already parsed
char * parse_string()
{
	int size = 1;
	char * result = (char *) allocate(memory, size, false);
	set_type(result, VTYPE_STRING);
	int c = buffered_read();

	while (c != -1 && c != '"')
	{
		if (c == '\\')
		{
			c = buffered_read();
			if (c != -1)
				c = replace_escapes(c);
		}

		if (c != -1)
		{
			result[size-1] = c;
			result = reallocate(memory, result, ++size, false);
			c = buffered_read();
		}
	}

	result[size-1] = 0;

	if (c != '"') return NULL; // string was not properly closed within length of buffer
	else return result;
}
/**
 * Parse a string of characters not surrounded by quotes.
 *
 * LISP labels have very little limitations, in part to make a point about
 * how something like a number is essentially just another abstract symbol.
 *
 * We make good use of this idea by parsing integers as a label first,
 * before converting them to actual integers.
 */
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
	while (c != -1 && !is_opening_bracket(c) && !is_closing_bracket(c) && !is_whitespace(c));

	if (c != -1) buffer_return(c);

	result[size-1] = 0;
	set_type(result, VTYPE_ID);
	return result;
}

/**
 * Call parse_label first, and then decide wheter we can represent the
 * outcome as an integer. The 'radix' being the base, this function could
 * also be used to parse e.g. hexadecimal numbers (base 16).
 */ 
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
	set_type(pointer, VTYPE_INT);
	(* pointer) = result * sign;

	return pointer;
}

static inline void * allocate_type(int size, int type)
{
	void * bla = allocate(memory, size, false);
	set_type(bla, type);
	return bla;
}

/**
 * Parse the shorthand quote.
 * I'm sure that this ought to be a common label plus a LISP macro,
 * but pending a macro system it is of some value to have it built in.
 */
Node * parse_quote()
{
	Node * node = new (Node, VTYPE_LIST);
	void * quote = allocate_type(6, VTYPE_ID);
	strcpy (quote, "quote");
	node->value = quote;

	Node * next = new (Node, VTYPE_LIST);
	next->next = NULL;
	next->value = parse_value();
	node->next = next;

	return node;
}

/**
 * Parse a LISP list; with support for alternative brackets.
 * Just tell what opening bracket you have already parsed.
 */
Node * parse_list(char opening_bracket)
{
	// the opening bracket is already here.
	
	char closing_bracket = opening_bracket + (opening_bracket == '(' ? 1 : 2); // see ASCII

	int ch = get_non_whitespace_char();
	if (ch == closing_bracket) return NULL; // empty list
	buffer_return(ch);

	Node * pair = new (Node, VTYPE_LIST);

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
		pair->next = parse_list(opening_bracket);
		return pair;
	}
}

/**
 * Parse any valid LISP value.
 *
 * Normally you would call either this function or parse_list to get things going.
 */
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
	else if (is_opening_bracket(ch))
		return parse_list(ch);
	else if (ch != -1) {
		return parse_label_or_number(ch, 10);
	}
	return NULL;
}

