#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "../tmmh/tmmh.h"

#include "structs.h"

#include "../wonky/wonky.h"

#include "transform.h"

#define new(Type, TYPE) (Type *) allocate_type (sizeof(Type), TYPE)
#define mask(val, m) (((intptr_t) val) & m)

static inline void * allocate_type(int size, int type)
{
  void * bla = allocate(memory, size, false);
  set_type(bla, type);
  return bla;
}

/**
 * This one may be provided by language implementation (lisp or scheme)
 * to account for different names / behaviours.
 * It may or may not produce a node as a side result; common wisdom says
 * that all LISP expressions should produce a result.
 */
bool transform_definition_expression(Node * list, Environment * env, Node ** result) {
  int type = get_type(list->value.ptr);
  if (type != VTYPE_ID) return false;

  char * label = list->value.str;

  // For now this 'define' represents all flavours of 'label', 'set!', etc.
  if (strcmp("define", label) == 0) {
    // always add to the current environment;
    // if that equals the global env
    // then the assignment is available from everywhere
    add_variable(env, list->next.node->value.str, (Element) NULL);

    // This is just because every expression is expected to return a result,
    // and we chain that result in a node.
    // It can be of little use except for clogging up the stack, all the more
    // because at least in Scheme the return value of 'define' is undefined (heh).
    (*result) = new(Node, VTYPE_LIST);
    (*result)->next =(Element) NULL;
    (*result)->value = (Element) NULL; // TODO type this value to avoid 'EVAL'!
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
bool transform_lambda_expression(Node * list, Environment * env, Node ** result) {
  int type = get_type(list->value.ptr);
  if (type != VTYPE_ID) return false;

  char * label = list->value.str;

  if (strcmp("lambda", label) == 0)
  {
    //  Extend env and pre-fill with args as a template
    Environment * lambda_env = new(Environment, VTYPE_ENVIRONMENT); // TODO or VTYPE_LAMBDA?
    lambda_env->parent = env;

    // extract arg names:
    if (list->next.ptr != NULL && get_type(list->next.ptr) == VTYPE_LIST)
    {
      Node * argslist = list->next.node->value.node;
      while (argslist != NULL)
      {
        add_variable(lambda_env, argslist->value.str, (Element) NULL);
        argslist = argslist->next.node;
      }
    }

    // Bind environment to one node before code
    Node * lambda_expr = new(Node, VTYPE_LIST);
    lambda_expr->value.ptr = (void*) (((intptr_t)lambda_env)|NATIVE); // = bind
    lambda_expr->next.node = transform(list->next.node->next.node->value.node, lambda_env);

    // Keep lambda expression out-of-line
    (*result) = new(Node, VTYPE_LIST);
    (*result)->value.ptr = (void*) (((intptr_t)lambda_expr)|NATIVE);
    (*result)->next = (Element) NULL;
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

State * make_new_state_for(void ** function, State * caller_state) {
printf("Making new state for %p %p\n", function, caller_state);

  if (function == NULL) return caller_state;

  int type = get_type((void*) (((intptr_t)(function)) & ~0b11));
  if (type == VTYPE_LAMBDA) {
  printf("it's a lambda!\n");
    Node * lambda = (Node*) (*function);
    Environment * env = (Environment *) lambda->value.ptr;
    Environment * lambda_env = new(Environment, VTYPE_ENVIRONMENT);
    lambda_env->parent = env;

    // extract arg names:
    if (lambda->next.ptr != NULL && get_type(lambda->next.ptr) == VTYPE_LIST)
    {
      Node * argslist = lambda->next.node->value.node;
      while (argslist != NULL)
      {
        add_variable(lambda_env, argslist->value.str, (Element) NULL);
        argslist = argslist->next.node;
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
  else if (type == VTYPE_PRIMITIVE || type == VTYPE_SPECIAL)
  {
    printf("Copying state for prim; passing stack in full.\n");
    State * state = malloc(sizeof(State)); // TODO use TMMH
    state->at = 0;
    state->code_size = 0; //TODO
    state->code = NULL; // TODO
    state->env = caller_state->env;
    state->stack = caller_state->stack; // TODO
    return state;
  }
  // else
  printf("returning caller state %d\n", type);
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
Node * transform_common_expression(Node * list, Environment * env) {
  int length = 0;

  int evaluate = 1; // tri-state: 0=never, 1=always; negative=until re-set to 0 (this is to support 'if')

  Node * total = NULL;
  Node * result = NULL;

  while(list != NULL)
  {
    if (list->value.ptr == NULL) {
//printf("null\n");
      result = new(Node, VTYPE_LIST);
      result->next = (Element) NULL;
      result->value = (Element) NULL; // TODO type this null
      goto process_result; // skip-ahead to bottom of loop
    }

    int type = get_type(list->value.ptr);
    if (type == VTYPE_INT && *(list->value.intptr) == (*(list->value.intptr) << 2) >> 2)
    {
      //printf("in-lining int val %ld\n", *((intptr_t *) list->value));
      // in range for in-line int
      result = new(Node, VTYPE_LIST);
      result->next = (Element) NULL;
      result->value.ptr = (void *) (((*(list->value.intptr)) << 2) | INTEGER); // encode as int
    }
    else if (type == VTYPE_ID)
    {
      result = new(Node, VTYPE_LIST);
      result->next.ptr = NULL;
      if (evaluate != 0) {
        Variable * var = find_variable(env, list->value.str, true);
        if (var != NULL) {
          // hmm, having to jump through the same hoops once more here
          // NOTE: our variables should be basic tagged too!
          Element value = (Element) (var->value.as_int & ~0b11);
          int basictype = var->value.as_int & 0b11;
          if (basictype == LABEL)
          {
            result->value = var->value;
          }
          else if (basictype == INTEGER)
          {
            result->value = var->value; // assume native data at this point
          }
          else if (basictype == NATIVE)
          {
            if(value.ptr != NULL) {
              int var_type = get_type(value.ptr);
              if (type == VTYPE_INT && (*(value.intptr)) == ((*(value.intptr)) << 2) >> 2) {
                result->value.as_int = ((*(value.intptr)) << 2) | INTEGER;
              }
              // TODO shouldn't encounter these anymore; instead see basictype == PRIMITIVE below
              else if (var_type == VTYPE_PRIMITIVE || var_type == VTYPE_SPECIAL) {
                if (var_type == VTYPE_SPECIAL)
                {
                  if (strcmp("if", list->value.str) == 0) evaluate = -2; // hack to only evaluate (this and) the first arg
                  else evaluate = 0;
                }
                result->value.as_int = (value.as_int | PRIMITIVE);
              }
              else if (var_type != VTYPE_ID) {
                result->value.as_int = (value.as_int | NATIVE); // assume native data at this point
              }
            }
          }
          else if (basictype == PRIMITIVE)
          {
            // We can't really tell direct primitives from direct specials.
            // So either we have to wrap them in an indirect pointer again,
            // or we have to work around this differently.
            // As we have established that for optimal results every 'special'
            // call should get a custom treatment here, maybe do that instead.
            if (strcmp("if", list->value.str) == 0) evaluate = -2; // hack to only evaluate (this and) the first arg
            else evaluate = 0;
            result->value = var->value;
          }
        } else {
        printf("null variable: %s\n", list->value.str);
          result->value = (Element) NULL; // TODO type this null
        }
      } else {
        // encode label reference
        // TODO: We can't have labels as normal char pointers IF we want support for negative values (label+eval).
        // But for now it's just char pointers followed by a separate eval.
        result->value.as_int = (list->value.as_int | LABEL); // encode as label reference == leave as-is
      }
    }
    else if (type < VTYPE_ID)
    {
      // Native value pointer
      result = new(Node, VTYPE_LIST);
      result->next.ptr = NULL;
      result->value.as_int = (list->value.as_int | NATIVE); // encode as language-native pointer
    }
    else if (type == VTYPE_LIST)
    {
      // Retain lists as data for as long as possible;
      // only call 'transform' when they get evaluated.
      // IF we have to evaluate at this point, we are faced with a
      // sub-expression: (* (+ x 2) 3) that we put in line.
      if (evaluate != 0) {
        result = transform(list->value.node, env); // may be NULL!
      } else {
        result = new(Node, VTYPE_LIST);
        result->next.ptr = NULL;
        result->value.as_int = (list->value.as_int | NATIVE); // encode as language-native pointer
        // Muhaha we still transform the list anyway!!1!
        //
        result->value.as_int = (((intptr_t) transform(list->value.node, env)) | NATIVE);
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
      while (result_end->next.node != NULL) {
        result_end = result_end->next.node;
        length++;
      }
      length++; // have at least one new node at this point

      if (total == NULL) {
        // add 'eval' statement at end of this expression
        total = new(Node, VTYPE_LIST);
        total->value.as_int = 0;
        total->next.ptr = NULL;
      }

      result_end->next.ptr = total;
      total = result;
      result = NULL;
    }

    // Proceed to next item
    if (evaluate < 0) evaluate++;
    list = ((Node *) list->next.ptr);
  }

  // Optional: transform to array (using length)

  return total;
}
/**
 * Translate pre-fixed, unlinked nodes into
 * postfixed, statically linked wonkybyte nodes.
 */
Node * transform(Node * list, Environment * env)
{
  if (list == NULL) return NULL;

  Node * result = NULL;

  if (transform_definition_expression(list, env, &result)) {
    return result;
  } else if(transform_lambda_expression(list, env, &result)) {
    return result;
  } else {
    return transform_common_expression(list, env);
  }
}
