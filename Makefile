OBJECTS=parse.o print.o eval.o structs.o lisp_primitives.o
CFLAGS=-Wall -Wunused -Os

all: lisp scheme

%.o: %.c
	gcc $(CFLAGS) -c $< -o $@

lisp: $(OBJECTS) mccarthy.o main.o
	gcc $(CFLAGS) $(OBJECTS) mccarthy.o main.o ../tmmh/libtmmh.a -o lisp
scheme: $(OBJECTS) scheme.o main.o
	gcc $(CFLAGS) $(OBJECTS) scheme.o main.o ../tmmh/libtmmh.a -o scheme

clean:
	rm -rf lisp scheme *.o
