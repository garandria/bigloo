/*---------------------------------------------------------------------*/
/*    serrano/prgm/project/bigloo/runtime/Clib/cmain.c                 */
/*                                                                     */
/*    Author      :  Manuel Serrano                                    */
/*    Creation    :  Fri Jul 17 09:40:49 1992                          */
/*    Last change :  Sat Nov 20 05:57:50 2010 (serrano)                */
/*                                                                     */
/*    Le fichier de main de toute application. Comme je m'y prends     */
/*    plus intelligement que dans la version 0.8 (si, si :-), je       */
/*    peux compiler une fois pour toute ce fichier. Je ne suis plus    */
/*    oblige de le construire et le compiler pour chaque application.  */
/*---------------------------------------------------------------------*/
#include <bigloo.h>
#include <time.h>
#if( (defined( sun ) && !defined( __svr4__) && !defined( __SVR4 )) \
     || defined( sony_news ) )
#   include <machine/vmparam.h>
#else
#   if( defined( sun ) && (defined( __svr4__ ) || defined( __SVR4 )) )
#      include <sys/vmparam.h>
#   endif
#endif

/*---------------------------------------------------------------------*/
/*    Heap handling                                                    */
/*---------------------------------------------------------------------*/
#define MegToByte(x) ((x) * (1024 * 1024))
#define MegToKilo(x) ((x) * 1024)
#define MegTo4K(x)   ((x) * (1024 >> 2))

#define DEFAULT_HEAP_SIZE 4

/* The initial heap size (in mega byte) */
BGL_RUNTIME_DEF long heap_size = DEFAULT_HEAP_SIZE;

/*---------------------------------------------------------------------*/
/*    Des recuperations externes                                       */
/*---------------------------------------------------------------------*/
extern void cref();
extern void memshow();
extern void bgl_init_eval_cnst();
extern obj_t c_constant_string_to_string();
extern obj_t c_error();
extern void bgl_init_objects();
extern obj_t bigloo_exit_apply();
extern void *bgl_callcc_get_top_of_stack( void * );
extern void bgl_end_io();
extern void GC_set_all_interior_pointers( int );

#ifndef _MSC_VER
extern char *getenv();
#endif

/*---------------------------------------------------------------------*/
/*    Une variable pour memoriser le bas de la pile                    */
/*---------------------------------------------------------------------*/
long glob_dummy;

BGL_RUNTIME_DEF obj_t command_line = 0L;
char *executable_name = 0L;
char **bgl_envp;
int bgl_envp_len;

/*---------------------------------------------------------------------*/
/*    obj_t                                                            */
/*    exit_mutex ...                                                   */
/*---------------------------------------------------------------------*/
extern obj_t bgl_exit_mutex();

/*---------------------------------------------------------------------*/
/*    bgl_get_top_of_stack ...                                         */
/*    -------------------------------------------------------------    */
/*    This function can't be included inside callcc.c otherwise it may */
/*    be incorrectly inlined by optimizing C compilers.                */
/*---------------------------------------------------------------------*/
void *
bgl_get_top_of_stack() {
   void *dummy;
   
   return bgl_callcc_get_top_of_stack( &dummy );
}

/*---------------------------------------------------------------------*/
/*    bigloo_exit ...                                                  */
/*---------------------------------------------------------------------*/
BGL_RUNTIME_DEF
obj_t
bigloo_exit( obj_t val ) {
   int n;
   val = bigloo_exit_apply( val );

   bgl_mutex_lock( bgl_exit_mutex() );

   bgl_end_io();
   
   n = (long)INTEGERP( val ) ? (long)CINT( val ) : 0;
   exit( n );
   
   bgl_mutex_unlock( bgl_exit_mutex() );
   
   return val;
}

/*---------------------------------------------------------------------*/
/*    bigloo_backend ...                                               */
/*---------------------------------------------------------------------*/
BGL_RUNTIME_DEF
char *
bigloo_backend() {
   return "bigloo-c";
}

/*---------------------------------------------------------------------*/
/*    bigloo_main ...                                                  */
/*    -------------------------------------------------------------    */
/*    Le vrai point d'entree d'une application `bigloo'. La fonction   */
/*    `main' qui est cree par le compilo ne sert a rien. Elle existe   */
/*    uniquement car je ne suis pas sur que tous les compilos supporte */
/*    que la fonction C `main' soit mise en librairie. Comme ca, je    */
/*    suis tranquile.                                                  */
/*---------------------------------------------------------------------*/
BGL_EXPORTED_DEF
int
_bigloo_main( int argc,
	      char *argv[],
	      char *env[],
	      obj_t (*bigloo_main)(obj_t),
	      int (*libinit)(int, char *[], char *[]),
   	      long uheapsize ) {
   long  mega_size;
   char *env_size;
   obj_t cons;
   long  i;

   /* we store the global environment */
   bgl_envp = env;
   bgl_envp_len = 0;

   if( env ) {
      char **runner = env;
      while( *runner ) bgl_envp_len++, runner++;
   }

   /* on initialise le tas */
   if( !(env_size = getenv( "BIGLOOHEAP" )) ) {
      mega_size = uheapsize ? uheapsize : DEFAULT_HEAP_SIZE;
   } else {
      mega_size = atoi( env_size );
   }

#if( BGL_GC == BGL_BOEHM_GC )
   heap_size = MegToByte( mega_size );
#endif

   if( !INIT_ALLOCATION( heap_size ) ) {
      char mes[ 600 ];

      sprintf( mes, "%ldMB wanted", heap_size );
      c_error( "Can't allocate heap", mes, -10 );
      return 1;
   } else {
      /* initialize the libraries */
      libinit( argc, argv, env );
      
      /* store the executable name */
      executable_name = argv[ 0 ];

      /* setup preallocated objects (chars, bool, ...) */
      bgl_init_objects();

      /* store the stack bottom address */
      BGL_ENV_STACK_BOTTOM_SET( BGL_CURRENT_DYNAMIC_ENV(), (char *)&argc );

      /* initialize constants */
      bgl_init_eval_cnst();

      /* build the Bigloo command line list */
      for( i = argc - 1, cons = BNIL; i >= 0; i-- )
         cons = MAKE_PAIR( c_constant_string_to_string( argv[ i ] ), cons );

      command_line = cons;

      /* random seed */
      {
	 time_t taux;
	 struct tm *tm;
	 time( &taux );
	 tm = gmtime( &taux );
	    
	 srand( (24 * (60 * tm->tm_sec + tm->tm_min)) + tm->tm_hour );

#if( BGL_HAVE_GMP )
	 /* big numbers init */
	 gmp_randinit_default( gmp_random_state );
	 gmp_randseed_ui( gmp_random_state,
			  (24 * (60 * tm->tm_sec + tm->tm_min)) + tm->tm_hour );
#endif	 
      }

      /* jump into the application main */
      bigloo_main( cons );

      return 0;
   }
}
