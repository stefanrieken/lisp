OBJECTS=parse.o print.o eval.o structs.o lisp_primitives.o
CFLAGS=-Wall -Wunused -Os

all: lisp scheme

%.o: %.c
	gcc $(CFLAGS) -c $< -o $@

lisp: $(OBJECTS) scheme.c
	gcc $(CFLAGS) $(OBJECTS) mccarthy.c main.c ../tmmh/libtmmh.a -o lisp
scheme: $(OBJECTS) scheme.c
	gcc $(CFLAGS) $(OBJECTS) scheme.c main.c ../tmmh/libtmmh.a -o scheme

clean:
	rm -rf lisp scheme *.o
