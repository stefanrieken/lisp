#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "../tmmh/tmmh.h"

#include "structs.h"

#include "../wonky/wonky.h"

#define new(Type, TYPE) (Type *) allocate_type (sizeof(Type), TYPE)

static inline void * allocate_type(int size, int type)
{
void * bla = allocate(memory, size, false);
set_type(bla, type);
return bla;
}

Node * transform(Node * list, Environment * global, Environment * env);

/**
 * This one may be provided by language implementation (lisp or scheme)
 * to account for different names / behaviours.
 * It may or may not produce a node as a side result; common wisdom says
 * that all LISP expressions should produce a result.
 */
bool transform_definition_expression(Node * list, Environment * global, Environment * env, Node ** result) {
  int type = get_type(list->value);
  if (type != ID) return false;

  char * label = ((char *) list->value);

  // For now this 'define' represents all flavours of 'label', 'set!', etc.
  if (strcmp("define", label) == 0) {
    // always add to the current environment;
    // if that equals the global env
    // then the assignment is available from everywhere
    add_variable(env, (char *) ((Node *) list->next)->value, NULL);

    // This is just because every expression is expected to return a result,
    // and we chain that result in a node.
    // It can be of little use except for clogging up the stack, all the more
    // because at least in Scheme the return value of 'define' is undefined (heh).
    (*result) = new(Node, LIST);
    (*result)->next = NULL;
    (*result)->value = NULL;
    return true;
  }
  // else
  return false;
}

/**
 * As with 'define', 'lambda' is essentially executed during transformation,
 * because we want its environment and arguments in place in the static environment.
 *
 * The result is a 'closure' - a combination of the current environment, extended
 * with argument definitions, and the code body.
 */
bool transform_lambda_expression(Node * list, Environment * global, Environment * env, Node ** result) {
  int type = get_type(list->value);
  if (type != ID) return false;

  char * label = ((char *) list->value);

  if (strcmp("lambda", label) == 0) {
  
    // Don't extend env and fill args here; do it later, on execution.
    // TODO *or* do it here for speed, then run on a copy during execution (and no copy needed if one call at a time!)
    /*
    Environment * lambda_env = new(Environment, ENVIRONMENT);
    lambda_env->parent = env;

    // extract arg names:
    
    if (list->next != NULL && get_type(list->next) == LIST)
    {
      Node * argslist = ((Node *) list->next)->value;
      while (argslist != NULL)
      {
        add_variable(lambda_env, (char *) argslist->value, NULL);
        argslist = argslist->next;
      }
    }*/

    (*result) = new(Node, LIST);
    (*result)->value = env; // = bind
    set_type((*result)->value, LAMBDA);
    (*result)->next = NULL; // copy(list); //transform(list, global, lambda_env); // TODO: transform this list? (Uhh TODO nope dude, at least fish out the args first!)
    return true;
  }
  // else
  return false;
}

/**
 * This is the callback from wonkybytes' eval().
 *
 * We will go by these rules:
 * - If the object is a lambda, use its stored state as a parent and add args.
 * - If the object is a 'let', use the caller_state is a parent and add args (not implemented yet!).
 * - If anything else, just return caller_state.
 */

State * make_new_state_for(Node * function, State * caller_state) {
  if (function == NULL || function->value == NULL) return caller_state;
  
  int type = get_type(function->value);
  if (type == LAMBDA) {
    Environment * env = (Environment *) function->value;
    Environment * lambda_env = new(Environment, ENVIRONMENT);
    lambda_env->parent = env;

    // extract arg names:    
    if (function->next != NULL && get_type(function->next) == LIST)
    {
      Node * argslist = ((Node *) function->next)->value;
      while (argslist != NULL)
      {
        add_variable(lambda_env, (char *) argslist->value, NULL);
        argslist = argslist->next;
      }
    }
    
    State * state = malloc(sizeof(State)); // TODO use TMMH
    state->at = 0;
    state->code_size = 0; //TODO
    state->code = NULL; // TODO
    state->env = lambda_env;
    state->stack = NULL; // TODO
    return state;
  }
  // else
  return caller_state;
}

Node * transform_common_expression(Node * list, Environment * global, Environment * env) {
  int length = 0;

  bool evaluate = true;

  Node * total = NULL;
  Node * result = NULL;

  while(list != NULL)
  {
    if (list->value == NULL) {
      result = new(Node, LIST);
      result->next = NULL;
      result->value = NULL;
      goto process_result; // skip-ahead to bottom of loop
    }

    int type = get_type(list->value);
    if (type == INT && ((intptr_t) list->value) == (((intptr_t) (list->value) << 2) >> 2))
    {
      // in range for in-line int
      result = new(Node, LIST);
      result->next = NULL;
      result->value = (void *) (((intptr_t)list->value) << 2); // encode as int
    }
    else if (type == ID)
    {
      result = new(Node, LIST);
      result->next = NULL;
      if (evaluate) {
        Variable * var = find_variable(env, list->value, true); // first local then global
        if (var == NULL && global != env) var = find_variable(global, list->value, true);
        if (var != NULL) result->value=var->value;
        else result->value = NULL;
      } else {
        result->value= (void *) ( ((intptr_t) list->value) & 0b01); // encode as label reference
      }
    }
    else if (type == LIST)
    {
      // Retain lists as data for as long as possible;
      // only call 'transform' when they get evaluated.
      // IF we have to evaluate at this point, we are faced with a
      // sub-expression: (* (+ x 2) 3) that we put in line.
      if (evaluate) {
        result = transform(list->value, global, env); // may be NULL!
      } else {
        result = new(Node, LIST);
        result->next = NULL;
        result->value = list->value;
      }
    }
    else if (type == SPECIAL || type == PRIMITIVE)
    {
      result = new(Node, LIST);
      result->next = NULL;
      if (type == SPECIAL) evaluate = false; // don't evaluate subsequent args
      result->value = (void *) (((intptr_t) list->value) & 0b11); // retain and encode as primitive
    }

    process_result:

    // Pre-to-postfix inversion:
    if (result != NULL) { // may be null in case of 'define'
      Node * result_end = result;
      while (result_end->next != NULL) {
        result_end = (Node *) result_end->next;
        length++;
      }
      length++; // have at least one new node at this point
      result_end->next = total;
      total = result;
      result = NULL;
    }
    // Proceed to next item
    list = ((Node *) list->next);
  }

  // Optional: transform to array (using length)

  return total;
}
/**
 * Translate pre-fixed, unlinked nodes into
 * postfixed, statically linked wonkybyte nodes.
 */
Node * transform(Node * list, Environment * global, Environment * env)
{
  if (list == NULL) return NULL;

  Node * result = NULL;

  if (transform_definition_expression(list, global, env, &result)) {
    return result;
  } else if(transform_lambda_expression(list, global, env, &result)) {
    return result;
  } else {
    return transform_common_expression(list, global, env);
  }
}
