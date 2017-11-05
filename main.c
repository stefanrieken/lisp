#include <stdio.h>

#include "lisp_parser.h"
#include "eval.h"
#include "../tmmh/tmmh.h"

static void * read()
{
	return parse_value();
}

int main ()
{
	printf("Welcome to LISP.\n");
	printf("^D to exit.\n");

	pif pifs[] = {pif_none, pif_none, pif_none, pif_none, pif_none, pif_none, pif_none, pif_none};

	tmmh_init(5000, pifs);
	Environment * root_env = (Environment *) allocate(sizeof(Environment), false);
	root_env->parent = NULL;
	root_env->variables = NULL;

	// Read, Eval, Print loop
	Node * command = read();
	while (command != NULL)
	{
		Node * result = eval(command, root_env);
		println_value(result);
		command = read();
	}
}
