OBJECTS=parse.o print.o eval.o structs.o lisp_primitives.o
OBJECTS2=parse.o print.o ../wonky/eval.o ../wonky/stack.o structs.o lisp_primitives.o
CFLAGS=-Wall -Wunused -Os

all: lisp scheme test test2

%.o: %.c
	gcc $(CFLAGS) -c $< -o $@

lisp: $(OBJECTS) mccarthy.o main.o
	gcc $(CFLAGS) $(OBJECTS) mccarthy.o main.o ../tmmh/libtmmh.a -o lisp
scheme: $(OBJECTS) scheme.o main.o
	gcc $(CFLAGS) $(OBJECTS) scheme.o main.o ../tmmh/libtmmh.a -o scheme
test: $(OBJECTS) scheme.o transform.o test.o
	gcc $(CFLAGS) $(OBJECTS) scheme.o test.o transform.c ../tmmh/libtmmh.a -o test
test2: $(OBJECTS2) scheme.o transform.o test2.o
	gcc $(CFLAGS) $(OBJECTS2) scheme.o test2.o transform.c ../tmmh/libtmmh.a -o test2

clean:
	rm -rf lisp scheme test test2 *.o
