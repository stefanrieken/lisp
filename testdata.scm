
;; A single expression to run through tests


(if [eq 2 2]
	{(lambda (x) (list "This test is" x)) "successful!"}
)

;; After transform, expect:
;;
;; "successful" lambda <apply> 2 2 eq <apply> if <apply>
