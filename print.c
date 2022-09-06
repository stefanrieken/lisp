#include <stdio.h>

#include "../tmmh/tmmh.h"

#include "structs.h"
#include "print.h"

void print_list_body(Node * list)
{
	print_value(list->value);
	if (list->next != NULL) {
		if (get_type(list->next) == LIST) {
			printf(" ");
			print_list_body((Node *) list->next);
		} else {
			printf(" . ");
			print_value(list->next);
		}
	}
}

void print_list(Node * list)
{
	printf ("(");
	print_list_body(list);
	printf (")");
}


void print_value(void * value)
{
	if (value == NULL) {
		printf("nil");
		return;
	}

	int type = get_type(value);

	if (type == LIST) print_list((Node *) value);
	else if (type == INT) printf("%d", * ((int32_t *) value));
	else if (type == ID) printf("%s", (char *) value);
	else if (type == STRING) printf("\"%s\"", (char *) value);
	else if (type == LAMBDA) printf("lambda");
	else if (type == SPECIAL) printf("<special>");
  else if (type == PRIMITIVE) printf("<fn-primitive>");
	else printf("unknown type %d %s", type, (char *) value);
}

void println_value(void * value)
{
	print_value(value);
	printf("\n");
}
