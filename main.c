#include <stdio.h>

#include "lisp_parser.h"
#include "lisp_primitives.h"
#include "special.h"
#include "eval.h"
#include "../tmmh/tmmh.h"

void * memory;

static void * read()
{
	return parse_value();
}

int main ()
{
	printf("Welcome to LISP.\n");
	printf("^D to exit.\n");

	pif pifs[] = {pif_none, pif_none, pif_none, pif_none, pif_none, pif_none, pif_none, pif_none};

	memory = tmmh_init(5000, pifs);
	Environment * root_env = (Environment *) allocate(memory, sizeof(Environment), false);

	root_env->parent = NULL;
	root_env->variables = NULL;

	register_specials(root_env);
	register_primitives(root_env);

	// Read, Eval, Print loop
	Node * command = read();
	while (command != NULL)
	{
		Node * result = eval(command, root_env);
		println_value(result);
		command = read();
	}
}
