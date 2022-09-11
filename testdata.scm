
;; A single expression to run through tests


(if [eq 2 2]
	{(lambda (x) (list "This test is" x)) "successful!"}
)

;; After transform, expect:
;;
;; ( ) 2 2 eq <apply> if <apply>
;;  |
;;  v
;; "successful" lambda <apply>
