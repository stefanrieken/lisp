#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "structs.h"
#include "parse.h"
#include "print.h"
#include "lisp_primitives.h"
#include "special.h"
#include "eval.h"
#include "../tmmh/tmmh.h"

void * memory;

int main (int argc, char ** argv)
{
	FILE * source = stdin;
	if (argc > 1) {
		source = fopen(argv[1], "rb");
		if (source == NULL) {
			printf("Could not open file: %s\n", argv[1]);
			exit(-1);
		}
	}

	if (isatty(fileno(source))) {
		printf("Welcome to LISP.\n");
		printf("^D to exit.\n");
	}

	pif pifs[] = {pif_none, pif_none, pif_none, pif_none, pif_none, pif_none, pif_none, pif_none};

	memory = tmmh_init(5000, pifs);
	Environment * root_env = (Environment *) allocate(memory, sizeof(Environment), false);

	root_env->parent = NULL;
	root_env->variables = NULL;

	register_specials(root_env);
	register_primitives(root_env);

	read_from(source);

	// Read, Eval, Print loop
	Node * command = parse_value();
	while (command != NULL)
	{
		Node * result = eval(command, root_env);
		println_value(result);
		command = parse_value();
	}
	
	fclose(source);
}
