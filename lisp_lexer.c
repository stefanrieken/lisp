// LISP style input lexer
// Note: returns the individual words but doesn't categorize them
// So after parsing, you can't tell a label from a string(!)
// Also, the lexer doesn't convert 

#include "lisp_lexer.h"

static inline bool is_whitespace (int c)
{
	return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

static inline bool is_bracket (int c)
{
	return c == '(' || c == ')' || c == '{' || c == '}' || c == '[' || c == ']';
}

static inline int parse_bracket(int c, char * buffer, int bufferLength)
{
	buffer[0] = (char) c;
	return 1;
}

static inline int get_non_whitespace_char()
{
	int c;
	do
	{
		c = buffered_read();
	}
	while (c != -1 && is_whitespace(c));

	return c;
}

// returns -1 if no success, otherwise size
int parse_label (int c, char * buffer, int buffer_length)
{
	int size = 0; // first character was already parsed
	bool bracket;
	bool at_end;
	do
	{
		buffer[size++] = c;
		c = buffered_read();
		bracket = is_bracket(c);
		if (bracket) buffer_return(c);
		at_end = (c == -1) || bracket || is_whitespace(c);
	}
	while (!at_end && size < buffer_length);

	if (!at_end) return -1; // buffer was too small
	else return size;
}

int parse_word(char * buffer, int buffer_length)
{
	int c = get_non_whitespace_char();

	if (c == -1) return -1;
	else if (is_bracket(c)) return parse_bracket(c, buffer, buffer_length);
	else if (c == '"') return parse_string(buffer, buffer_length);
	else return parse_label(c, buffer, buffer_length);
}

int parse_zero_ending_word(char * buffer, int buffer_length)
{
	int size = parse_word(buffer, buffer_length-1);
	if (size != -1) buffer[size++] = (char) 0;
	return size;
}

void discard_initial_opening_bracket()
{
	int c = get_non_whitespace_char();
	if (c != '(') buffer_return(c);
}
