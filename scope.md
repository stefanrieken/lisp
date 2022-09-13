
Implementing lexical scoping

1) Library functions, namespace roots, etc. are added to the initial scope.

2) This is the scope initially made available to the application.

3) During execution (or pre-conversion) the initial scope may be:
   a) Permanently extended ('define')
   b) Temporarily extended ('let')
   
4) On function definition, the current scope (extended with the argument slots)
   is stored with the function body; on any subsequent execution, the argument
   values are filled and the function is run against its own scope.

5) Although with dynamic scoping we could grow and shrink a single dictionary
   in line with blocks being executed, with lexical scoping, we must expect a
   temporal sub-scope to occasionally be held indefinitely within a function
   closure. To this end, chains of sub-scopes, also known as spaghetti stacks,
   may be employed.

6) 'define' may be fully handled in pre-conversion, so that after this step all
   variables are statically known; even so, any mention of a variable before
   definition will rightfully produce an error (just during pre-conversion).

7) Wherever code is executed in parallel, with the expectation that a let-block
   or function has separate variable instances per thread, having a template
   for their variables still allows for pre-computing their relative positions.


