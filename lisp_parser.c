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

#define new(bla) (bla *) malloc(sizeof(bla))

static inline Node * create_list_node (Node * value)
{
	Node * node = (Node *) new(Node);
	node->list_value = value;
	node->value_type = LIST;
	return node;
}

static inline Node * create_string_node (char * value)
{
	Node * node = (Node *) new(Node);
	node->string_value = value;
	node->value_type = STRING;
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

	while (list != NULL)
	{
		printf(sep);
		if (list->value_type == LIST) print_list(list->list_value);
		if (list->value_type == STRING) printf(list->string_value);
		if (list->value_type == ENVIRONMENT) printf("lambda");
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
