OBJECTS=parse.o print.o eval.o structs.o lisp_primitives.o
CFLAGS=-Wall -Wunused -Os

all: lisp scheme test

%.o: %.c
	gcc $(CFLAGS) -c $< -o $@

lisp: $(OBJECTS) mccarthy.o main.o
	gcc $(CFLAGS) $(OBJECTS) mccarthy.o main.o ../tmmh/libtmmh.a -o lisp
scheme: $(OBJECTS) scheme.o main.o
	gcc $(CFLAGS) $(OBJECTS) scheme.o main.o ../tmmh/libtmmh.a -o scheme
test: $(OBJECTS) scheme.o transform.o test.o
	gcc $(CFLAGS) $(OBJECTS) scheme.o test.o transform.c ../tmmh/libtmmh.a -o test

clean:
	rm -rf lisp scheme test *.o
