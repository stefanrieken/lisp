// This is an implementation of 'cons cells'.
// Interesting fact: the original IBM 704 had 36-bit words,
// divided into a 15-bit 'address register', another 15-bit 'decrement register'
// and a 6-bit 'tag'. This just happened to be the ideal storage for a 'cons cell'.
// On modern computers, the struct is awkwardly mis-aligned.
// There are different solutions for it:
// - storing the type with the value, diminishing pointer / integer expressiveness
// - dropping the 'cons cells' structure altogether
//   (see e.g. https://clojure.org/reference/sequences)
// This implementation explores the traditional approach, ignoring optimization.

#include "lisp_parser.h"
#include "../tmmh/tmmh.h"

static inline Node * create_list_node (Node * value)
{
	Node * node = (Node *) allocate(sizeof(Node), false);
	set_type(node, LIST);
	node->value = value;
	node->next = NULL;
	return node;
}

static inline Node * create_string_node (char * value)
{
	Node * node = (Node *) allocate(sizeof(Node), false);
	set_type(node, LIST);
	node->value = value;
	node->next = NULL;
	return node;
}

Node * parse_list()
{
	Node * first_node = NULL;
	Node * last_node = NULL;

	char * word = parse_zero_ending_word();

	while (word != NULL)
	{
		if (word[0] == ')') // end of this list
			return first_node;

		Node * new_node;

		if (word[0] == '(') new_node = create_list_node(parse_list());
		else new_node = create_string_node(word);

		if (last_node != NULL)
		{
			last_node->next = new_node;
			last_node = new_node;
		}
		else last_node = new_node;

		if (first_node == NULL) first_node = new_node;

		word = parse_zero_ending_word();
	}

	return first_node;
}

void print_list(Node * list)
{
	printf ("(");
	char * sep = "";

	while (list != NULL && list->value != NULL)
	{
		printf(sep);
		int type = get_type(list->value);

		if (type == LIST) print_list(list->value);
		else if (type == ID) printf(list->value);
		else if (type == STRING) printf("\"%s\"", (char *) list->value);
		else if (type == LAMBDA) printf("lambda");
		else printf("unknown type %d %s", type, (char *) list->value);
		sep = " ";

		list = list->next;
	}

	printf(")");
}

void println_list(Node * list)
{
	print_list(list);
	printf("\n");
}
