This is another LISP parser research project, this time in C.

State
-----
It has most of the traditional logic primitives implemented.

This project is in the transition of using tmmh for its heap management and data typing.
It is intended to do manual GC flushing, but this part is not yet incorporated, so currently
memory usage will just 'heap up'.

Besides string values, this LISP supports integers at system pointer size. The idea is for it
to be easily used for memory manipulation. Currently you can already e.g. say:

	(label a "42")                    ; this is the antique McCarthy version of 'define'
	(label b (value-at (address a)))  ; equivalent to (label b a)
	b                                 ; evaluates to "42"
	(address b)		          ; address of value; produces same pointer as (address a)

Error handling
--------------
I have currently designed the built-in functions so that they return a useful value on success,
even if it's only echoing (part of) their inputs.
Invalid input is currently silently refused by returning a NULL value in code,
which is shown as an empty list.
In part, this reflects the concept of 'true' being any value but NIL; and NIL being the empty list.

But be aware that errors can currently silently be converted into empty lists,
which might even be successfully processed further down. I'm sure that this system won't hold in the long run,
but for now it helps me following the intuition of 'no side effects'.

Side effects
------------
The 'label' primitive, which I didn't invent myself, modifies the current state.
This is slightly annoying because apart from that, the current system is completely side-effect free.
Input is only accepted in the form of input data, output is purely return values.
This is part laziness part conscious approach.

The 'label' primitive is what you get when you model your core on a late 1950's experiment.
Maybe it would pay to re-model the core primitives on e.g. Scheme in the future.
I'm curious to compare notes between the two.


Hello
-----
Here's how you can currently get a hello-world-like result on stack:

	(label greet (lambda (x) (cons "hello" (cons x (quote ())))))
	(greet "world")

Note that if I'd just quote 'hello' and 'x', 'x' would never get evaluated.
In my mind, this just goes to show how dense LISP (originally) really is.
Remember they initially tried to make an easier-to-understand Turing machine, not a practical language!
But it also goes to show why people call the macro functionality of LISP 'a great tool'.
Personally I'm unsure whether hiding such dense syntax under a carpet of macros is an advertisement for any language.

This very simple LISP has no macro support at the moment.
I did add a 'list' primitive which takes the biggest load off the above:

	(label greet (lambda (x) (list "hello" x)))
	(greet world)

It is also already possible to directly use an anonymous function:

	((lambda (x y) (list y x)) "world" "hello")

Or even to use remainder args:

	((lambda (x . y) (list (car y) (car (cdr y)))) "print" "hello" "world")

And pair notation in general:

	'("hello" . ("world" . ()))

