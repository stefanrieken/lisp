#include "buffered_read.h"

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
