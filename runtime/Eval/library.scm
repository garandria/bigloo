;*=====================================================================*/
;*    serrano/prgm/project/bigloo/runtime/Eval/library.scm             */
;*    -------------------------------------------------------------    */
;*    Author      :  Manuel Serrano                                    */
;*    Creation    :  Thu Jun 23 15:31:39 2005                          */
;*    Last change :  Tue Sep  8 09:55:09 2009 (serrano)                */
;*    Copyright   :  2005-09 Manuel Serrano                            */
;*    -------------------------------------------------------------    */
;*    The library-load facility                                        */
;*=====================================================================*/

;*---------------------------------------------------------------------*/
;*    The module                                                       */
;*---------------------------------------------------------------------*/
(module __library
   
   (import __error
	   __object
	   __thread
	   __type
	   __bigloo
	   __configure
	   __param
	   __eval
	   __r5_control_features_6_4
	   __everror
	   __expander_srfi0)
   
   (use     __tvector
	    __bexit
	    __bignum
	    __os
	    __foreign
	    __evenv
	    __evmodule
	    __structure
	    
	    __r4_numbers_6_5_fixnum
	    __r4_numbers_6_5_flonum
	    __r4_booleans_6_1
	    __r4_symbols_6_4
	    __r4_vectors_6_8
	    __r4_control_features_6_9
	    __r4_pairs_and_lists_6_3
	    __r4_characters_6_6
	    __r4_equivalence_6_2
	    __r4_strings_6_7
	    __r4_ports_6_10_1
	    __r4_ports_6_10_1
	    __r4_output_6_10_3
	    __r4_input_6_10_2)

   (export  (declare-library! id
			      #!key
			      (version (bigloo-config 'release-number))
			      (basename (symbol->string id))
			      dlopen-init
			      module-init module-eval
			      class-init class-eval
			      init
			      eval
			      (srfi '()))
	    (library-info::obj ::symbol)
	    (library-translation-table-add! ::symbol ::bstring . ::obj)
	    (library-file-name::bstring ::symbol ::bstring ::symbol)
	    (library-load ::obj . opt)
	    (library-exists? ::symbol . opt)))

;*---------------------------------------------------------------------*/
;*    *library-mutex* ...                                              */
;*---------------------------------------------------------------------*/
(define *library-mutex* (make-mutex 'library))

;*---------------------------------------------------------------------*/
;*    libinfo ...                                                      */
;*    -------------------------------------------------------------    */
;*    This structure must be identically defined in                    */
;*    comptime/alibrary.scm                                            */
;*---------------------------------------------------------------------*/
(define-struct libinfo
   id basename version
   init_s init_e
   module_s module_e
   class_s class_e
   init eval srfi)

;*---------------------------------------------------------------------*/
;*    *libraries* ...                                                  */
;*---------------------------------------------------------------------*/
(define *libraries* '())

;*---------------------------------------------------------------------*/
;*    declare-library! ...                                             */
;*---------------------------------------------------------------------*/
(define (declare-library! id
			  #!key
			  (version (bigloo-config 'release-number))
			  (basename (symbol->string id))
			  dlopen-init
			  module-init module-eval
			  class-init class-eval
			  init
			  eval
			  (srfi '()))
   (mutex-lock! *library-mutex*)
   (unless (memq id *libraries*)
      (set! *libraries*
	    (cons (cons id
			(libinfo id basename version
				 (when dlopen-init (format "~a_s" dlopen-init))
				 (when dlopen-init (format "~a_e" dlopen-init))
				 module-init module-eval
				 class-init class-eval
				 init eval srfi))
		  *libraries*))
      (for-each (lambda (s)
		   (register-srfi! s)
		   (register-eval-srfi! s))
		srfi))
   (mutex-unlock! *library-mutex*))

;*---------------------------------------------------------------------*/
;*    library-info ...                                                 */
;*---------------------------------------------------------------------*/
(define (library-info id)
   (cond ((assq id *libraries*) => cdr)))

;*---------------------------------------------------------------------*/
;*    library-translation-table-add! ...                               */
;*---------------------------------------------------------------------*/
(define (library-translation-table-add! name translation . opt)
   (let ((version (bigloo-config 'release-number))
	 (dlopen-init (symbol->string name)))
      (let loop ((opt opt))
	 (when (pair? opt)
	    (cond
	       ((string? (car opt))
		(set! version (car opt))
		(loop (cdr opt)))
	       ((not (car opt))
		(set! version #f)
		(loop (cdr opt)))
	       ((eq? (car opt) :dlopen-init)
		(cond
		   ((null? (cdr opt))
		    (error 'library-translation-table-add!
			   "Illegal :dlopen-init argument"
			   opt))
		   ((not (string? (cadr opt)))
		    (error 'library-translation-table-add!
			   "Illegal :dlopen-init value"
			   opt))
		   (else
		    (set! dlopen-init (cadr opt))
		    (loop (cddr opt)))))
	       (else
		(error 'library-translation-table-add!
		       "Illegal argument"
		       opt)))))
      (mutex-lock! *library-mutex*)
      (set! *libraries*
	    (cons (cons name
			(libinfo name translation version
				 (when dlopen-init
				    (string-append (mangle dlopen-init) "_s"))
				 (when dlopen-init
				    (string-append (mangle dlopen-init) "_e"))
				 #f #f
				 #f #f
				 #f #f #f))
		  *libraries*))
      (mutex-unlock! *library-mutex*)))

;*---------------------------------------------------------------------*/
;*    library-init-file ...                                            */
;*---------------------------------------------------------------------*/
(define (library-init-file lib)
   (string-append (symbol->string lib) ".init"))

;*---------------------------------------------------------------------*/
;*    untranslate-library-name ...                                     */
;*---------------------------------------------------------------------*/
(define (untranslate-library-name library::symbol)
   (let ((info (library-info library)))
      (if info
	  (values (libinfo-basename info) (libinfo-version info))
	  (values (symbol->string library) (bigloo-config 'release-number)))))
   
;*---------------------------------------------------------------------*/
;*    library-file-name ...                                            */
;*---------------------------------------------------------------------*/
(define (library-file-name library suffix backend)
   (define (forge-name base suffix version)
      (cond
	 ((not version)
	  (string-append base suffix))
	 ((string? version)
	  (string-append base suffix "-" version))
	 (else
	  (error 'library-file-name "Illegal version" version))))
   (multiple-value-bind (base version)
      (untranslate-library-name library)
      (case backend
	 ((bigloo-c)
	  (cond
	     ((or (string=? (os-class) "unix")
		  (string=? (os-class) "mingw"))
	      (forge-name base suffix version))
	     ((string=? (os-class) "win32")   
	      (string-append base suffix))
	     (else
	      (error 'library-file-name "Unknown os" (os-class)))))
	 ((bigloo-jvm)
	  (forge-name base suffix version))
	 ((bigloo-.net bigloo.net)
	  (forge-name base suffix version))
	 (else
	  (error 'library-file-name "Illegal backend" backend)))))

;*---------------------------------------------------------------------*/
;*    mangle ...                                                       */
;*---------------------------------------------------------------------*/
(define (mangle name)
   (if (bigloo-need-mangling? name)
       (bigloo-mangle name)
       name))

;*---------------------------------------------------------------------*/
;*    library-load ...                                                 */
;*---------------------------------------------------------------------*/
(define (library-load lib . path)
   (let ((mod (eval-module)))
      ($eval-module-set! (interaction-environment))
      (unwind-protect
	 (cond
	    ((string? lib)
	     (dynamic-load lib))
	    ((not (symbol? lib))
	     (bigloo-type-error 'library-load "string or symbol" lib))
	    (else
	     (let* ((path (if (pair? path)
			      path
			      (let ((venv (getenv "BIGLOOLIB")))
				 (if (not venv)
				     (bigloo-library-path)
				     (cons "." (unix-path->list venv))))))
		    (init (find-file/path (library-init-file lib) path))
		    (be (cond-expand
			   (bigloo-c 'bigloo-c)
			   (bigloo-jvm 'bigloo-jvm)
			   (bigloo-.net 'bigloo-.net))))
		(when init (loadq init))
		(let* ((info (library-info lib))
		       (n (make-shared-lib-name
			   (library-file-name lib "" be) be))
		       (ns (make-shared-lib-name
			    (library-file-name lib "_s" be) be))
		       (ne (make-shared-lib-name
			    (library-file-name lib "_e" be) be))
		       (rsc (let ((p (string-append "/resource/bigloo/"
						    (symbol->string lib)
						    "/make-lib.class")))
			       ;; JVM supports fake file system in the JAR
			       ;; file. In Bigloo it is a sub-directory of
			       ;; /resource inside ressources, we always search
			       ;; for a class named make-lib. This means that
			       ;; all JVM libraries must be provided with
			       ;; such a class.
			       (and (file-exists? p) p)))
		       (libs (find-file/path ns path))
		       (libe (find-file/path ne path))
		       (name (symbol->string lib))
		       (init_s (and info (libinfo-init_s info)))
		       (init_e (and info (libinfo-init_e info)))
		       (module_s (and info
				      (cond-expand
					 (bigloo-c (libinfo-module_s info))
					 (else (libinfo-class_s info)))))
		       (module_e (and info
				      (cond-expand
					 (bigloo-c (libinfo-module_e info))
					 (else (libinfo-class_e info))))))
		   (cond
		      ((and (not (string? rsc)) (not (string? libs)))
		       (error 'library-load
			      (format "Can't find library `~a' (`~a')" lib ns)
			      path))
		      ((not (string? libe))
		       (cond-expand
			  ((not bigloo-jvm)
			   (evmeaning-warning
			    #f
			    'library-load
			    (format "Can't find _e library `~a' (`~a') in path "
				    lib ne)
			    path)))
		       (if (string? libs)
			   (dynamic-load libs init_s module_s)
			   (dynamic-load rsc init_s module_s)))
		      (else
		       (if (string? libs)
			   (dynamic-load libs init_s module_s)
			   (dynamic-load rsc init_s module_s))
		       (dynamic-load libe init_e module_e)))
		   (when (and info (libinfo-init info))
		      (eval '((libinfo-init info))))
		   (when (and info (libinfo-eval info))
		      (eval '((libinfo-eval info))))))))
	 ($eval-module-set! mod))))

;*---------------------------------------------------------------------*/
;*    library-exists? ...                                              */
;*---------------------------------------------------------------------*/
(define (library-exists? lib . path)
   (let* ((path (if (pair? path)
		    path
		    (let ((venv (getenv "BIGLOOLIB")))
		       (if (not venv)
			   (bigloo-library-path)
			   (cons "." (unix-path->list venv))))))
	  (suffix (cond-expand
		     (bigloo-c ".heap")
		     (bigloo-jvm ".jheap")
		     (bigloo-.net ".jheap")))
	  (heap (string-append (symbol->string lib) suffix)))
      (string? (find-file/path heap path))))

