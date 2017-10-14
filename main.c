#include "eval.h"

static Node * read()
{
	discard_initial_opening_bracket();
	return parse_list();
}

int main ()
{
	printf("Welcome to LISP.\n");
	printf("^D to exit.\n");

	Environment * root_env = (Environment *) malloc(sizeof(Environment));

	// Read, Eval, Print loop
	Node * command = read();
	while (command != NULL)
	{
		Node * result = eval(command, root_env);
		println_list(result);
		command = read();
	}
}
