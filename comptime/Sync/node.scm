;*=====================================================================*/
;*    serrano/prgm/project/bigloo/comptime/Sync/node.scm               */
;*    -------------------------------------------------------------    */
;*    Author      :  Manuel Serrano                                    */
;*    Creation    :  Sun Nov 18 08:38:02 2012                          */
;*    Last change :  Mon Dec 10 00:02:23 2012 (serrano)                */
;*    Copyright   :  2012 Manuel Serrano                               */
;*    -------------------------------------------------------------    */
;*    SYNC2NODE, this expands a SYNC node into a plain node using      */
;*    explicitly lock/unlock and push/pop operations. Used by the      */
;*    C backend. The JVM backend should, someday, compile directly     */
;*    a synchronize block and should not use this expansion.           */
;*=====================================================================*/

;*---------------------------------------------------------------------*/
;*    The module                                                       */
;*---------------------------------------------------------------------*/
(module sync_node
   
   (include "Tools/trace.sch"
	    "Tools/location.sch")
   
   (import  tools_error
	    tools_shape
	    engine_param
	    type_type
	    type_tools
	    type_cache
	    type_typeof
	    object_class
	    object_slots
	    ast_var
	    ast_node
	    ast_local
	    ast_sexp
	    ast_app
	    ast_dump
	    backend_backend
	    inline_app
	    effect_effect
	    backend_cplib
	    sync_failsafe)
   
   (export (sync->sequence::node ::sync)))

;*---------------------------------------------------------------------*/
;*    lock cache                                                       */
;*---------------------------------------------------------------------*/
(define mlock #f)
(define mlockprelock #f)
(define mulock #f)
(define mpush #f)
(define mpop #f)
(define getexitdtop #f)

;*---------------------------------------------------------------------*/
;*    init-sync! ...                                                   */
;*---------------------------------------------------------------------*/
(define (init-sync! loc)
   (unless mlock
      (set! getexitdtop (sexp->node '$get-exitd-top '() loc 'app))
      (set! mlock (sexp->node '$mutex-lock '() loc 'app))
      (set! mlockprelock (sexp->node '$mutex-lock-prelock '() loc 'app))
      (set! mulock (sexp->node '$mutex-unlock '() loc 'app))
      (case (backend-language (the-backend))
	 ((c)
	  (set! mpush (sexp->node '$exitd-push-mutex! '() loc 'app))
	  (set! mpop (sexp->node '$exitd-pop-mutex! '() loc 'app)))
	 (else
	  (set! mpush (sexp->node '(@ exitd-push-mutex! __bexit) '() loc 'app))
	  (set! mpop (sexp->node '(@ exitd-pop-mutex! __bexit) '() loc 'app))))
      (set-variable-name! (var-variable getexitdtop))
      (set-variable-name! (var-variable mlock))
      (set-variable-name! (var-variable mlockprelock))
      (set-variable-name! (var-variable mulock))
      (set-variable-name! (var-variable mpush))
      (set-variable-name! (var-variable mpop))))

;*---------------------------------------------------------------------*/
;*    sync->sequence ...                                               */
;*    -------------------------------------------------------------    */
;*    This function performs the following expansions:                 */
;*    1- if prelock is NIL:                                            */
;*        (sync m body)                                                */
;*      =>                                                             */
;*        (begin                                                       */
;*           ($mutex-lock m)                                           */
;*           ((@ exitd-push-mutex! __bexit) m)                         */
;*           (let ((tmp body))                                         */
;*              ((@ exitd-pop-mutex! __bexit) m)                       */
;*              ($mutex-unlock m)))                                    */
;*    2- if prelock is not NIL:                                        */
;*        (sync m prelock body)                                        */
;*      =>                                                             */
;*        (begin                                                       */
;*           ($mutex-lock-prelock m prelock)                           */
;*           ((@ exitd-push-mutex! __bexit) m)                         */
;*           (let ((tmp body))                                         */
;*              ((@ exitd-pop-mutex! __bexit) m)                       */
;*              ($mutex-unlock m)))                                    */
;*---------------------------------------------------------------------*/
(define (sync->sequence node::sync)
   
   (define (app expr loc)
      (application->node expr '() loc 'value))

   (define (failsafe-sync->sequence node)
      ;; no exception raised, avoid pushing/poping mutexes
      (with-access::sync node (loc nodes mutex type prelock)
	 (let* ((tmp (make-local-svar (gensym 'tmp) type))
		(lock (if (atom? prelock)
			  (app `(,mlock ,mutex) loc)
			  (app `(,mlockprelock ,mutex ,prelock) loc)))
		(unlock (app `(,mulock ,mutex) loc))
		(vref (instantiate::var
			 (loc loc)
			 (type type)
			 (variable tmp)))
		(sbody (if (and (pair? nodes) (null? (cdr nodes)))
			   (car nodes)
			   (instantiate::sequence
			      (loc loc)
			      (type type)
			      (nodes nodes))))
		(lbody (instantiate::let-var
			  (loc loc)
			  (type type)
			  (bindings (list (cons tmp sbody)))
			  (body (instantiate::sequence
				   (loc loc)
				   (type type)
				   (nodes (list unlock vref)))))))
	    (instantiate::sequence
	       (loc loc)
	       (type type)
	       (nodes (list lock lbody))))))
   
   (define (effect-sync->sequence node)
      ;; exceptions potentially raised, slow path compilation
      (with-access::sync node (loc nodes mutex type prelock)
	 (let* ((tmp (make-local-svar (gensym 'tmp) type))
		(top (make-local-svar (gensym 'top) *obj*))
		(topref (instantiate::var
			   (loc loc)
			   (type *obj*)
			   (variable top)))
		(gettop (app `(,getexitdtop) loc))
		(lock (if (atom? prelock)
			  (app `(,mlock ,mutex) loc)
			  (app `(,mlockprelock ,mutex ,prelock) loc)))
		(push (app `(,mpush ,topref ,mutex) loc))
		(pop (app `(,mpop ,topref ,mutex) loc))
		(unlock (app `(,mulock ,mutex) loc))
		(vref (instantiate::var
			 (loc loc)
			 (type type)
			 (variable tmp)))
		(sbody (if (and (pair? nodes) (null? (cdr nodes)))
			   (car nodes)
			   (instantiate::sequence
			      (loc loc)
			      (type type)
			      (nodes nodes))))
		(lbody (instantiate::let-var
			  (loc loc)
			  (type type)
			  (bindings (list (cons tmp sbody)))
			  (body (instantiate::sequence
				   (loc loc)
				   (type type)
				   (nodes (list pop unlock vref)))))))
	    (instantiate::let-var
	       (loc loc)
	       (type type)
	       (bindings (list (cons top gettop)))
	       (body (instantiate::sequence
			(loc loc)
			(type type)
			(nodes (list lock push lbody))))))))
   
   (with-access::sync node (loc)
      (init-sync! loc))
   
   (if (failsafe-sync? node)
       (failsafe-sync->sequence node)
       (effect-sync->sequence node)))
