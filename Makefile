all:
	gcc -Wall -Wunused main.c lisp_parser.c lisp_lexer.c parse_string.c buffered_read.c eval.c structs.c lisp_primitives.c ../tmmh/libtmmh.a -o lisp
clean:
	rm lisp
