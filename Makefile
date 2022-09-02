all:
	gcc -Wall -Wunused main.c lisp_parser.c lisp_lexer.c parse_string.c buffered_read.c eval.c structs.c lisp_primitives.c mccarthy.c ../tmmh/libtmmh.a -o lisp
	gcc -Wall -Wunused main.c lisp_parser.c lisp_lexer.c parse_string.c buffered_read.c eval.c structs.c lisp_primitives.c scheme.c ../tmmh/libtmmh.a -o scheme
clean:
	rm lisp
