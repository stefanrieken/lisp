all:
	gcc -Wall main.c lisp_parser.c lisp_lexer.c parse_string.c buffered_read.c eval.c -o lisp
clean:
	rm lisp
