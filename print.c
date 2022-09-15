#include <stdio.h>

#include "../tmmh/tmmh.h"

#include "structs.h"
#include "print.h"

void print_list_body(Node * list)
{
	print_value(list->value);
	if (list->next.ptr != NULL) {
		if (get_type(list->next.ptr) == VTYPE_LIST) {
			printf(" ");
			print_list_body((Node *) list->next.node);
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


void print_value(Element value)
{
	if (value.ptr == NULL) {
		printf("nil");
		return;
	}

	int type = get_type(value.ptr);

	if (type == VTYPE_LIST) print_list(value.node);
	else if (type == VTYPE_INT) printf("%ld", *(value.intptr));
	else if (type == VTYPE_ID) printf("%s", value.str);
	else if (type == VTYPE_STRING) printf("\"%s\"", value.str);
	else if (type == VTYPE_LAMBDA) printf("lambda");
	else if (type == VTYPE_SPECIAL) printf("<special>");
	else if (type == VTYPE_PRIMITIVE) printf("<fn-primitive>");
	else printf("unknown type %d %s", type, value.str);
}

void println_value(Element value)
{
	print_value(value);
	printf("\n");
}
