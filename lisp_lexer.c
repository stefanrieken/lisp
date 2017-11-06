// LISP style input lexer
// Note: returns the individual words but doesn't categorize them
// So after parsing, you can't tell a label from a string(!)
// Also, the lexer doesn't convert 

#include "lisp_lexer.h"
#include "structs.h"
#include "../tmmh/tmmh.h"

static inline bool is_whitespace (int c)
{
	return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

static inline bool is_bracket (int c)
{
	return c == '(' || c == ')' || c == '{' || c == '}' || c == '[' || c == ']';
}

char * parse_bracket(int c)
{
	char * value = (char *) allocate(2, false);
	value[0] = c;
	value[1] = 0;
	return value;
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

char * parse_label (int c)
{
	int size = 1;
	char * result = (char *) allocate(size, false);

	do
	{
		result[size-1] = (char) c;
		result = reallocate(result, ++size, false);
		c = buffered_read();
	}
	while (c != -1 && !is_bracket(c) && !is_whitespace(c));

	if (c != -1) buffer_return(c);

	result[size-1] = 0;
	set_type(result, ID);
	return result;
}

