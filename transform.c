#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "../tmmh/tmmh.h"

#include "structs.h"

#include "../wonky/wonky.h"

#define new(Type, TYPE) (Type *) allocate_type (sizeof(Type), TYPE)
#define mask(val, m) (((intptr_t) val) & m)

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
  if (type != VTYPE_ID) return false;

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
    (*result) = new(Node, VTYPE_LIST);
    (*result)->next = NULL;
    (*result)->value = NULL; // TODO type this value to avoid 'EVAL'!
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
  if (type != VTYPE_ID) return false;

  char * label = ((char *) list->value);

  if (strcmp("lambda", label) == 0)
  {
    //  Extend env and pre-fill with args as a template
    Environment * lambda_env = new(Environment, VTYPE_ENVIRONMENT); // TODO or VTYPE_LAMBDA?
    lambda_env->parent = env;

    // extract arg names:
    if (list->next != NULL && get_type(list->next) == VTYPE_LIST)
    {
      Node * argslist = ((Node *) list->next)->value;
      while (argslist != NULL)
      {
        add_variable(lambda_env, (char *) argslist->value, NULL);
        argslist = argslist->next;
      }
    }

    // Bind environment to one node before code
    Node * lambda_expr = new(Node, VTYPE_LIST);
    lambda_expr->value = (void*) (((intptr_t)lambda_env)|NATIVE); // = bind
    lambda_expr->next = transform(((Node*) ((Node*)list->next)->next)->value, global, lambda_env);

    // Keep lambda expression out-of-line    
    (*result) = new(Node, VTYPE_LIST);
    (*result)->value = (void*) (((intptr_t)lambda_expr)|NATIVE);
    (*result)->next = NULL;
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
  if (type == VTYPE_LAMBDA) {
    Environment * env = (Environment *) function->value;
    Environment * lambda_env = new(Environment, VTYPE_ENVIRONMENT);
    lambda_env->parent = env;

    // extract arg names:    
    if (function->next != NULL && get_type(function->next) == VTYPE_LIST)
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

/**
 * Transformation does these things:
 * 1) Resolve labels to variable slots (not to values; this is to be done at runtime)
 * 2) Recursively transform conditionally executed sub-expressions;
 * 3) Recursively transform and in-line regular sub-expressions.
 * 4) Extend the lexical environment and add variable slots; add code to activate extended environment(TODO)
 * 5) Extend the lexical environment as a function call template and add argument slots 
 * 6) Re-chain to postfix (stack order)
 *
 * Not all of the above are done all of the time:
 *
 * QUOTE: don't resolve labels, don't transform or in-line expressions (sublists). Result order irrelevant: only supports 1 result.
 * LIST: tricky, because actually vararg! 'macroexpand' to conses, or to (list n-items ....) . DO evaluate and transform arguments though %)
 *       Alternative: (list (a b c)) where a b c are transformed but must still be invoked by 'list'
 * DOSEQ: just transform and in-line all expressions and drop the actual call to 'doseq' itself
 * IF: in-line 'cond' block, transform 'then' and 'else' blocks.
 * LET: extend lexical scope, add vars, add code to activate extended environment(TODO)
 * LAMBDA: extend lexical scope, add arg slots; save scope template with code
 *
 */
Node * transform_common_expression(Node * list, Environment * global, Environment * env) {
  int length = 0;

  int evaluate = 1; // tri-state: 0=never, 1=always; negative=until re-set to 0 (this is to support 'if')

  Node * total = NULL;
  Node * result = NULL;

  while(list != NULL)
  {
    if (list->value == NULL) {
//printf("null\n");
      result = new(Node, VTYPE_LIST);
      result->next = NULL;
      result->value = NULL; // TODO type this null
      goto process_result; // skip-ahead to bottom of loop
    }

    int type = get_type(list->value);
    if (type == VTYPE_INT && *((intptr_t *) list->value) == (*((intptr_t *) list->value) << 2) >> 2)
    {
      //printf("in-lining int val %ld\n", *((intptr_t *) list->value));
      // in range for in-line int
      result = new(Node, VTYPE_LIST);
      result->next = NULL;
      result->value = (void *) ((*((intptr_t *) list->value) << 2) | INTEGER); // encode as int
    }
    else if (type == VTYPE_ID)
    {
      result = new(Node, VTYPE_LIST);
      result->next = NULL;
      if (evaluate != 0	) {
        Variable * var = find_variable(env, list->value, true); // first local then global
        if (var == NULL && global != env) var = find_variable(global, list->value, true);
        if (var != NULL) {
          // hmm, having to jump through the same hoops once more here
          if(var->value != NULL) {
            int var_type = get_type(var->value);
            if (type == VTYPE_INT && ((intptr_t) var->value) == (((intptr_t) (var->value) << 2) >> 2)) {
              result->value = (void*) ( (((intptr_t) result->value) << 2 ) | INTEGER);
            }
            else if (var_type == VTYPE_PRIMITIVE || var_type == VTYPE_SPECIAL) {
              if (var_type == VTYPE_SPECIAL)
              {
                if (strcmp("if", list->value) == 0) evaluate = -2; // hack to only evaluate (this and) the first arg
                else evaluate = 0;
              }
              result->value = (void*) (((intptr_t) var->value) | PRIMITIVE);
            }
            else if (var_type != VTYPE_ID) {
              result->value = (void*) (((intptr_t) var->value) | NATIVE); // assume native data at this point
            }
          }
        } else {
          result->value = NULL; // TODO type this null
        }
      } else {
        // encode label reference
        // TODO: We can't have labels as normal char pointers IF we want support for negative values (label+eval).
        // But for now it's just char pointers followed by a separate eval.
        result->value=(void*) (((intptr_t)list->value) | LABEL); // encode as label reference == leave as-is
      }
    }
    else if (type < VTYPE_ID)
    {
      // Native value pointer
      result = new(Node, VTYPE_LIST);
      result->next = NULL;
      result->value = (void*) (((intptr_t) list->value) | NATIVE); // encode as language-native pointer
    }
    else if (type == VTYPE_LIST)
    {
      // Retain lists as data for as long as possible;
      // only call 'transform' when they get evaluated.
      // IF we have to evaluate at this point, we are faced with a
      // sub-expression: (* (+ x 2) 3) that we put in line.
      if (evaluate != 0) {
        result = transform(list->value, global, env); // may be NULL!
      } else {
        result = new(Node, VTYPE_LIST);
        result->next = NULL;
        result->value = (void *) (((intptr_t) list->value) | NATIVE); // encode as language-native pointer
        // Muhaha we still transform the list anyway!!1!
        // 
        result->value = (void*) ((intptr_t)transform(list->value, global, env) | NATIVE);
      }
    }
/*  else if (type == SPECIAL || type == PRIMITIVE)
    {
      // Umm you don't really get here: specials and primitives are found by ID in the root Environment
    }*/
    else
    {
      printf("Could not process:  %d\n", type);
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
      
      if (total == NULL) {
        // add 'eval' statement at end of this expression
        total = new(Node, VTYPE_LIST);
        total->value = 0;
        total->next = NULL;
      }

      result_end->next = total;
      total = result;
      result = NULL;
    }

    // Proceed to next item
    if (evaluate < 0) evaluate++;
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
