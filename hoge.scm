(name! reduce 
  (lambda (fn list acm)
    (if (null? list)
	acm
	(reduce fn (cdr list) (fn (car list) acm)))))

(name! reverse
  (lambda (list)
    (reduce cons list '())))
 
(name! map1
  (lambda (fn list)
    (name! map1_tail
      (lambda (fn list acm)
	(if (null? list)
	    (reverse acm)
	    (map1_tail fn (cdr list) (cons (fn (car list)) acm)))))
    (map1_tail fn list '())))

(name! first  (lambda (l) (car l)))
(name! second (lambda (l) (car (cdr l))))
(name! third  (lambda (l) (car (cdr (cdr l)))))

(name! sc-macro-transformer
       (lambda (expander)
	 (lambda (form mac use)
	   (make-syntactic-closure mac
				   '()
				   (expander form use)))))

(name! rsc-macro-transformer
       (lambda (expander)
	 (lambda (form mac use)
	   (make-syntactic-closure use
				   (expander form mac)))))

(define-syntax define
  (rsc-macro-transformer
   (lambda (form env)
     (if (pair? (second form))
	 (list (make-syntactic-closure env '() 'name!)
	       (car (second form))
	       (cons (make-syntactic-closure env '() 'lambda)
		     (cons (cdr (second form))
			   (cdr (cdr form)))))
	 (list (make-syntactic-closure env '() 'name!)
	       (car (cdr form))
	       (car (cdr (cdr form))))))))

(define-syntax let
  (rsc-macro-transformer
   (lambda (form env)
     (if (symbol? (second form))
	 ((lambda (name args bosy)
	     (list (list (make-syntactic-closure env '() lambda) 
			 '()
			 (cons (make-syntactic-closure env '() 'define)
			       (cons (cons name (map1 car args)) body))
			 (cons name (map1 cdr args)))))
	  (second form)
	  (third form)
	  (cdr (cdr (cdr form))))
	 ((lambda (args body)
	     (cons (cons (make-syntactic-closure env '() 'lambda)
			 (cons (map1 first args)
			       body)) (map1 second args)))
	  (second form)
	  (cdr (cdr form)))))))

(let fib ((n 10) (m 1) (l 1))
  (if (= n 0)
      m
      (fib (- n 1) l (+ m l))))
