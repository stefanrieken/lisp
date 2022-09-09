;;
;; This file contains some basic tests using the 'lisp' dialect. Run with ./lisp < test.lisp
;;
(label a (lambda (x) (list "Message from fn a:" x)))
(label b (lambda (x) (list "Message from fn b:" x)))
(a "hi")
((cond ([eq 1 2] a) ([eq 1 1] b)) "bye")

