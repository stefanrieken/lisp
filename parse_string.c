#include "parse_string.h"
#include "../tmmh/tmmh.h"
#include "lisp_lexer.h"

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
	char * result = (char *) allocate(size, false);
	set_type(result, STRING);
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
			result = reallocate(result, ++size, false);
			c = buffered_read();
		}
	}

	result[size-1] = 0;

	if (c != '"') return NULL; // string was not properly closed within length of buffer
	else return result;
}


