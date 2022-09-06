;;
;; This file contains some basic tests using the 'scheme' dialect. Run with ./scheme < test.scm
;;
(set! (quote a) (lambda (x) (list "Message from fn a:" x)))
(set! (quote b) (lambda (x) (list "Message from fn b:" x)))
(a "hi")
((if(eq 1 2) a b) "bye")

