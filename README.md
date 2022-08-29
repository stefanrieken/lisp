This is another LISP parser research project, this time in C.

State
-----
It has most of the traditional logic primitives implemented.

This project is in the transition of using tmmh for its heap management and data typing.
It is possible to do manual GC flushing, but not yet heap compression.

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
The achaic 'label' primitive modifies the current state. This is slightly
annoying because apart from that, the current system is completely side-effect
free. Input is only accepted in the form of input data, output is purely return
values. This is part laziness part conscious approach.

The 'label' primitive is what you get when you model your core on a late 1950's
experiment. Maybe it would pay to re-model the core primitives on e.g. Scheme
in the future. I'm curious to compare notes between the two.

On Authenticity, S-expressions and linked lists
-----------------------------------------------
So as said we follow McCarthy's original list of special / fundamental forms /
functions, in order to study the point he was making with them.

Much else has changed in LISP since that original 1960 paper that we (and this
implementation) now take for granted, including the abandoning of
'M-expressions' (that only shortly served to make a point on programs vs data),
and the introduction of common lexical scoping.

Nevertheless, one low-level syntax quirk that made it all the way to the present
day is that of the dotted-pair 'cons cell' notation, such that:

  (a b) == (a . (b . ()))

and therefore:

  (a b) != (a . b)

This more or less dictates a linked-list based implementation, which happened to
be very effective on the machine McCarthy worked on, where a pair of pointers
fitted snugly into a single machine word (and could be split using the assembly
macros 'car' and 'cdr'!). Paradoxically, some 60odd years later it is actually
somewhat of a challenge to recreate this concept with a similar elegance - yet
the majority of implementations do not question this particular choice.

We may debate the possibility of implemementing a LISP using arrays, perhaps
bridging the surface differences by, say, using an optional terminator
character and supporting array slices. But for the purpose of studying its
mechanics, this implementation will stick to the original idea.

On special values
-----------------
In his 1960 paper, McCarthy defines NIL as "an atomic symbol used to terminate
lists"; and he introduces T and F without comment to represent true and false.
This idea, of using (common) symbols to represent values, is after all what he
had intended LISP to be about. It was later picked up again by Scheme, but in the
intermediate, the characteristic usage in LISP would be that NIL was equivalent
to the empty list; and to false; and any other value would evaluate to true.

Our "McCarthy" implementation actually follows the latter idea, up to the point
where my "empty lists" are actually implemented as NULL values in C.
There are all kinds of subtle implications; one is that our "original McCarthy"
implementation of "atom" says that `(atom NIL)` is false (that is, it returns NIL).
We might implement the opposite approach as part of our study of Scheme.

On special forms
----------------
The term special / fundamental form may either refer to the most basic functional
building blocks required to construct a LISP environment; or only to those
that need special (=conditional) evaluation of their arguments during runtime.

McCarthy's list contains both, as functions like "atom" and "eq" evaluate like
all other functions. In contrast, the functions marked as "fundamental forms" in
Scheme are only those that require altered evaluation, but they serve very little
pracitcal value without the bulk of "standard procedures" coming alongside it.

I do not intend to build a full R5RS as a consequence. Instead, it may be of
interest to implement Scheme's primitive equivalents of McCarthy's functions
(e.g. let instead of label, if instead of cond) just to evaluate their
respecive merits.

On Macros
---------
From a distance, macros seem to be both complicated and fundamental to LISP.
They don't need to be either. One defines a LISP macro by supplying both a
match pattern and a replacement, which the system then adds to its registry
and subsequently applies to any incoming code, which is after all list data.
(In LISP the command is defmacrol in Scheme it is define-syntax, which may sound
more intimidating but really is the same thing.)

It is true that the effect of a macro can not always be replicated in plain
code, but if it really comes to that, the same can always be achieved by means
of an additional primitive function. For a small embedded interpreter (for
which LISP is a perfect match as well), this may be the more economical
solution.

Having said that, implementing them should make for an interesting study.

Hello
-----
Here's how you can currently get a hello-world-like result on stack:

	(label greet (lambda (x) (cons "hello" (cons x (quote ())))))
	(greet "world")

Note that if I'd just quote 'hello' and 'x', 'x' would never get evaluated.
In my mind, this just goes to show how dense LISP (originally) really is.
Remember they initially tried to make an easier-to-understand Turing machine,
not a practical language!

I did add a 'list' primitive which takes the biggest load off the above:

	(label greet (lambda (x) (list "hello" x)))
	(greet world)

It is also already possible to directly use an anonymous function:

	((lambda (x y) (list y x)) "world" "hello")

Or even to use remainder args:

	((lambda (x . y) (list (car y) (car (cdr y)))) "print" "hello" "world")

And pair notation in general:

	'("hello" . ("world" . ()))

