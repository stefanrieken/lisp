OBJECTS=lisp_parser.o lisp_lexer.o parse_string.o buffered_read.o eval.o structs.o lisp_primitives.o
CFLAGS=-Wall -Wunused

all: lisp scheme

%.o: %.c
	gcc $(CFLAGS) -c $< -o $@

lisp: $(OBJECTS)
	gcc $(CFLAGS) $(OBJECTS) mccarthy.c main.c ../tmmh/libtmmh.a -o lisp
scheme: $(OBJECTS)
	gcc $(CFLAGS) $(OBJECTS) scheme.c main.c ../tmmh/libtmmh.a -o scheme

clean:
	rm -rf lisp scheme *.o
