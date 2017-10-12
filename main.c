#include "lisp_parser.h"

int main ()
{
	printf("Welcome to LISP.\n");
	printf("Well... at this point, to a primitive parser.\n");
	printf("Your input will be converted into a LISP 'cons cell' structure\n");
	printf("and printed back to you.\n");
	printf("You don't need to start the top list with a '('.\n");
	printf("Type ')' or Ctrl+D (a few times) to exit.\n");

	discard_initial_opening_bracket();
	Node * list = parse_list();
	print_list(list);
	printf("\n");
}
