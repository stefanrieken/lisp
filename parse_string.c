#include "parse_string.h"

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
// returns -1 if no success, otherwise size
int parse_string(char * buffer, int bufferLength)
{
	int size = 0;
	int c = buffered_read();

	while (c != -1 && c != '"' && size < bufferLength)
	{
		if (c == '\\')
		{
			c = buffered_read();
			if (c != -1)
				buffer[size++] = replace_escapes(c);
		}
		else buffer[size++] = c;

		c = buffered_read();
	}

	if (c != '"') return -1; // string was not properly closed within length of buffer
	else return size;
}


