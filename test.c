#include <stdio.h>
#include "lisp_lexer.h"

char testbuffer[5];

int main()
{
	if (parse_zero_ending_word(testbuffer, 5) != -1)
	{
		printf(testbuffer);
		printf("\n");
	}
	else printf("Could not parse word into buffer.\n");
}

