/*=====================================================================*/
/*    serrano/prgm/project/bigloo/api/libuv/src/Clib/bgluv.c           */
/*    -------------------------------------------------------------    */
/*    Author      :  Manuel Serrano                                    */
/*    Creation    :  Tue May  6 13:53:14 2014                          */
/*    Last change :  Mon Sep 22 17:53:06 2014 (serrano)                */
/*    Copyright   :  2014 Manuel Serrano                               */
/*    -------------------------------------------------------------    */
/*    LIBUV Bigloo C binding                                           */
/*=====================================================================*/
#include <bigloo.h>

#include <uv.h>

#include "bgluv.h"

/*---------------------------------------------------------------------*/
/*    type alias                                                       */
/*---------------------------------------------------------------------*/
typedef BgL_uvloopz00_bglt bgl_uv_loop_t;
typedef BgL_uvfilez00_bglt bgl_uv_file_t;
typedef BgL_uvhandlez00_bglt bgl_uv_handle_t;
typedef BgL_uvstreamz00_bglt bgl_uv_stream_t;
typedef BgL_uvwatcherz00_bglt bgl_uv_watcher_t;
typedef BgL_uvasyncz00_bglt bgl_uv_async_t;

/*---------------------------------------------------------------------*/
/*    bgl_uv_mutex                                                     */
/*---------------------------------------------------------------------*/
extern obj_t bgl_uv_mutex;
extern obj_t bgl_make_input_port( obj_t, FILE *, obj_t, obj_t );
extern obj_t bgl_uv_new_file( int, obj_t );

/*---------------------------------------------------------------------*/
/*    obj_t                                                            */
/*    gc_marks ...                                                     */
/*---------------------------------------------------------------------*/
obj_t gc_marks = BNIL;

/*---------------------------------------------------------------------*/
/*    static void                                                      */
/*    gc_mark ...                                                      */
/*---------------------------------------------------------------------*/
static void
gc_mark( obj_t obj ) {
   BGL_MUTEX_LOCK( bgl_uv_mutex );
   gc_marks = MAKE_PAIR( obj, gc_marks );
   BGL_MUTEX_UNLOCK( bgl_uv_mutex );
}

/*---------------------------------------------------------------------*/
/*    void                                                             */
/*    gc_unmark ...                                                    */
/*---------------------------------------------------------------------*/
void
gc_unmark( obj_t obj ) {
   BGL_MUTEX_LOCK( bgl_uv_mutex );
   gc_marks = bgl_remq( obj, gc_marks );
   BGL_MUTEX_UNLOCK( bgl_uv_mutex );
}

/*---------------------------------------------------------------------*/
/*    void                                                             */
/*    bgl_uv_close_cb ...                                              */
/*---------------------------------------------------------------------*/
void
bgl_uv_close_cb( uv_handle_t *handle ) {
   bgl_uv_handle_t o = (bgl_uv_handle_t)handle->data;
   obj_t p = o->BgL_onclosez00;

   if( PROCEDUREP( p ) ) PROCEDURE_ENTRY( p )( p, BEOA );
}

/*---------------------------------------------------------------------*/
/*    void                                                             */
/*    bgl_uv_handle_cb ...                                             */
/*---------------------------------------------------------------------*/
void
bgl_uv_handle_cb( uv_handle_t *handle, int status ) {
   bgl_uv_watcher_t o = (bgl_uv_watcher_t)handle->data;
   obj_t p = o->BgL_cbz00;

   if( PROCEDUREP( p ) ) PROCEDURE_ENTRY( p )( p, o, BINT( status ), BEOA );
}

/*---------------------------------------------------------------------*/
/*    uv_timer_t *                                                     */
/*    bgl_uv_timer_new ...                                             */
/*---------------------------------------------------------------------*/
uv_timer_t *
bgl_uv_timer_new( BgL_uvtimerz00_bglt o, bgl_uv_loop_t loop ) {
   uv_timer_t *new = (uv_timer_t *)GC_MALLOC( sizeof( uv_timer_t ) );
   new->data = o;
   new->close_cb = &bgl_uv_close_cb;

   uv_timer_init( (uv_loop_t *)loop->BgL_z42builtinz42, new );
   return new;
}

/*---------------------------------------------------------------------*/
/*    uv_idle_t *                                                      */
/*    bgl_uv_idle_new ...                                              */
/*---------------------------------------------------------------------*/
uv_idle_t *
bgl_uv_idle_new( BgL_uvidlez00_bglt o, bgl_uv_loop_t loop ) {
   uv_idle_t *new = (uv_idle_t *)GC_MALLOC( sizeof( uv_idle_t ) );
   new->data = o;
   new->close_cb = &bgl_uv_close_cb;

   uv_idle_init( (uv_loop_t *)loop->BgL_z42builtinz42, new );
   return new;
}

/*---------------------------------------------------------------------*/
/*    static void                                                      */
/*    bgl_uv_async_cb ...                                              */
/*---------------------------------------------------------------------*/
static void
bgl_uv_async_cb( uv_async_t *handle ) {
   bgl_uv_watcher_t o = (bgl_uv_watcher_t)handle->data;
   obj_t p = o->BgL_cbz00;

   if( PROCEDUREP( p ) ) {
      PROCEDURE_ENTRY( p )( p, o, BEOA );
   }
}

/*---------------------------------------------------------------------*/
/*    uv_async_t *                                                     */
/*    bgl_uv_async_new ...                                             */
/*---------------------------------------------------------------------*/
uv_async_t *
bgl_uv_async_new( bgl_uv_async_t o, bgl_uv_loop_t loop ) {
   uv_async_t *new = (uv_async_t *)GC_MALLOC( sizeof( uv_async_t ) );
   new->data = o;

   uv_async_init( (uv_loop_t *)loop->BgL_z42builtinz42, new, &bgl_uv_async_cb );
   return new;
}

/*---------------------------------------------------------------------*/
/*    static obj_t                                                     */
/*    _irq ...                                                         */
/*---------------------------------------------------------------------*/
static obj_t _irq = BUNSPEC, _idle, _sys, _nice, _user, _times, _speed, _model;

/*---------------------------------------------------------------------*/
/*    static void                                                      */
/*    uv_init_cpus ...                                                 */
/*---------------------------------------------------------------------*/
static void
uv_init_cpus() {

   if( _irq == BUNSPEC ) {
      _irq = string_to_symbol( "irq" );
      _idle = string_to_symbol( "idle" );
      _sys = string_to_symbol( "sys" );
      _nice = string_to_symbol( "nice" );
      _user = string_to_symbol( "user" );
      _times = string_to_symbol( "times" );
      _speed = string_to_symbol( "speed" );
      _model = string_to_symbol( "model" );
   }
}
   
/*---------------------------------------------------------------------*/
/*    obj_t                                                            */
/*    bgl_uv_cpu ...                                                   */
/*---------------------------------------------------------------------*/
static obj_t
bgl_uv_cpu( uv_cpu_info_t cpu ) {
   obj_t res = BNIL;
   obj_t times = BNIL;

   times = MAKE_PAIR(
      MAKE_PAIR( _irq, BGL_INT64_TO_BINT64( cpu.cpu_times.irq ) ),
      times );
   times = MAKE_PAIR(
      MAKE_PAIR( _idle, BGL_INT64_TO_BINT64( cpu.cpu_times.idle ) ),
      times );
   times = MAKE_PAIR(
      MAKE_PAIR( _sys, BGL_INT64_TO_BINT64( cpu.cpu_times.sys ) ),
      times );
   times = MAKE_PAIR(
      MAKE_PAIR( _nice, BGL_INT64_TO_BINT64( cpu.cpu_times.nice) ),
      times );
   times = MAKE_PAIR(
      MAKE_PAIR( _user, BGL_INT64_TO_BINT64( cpu.cpu_times.user ) ),
      times );

   res = MAKE_PAIR( MAKE_PAIR( _times, times ), res );
   res = MAKE_PAIR( MAKE_PAIR( _speed, BINT( cpu.speed ) ), res );
   res = MAKE_PAIR( MAKE_PAIR( _model, string_to_bstring( cpu.model ) ), res );

   return res;
}

/*---------------------------------------------------------------------*/
/*    obj_t                                                            */
/*    bgl_uv_cpus ...                                                  */
/*---------------------------------------------------------------------*/
obj_t
bgl_uv_cpus() {
   int count;
   uv_cpu_info_t *cpus;

   if( uv_cpu_info( &cpus, &count ) ) {
      return create_vector( 0 );
   } else {
      obj_t vec = create_vector( count );
      int i;

      uv_init_cpus();
      
      for( i = 0; i < count; i++ ) {
	 VECTOR_SET( vec, i, bgl_uv_cpu( cpus[ i ] ) );
      }

      uv_free_cpu_info( cpus, count );
      return vec;
   }
}

/*---------------------------------------------------------------------*/
/*    BGL_UV_FS_WRAPPER ...                                            */
/*---------------------------------------------------------------------*/
#define BGL_UV_FS_WRAPPER0( name, obj ) { \
   uv_loop_t *loop = (uv_loop_t *)bloop->BgL_z42builtinz42; \
   int r; \
   if( bgl_check_fs_cb( proc, 1, #name ) ) { \
      uv_fs_t *req = (uv_fs_t *)malloc( sizeof( uv_fs_t ) ); \
      req->data = proc; \
      if( (r = name( loop, req, obj, &bgl_uv_fs_cb ) >= 0) ) { \
        gc_mark( proc ); \
      } else { \
        free( req ); \
      } \
      return r; \
   } else { \
      uv_fs_t req; \
      if( (r = name( loop, &req, obj, 0L )) >= 0 ) { \
         r = req.result; \
      } \
      uv_fs_req_cleanup( &req ); \
      return r; \
   } \
}

#define BGL_UV_FS_WRAPPER1( name, obj, arg ) { \
   uv_loop_t *loop = (uv_loop_t *)bloop->BgL_z42builtinz42; \
   int r; \
   if( bgl_check_fs_cb( proc, 1, #name ) ) { \
      uv_fs_t *req = (uv_fs_t *)malloc( sizeof( uv_fs_t ) ); \
      req->data = proc; \
      if( (r = name( loop, req, obj, arg, &bgl_uv_fs_cb ) >= 0) ) { \
        gc_mark( proc ); \
      } else { \
        free( req ); \
      } \
      return r; \
   } else { \
      uv_fs_t req; \
      if( (r = name( loop, &req, obj, arg, 0L )) >= 0 ) { \
        r = req.result; \
      } \
      uv_fs_req_cleanup( &req ); \
      return r; \
   } \
}

#define BGL_UV_FS_WRAPPER2( name, obj, arg0, arg1 ) {	\
   uv_loop_t *loop = (uv_loop_t *)bloop->BgL_z42builtinz42; \
   int r; \
   if( bgl_check_fs_cb( proc, 1, #name ) ) { \
      uv_fs_t *req = (uv_fs_t *)malloc( sizeof( uv_fs_t ) ); \
      req->data = proc; \
      if( (r = name( loop, req, obj, arg0, arg1, &bgl_uv_fs_cb ) >= 0) ) { \
        gc_mark( proc ); \
      } else { \
        free( req ); \
      } \
      return r; \
   } else { \
      uv_fs_t req; \
      if( (r = name( loop, &req, obj, arg0, arg1, 0L )) >= 0 ) { \
        r = req.result; \
      } \
      uv_fs_req_cleanup( &req ); \
      return r; \
   } \
}

/*---------------------------------------------------------------------*/
/*    static void                                                      */
/*    bgl_uv_fs_cb ...                                                 */
/*---------------------------------------------------------------------*/
static void
bgl_uv_fs_cb( uv_fs_t *req ) {
   obj_t p = (obj_t)req->data;

   gc_unmark( p );

   if( PROCEDUREP( p ) ) {
      PROCEDURE_ENTRY( p )( p, BINT( req->result ), BEOA );
   }
   
   uv_fs_req_cleanup( req );
   free( req );
}

/*---------------------------------------------------------------------*/
/*    static int                                                       */
/*    bgl_check_fs_cb ...                                              */
/*---------------------------------------------------------------------*/
static int
bgl_check_fs_cb( obj_t proc, int arity, char *fun ) {
   if( PROCEDUREP( proc ) ) {
      if( PROCEDURE_CORRECT_ARITYP( proc, arity ) ) {
	 return 1;
      } else {
	 C_SYSTEM_FAILURE( BGL_TYPE_ERROR, fun,
			   "wrong callback arity", proc );
	 return -1;
      }
   } else {
      return 0;
   }
}

/*---------------------------------------------------------------------*/
/*    int                                                              */
/*    bgl_uv_fs_rename ...                                             */
/*---------------------------------------------------------------------*/
int
bgl_uv_fs_rename( char *oldp, char *newp, obj_t proc, bgl_uv_loop_t bloop ) {
   BGL_UV_FS_WRAPPER1( uv_fs_rename, oldp, newp )
}

/*---------------------------------------------------------------------*/
/*    int                                                              */
/*    bgl_uv_fs_ftruncate ...                                          */
/*---------------------------------------------------------------------*/
int
bgl_uv_fs_ftruncate( obj_t obj, long offset, obj_t proc, bgl_uv_loop_t bloop ) {
   int fd = ((bgl_uv_file_t)obj)->BgL_fdz00;

   BGL_UV_FS_WRAPPER1( uv_fs_ftruncate, fd, offset )
}

/*---------------------------------------------------------------------*/
/*    int                                                              */
/*    bgl_uv_fs_fchown ...                                             */
/*---------------------------------------------------------------------*/
int
bgl_uv_fs_fchown( obj_t obj, int uid, int gid, obj_t proc, bgl_uv_loop_t bloop ) {
   int fd = ((bgl_uv_file_t)obj)->BgL_fdz00;
   
   BGL_UV_FS_WRAPPER2( uv_fs_fchown, fd, uid, gid )
}

/*---------------------------------------------------------------------*/
/*    int                                                              */
/*    bgl_uv_fs_lchown ...                                             */
/*---------------------------------------------------------------------*/
int
bgl_uv_fs_lchown( char *path, int uid, int gid, obj_t proc, bgl_uv_loop_t bloop ) {
   C_SYSTEM_FAILURE( BGL_IO_PORT_ERROR, "uv_fs_lchown",
		     "Not implemented", proc );
}

/*---------------------------------------------------------------------*/
/*    int                                                              */
/*    bgl_uv_fs_chown ...                                              */
/*---------------------------------------------------------------------*/
int
bgl_uv_fs_chown( char *path, int uid, int gid, obj_t proc, bgl_uv_loop_t bloop ) {
   BGL_UV_FS_WRAPPER2( uv_fs_chown, path, uid, gid )
}

/*---------------------------------------------------------------------*/
/*    int                                                              */
/*    bgl_uv_fs_fchmod ...                                             */
/*---------------------------------------------------------------------*/
int
bgl_uv_fs_fchmod( obj_t obj, int mod, obj_t proc, bgl_uv_loop_t bloop ) {
   int fd = ((bgl_uv_file_t)obj)->BgL_fdz00;
   
   BGL_UV_FS_WRAPPER1( uv_fs_fchmod, fd, mod )
}

/*---------------------------------------------------------------------*/
/*    int                                                              */
/*    bgl_uv_fs_chmod ...                                              */
/*---------------------------------------------------------------------*/
int
bgl_uv_fs_chmod( char *path, int mod, obj_t proc, bgl_uv_loop_t bloop ) {
   BGL_UV_FS_WRAPPER1( uv_fs_chmod, path, mod )
}

/*---------------------------------------------------------------------*/
/*    static void                                                      */
/*    bgl_uv_fs_open_cb ...                                            */
/*---------------------------------------------------------------------*/
static void
bgl_uv_fs_open_cb( uv_fs_t* req ) {
   obj_t proc = (obj_t)req->data;
   obj_t obj;

   gc_unmark( req->data );

   if( req->result <= 0 ) {
      obj = BINT( req->result );
   } else {
      obj_t name = string_to_bstring( (char *)req->path );
      obj = bgl_uv_new_file( req->result, name );
      ((bgl_uv_file_t)obj)->BgL_z52readreqz52 =
	 GC_MALLOC( sizeof( uv_fs_t ) );
   }
   
   uv_fs_req_cleanup( req );
   free( req );

   PROCEDURE_ENTRY( proc )( proc, obj, BEOA );
}

/*---------------------------------------------------------------------*/
/*    obj_t                                                            */
/*    bgl_uv_fs_open ...                                               */
/*---------------------------------------------------------------------*/
obj_t
bgl_uv_fs_open( obj_t bpath, int flags, int mode, obj_t proc, bgl_uv_loop_t bloop ) {
   uv_loop_t *loop = (uv_loop_t *)bloop->BgL_z42builtinz42;
   char *path = BSTRING_TO_STRING( bpath );

   if( bgl_check_fs_cb( proc, 1, "uv-fs-open" ) ) {
      uv_fs_t *req = (uv_fs_t *)malloc( sizeof( uv_fs_t ) );
   
      req->data = proc;
      gc_mark( req->data );

      uv_fs_open( loop, req, path, flags, mode, bgl_uv_fs_open_cb );

      return BUNSPEC;
   } else {
      uv_fs_t req;
      obj_t res;

      uv_fs_open( loop, &req, path, flags, mode, 0L );

      if( req.result <= 0 ) {
	 res = BINT( req.result );
      } else {
	 res = bgl_uv_new_file( req.result, bpath );
	 ((bgl_uv_file_t)res)->BgL_z52readreqz52 =
	    GC_MALLOC( sizeof( uv_fs_t ) );
      }

      uv_fs_req_cleanup( &req );

      
      return res;
   }
}

/*---------------------------------------------------------------------*/
/*    int                                                              */
/*    bgl_uv_fs_close ...                                              */
/*---------------------------------------------------------------------*/
int
bgl_uv_fs_close( obj_t port, obj_t proc, bgl_uv_loop_t bloop ) {
   int fd = ((bgl_uv_file_t)port)->BgL_fdz00;

   BGL_UV_FS_WRAPPER0( uv_fs_close, fd )
}

/*---------------------------------------------------------------------*/
/*    statbuf ...                                                      */
/*---------------------------------------------------------------------*/
static obj_t _dev = BUNSPEC, _mode, _nlink, _uid, _gid, _rdev, _ino, _size;
static obj_t _blksize, _blocks, _flags, _gen, _atime, _mtime, _ctime;

/*---------------------------------------------------------------------*/
/*    static void                                                      */
/*    uv_init_stat ...                                                 */
/*---------------------------------------------------------------------*/
static void
uv_init_stat() {
   if( _dev == BUNSPEC ) {
      _dev = string_to_symbol( "dev" );
      _mode = string_to_symbol( "mode" );
      _nlink = string_to_symbol( "nlink" );
      _uid = string_to_symbol( "uid" );
      _gid = string_to_symbol( "gid" );
      _rdev = string_to_symbol( "rdev" );
      _ino = string_to_symbol( "ino" );
      _size = string_to_symbol( "size" );
      _blksize = string_to_symbol( "blksize" );
      _blocks = string_to_symbol( "blocks" );
      _flags = string_to_symbol( "flags" );
      _gen = string_to_symbol( "gen" );
      _atime = string_to_symbol( "atime" );
      _mtime = string_to_symbol( "mtime" );
      _ctime = string_to_symbol( "ctime" );
   }
}
   
/*---------------------------------------------------------------------*/
/*    obj_t                                                            */
/*    bgl_uv_fstat ...                                                 */
/*---------------------------------------------------------------------*/
obj_t
bgl_uv_fstat( uv_stat_t buf ) {
   obj_t res = BNIL;

   uv_init_stat();

   fprintf( stderr, "bgl_uv_fstat=%ld\n", buf.st_mode );
   res = MAKE_PAIR(
      MAKE_PAIR( _ctime, ELONG_TO_BELONG( buf.st_ctim.tv_sec ) ),
      res );
   res = MAKE_PAIR(
      MAKE_PAIR( _mtime, ELONG_TO_BELONG( buf.st_mtim.tv_sec ) ),
      res );
   res = MAKE_PAIR(
      MAKE_PAIR( _atime, ELONG_TO_BELONG( buf.st_atim.tv_sec ) ),
      res );
   res = MAKE_PAIR(
      MAKE_PAIR( _gen, BGL_INT64_TO_BINT64( buf.st_gen ) ),
      res );
   res = MAKE_PAIR(
      MAKE_PAIR( _flags, BGL_INT64_TO_BINT64( buf.st_flags ) ),
      res );
   res = MAKE_PAIR(
      MAKE_PAIR( _blocks, BGL_INT64_TO_BINT64( buf.st_blocks ) ),
      res );
   res = MAKE_PAIR(
      MAKE_PAIR( _blksize, BGL_INT64_TO_BINT64( buf.st_blksize ) ),
      res );
   res = MAKE_PAIR(
      MAKE_PAIR( _size, BGL_INT64_TO_BINT64( buf.st_size ) ),
      res );
   res = MAKE_PAIR(
      MAKE_PAIR( _ino, BGL_INT64_TO_BINT64( buf.st_ino ) ),
      res );
   res = MAKE_PAIR(
      MAKE_PAIR( _rdev, BGL_INT64_TO_BINT64( buf.st_rdev ) ),
      res );
   res = MAKE_PAIR(
      MAKE_PAIR( _gid, BGL_INT64_TO_BINT64( buf.st_gid ) ),
      res );
   res = MAKE_PAIR(
      MAKE_PAIR( _uid, BGL_INT64_TO_BINT64( buf.st_uid ) ),
      res );
   res = MAKE_PAIR(
      MAKE_PAIR( _nlink, BGL_INT64_TO_BINT64( buf.st_nlink ) ),
      res );
   res = MAKE_PAIR(
      MAKE_PAIR( _mode, BGL_INT64_TO_BINT64( buf.st_mode ) ),
      res );
   res = MAKE_PAIR(
      MAKE_PAIR( _dev, BGL_INT64_TO_BINT64( buf.st_dev ) ),
      res );

   return res;
}

/*---------------------------------------------------------------------*/
/*    static void                                                      */
/*    bgl_uv_fs_fstat_cb ...                                           */
/*---------------------------------------------------------------------*/
static void
bgl_uv_fs_fstat_cb( uv_fs_t *req ) {
   obj_t p = (obj_t)req->data;

   gc_unmark( p );

   if( req->result < 0 ) {
      PROCEDURE_ENTRY( p )( p, BINT( req->result ), BEOA );
   } else {
      PROCEDURE_ENTRY( p )( p, bgl_uv_fstat( req->statbuf ), BEOA );
   }

   uv_fs_req_cleanup( req );
   free( req );
}

/*---------------------------------------------------------------------*/
/*    obj_t                                                            */
/*    bgl_uv_fs_fstat ...                                              */
/*---------------------------------------------------------------------*/
obj_t
bgl_uv_fs_fstat( obj_t port, obj_t proc, bgl_uv_loop_t bloop ) {
   uv_loop_t *loop = (uv_loop_t *)bloop->BgL_z42builtinz42;
   int fd = ((bgl_uv_file_t)port)->BgL_fdz00;

   if( bgl_check_fs_cb( proc, 1, "uv_fs_fstat" ) ) {
      uv_fs_t *req = (uv_fs_t *)malloc( sizeof( uv_fs_t ) );
      req->data = proc;
      gc_mark( proc );
      
      uv_fs_fstat( loop, req, fd, &bgl_uv_fs_fstat_cb );
      
      return BUNSPEC;
   } else {
      uv_fs_t req;

      if( uv_fs_fstat( loop, &req, fd, 0L ) < 0 ) {
	 uv_fs_req_cleanup( &req );
	 return BINT( req.result );
      } else {
	 obj_t res = bgl_uv_fstat( req.statbuf );

	 uv_fs_req_cleanup( &req );

	 return res;
      }
   }
}

/*---------------------------------------------------------------------*/
/*    obj_t                                                            */
/*    bgl_uv_fs_lstat ...                                              */
/*---------------------------------------------------------------------*/
obj_t
bgl_uv_fs_lstat( char *path, obj_t proc, bgl_uv_loop_t bloop ) {
   uv_loop_t *loop = (uv_loop_t *)bloop->BgL_z42builtinz42;

   if( bgl_check_fs_cb( proc, 1, "uv_fs_lstat" ) ) {
      uv_fs_t *req = (uv_fs_t *)malloc( sizeof( uv_fs_t ) );
      req->data = proc;
      gc_mark( proc );
      
      uv_fs_lstat( loop, req, path, &bgl_uv_fs_fstat_cb );
      
      return BUNSPEC;
   } else {
      uv_fs_t req;

      if( uv_fs_lstat( loop, &req, path, 0L ) < 0 ) {
	 uv_fs_req_cleanup( &req );
	 return BINT( req.result );
      } else {
	 obj_t res = bgl_uv_fstat( req.statbuf );

	 uv_fs_req_cleanup( &req );

	 return res;
      }
   }
}

/*---------------------------------------------------------------------*/
/*    obj_t                                                            */
/*    bgl_uv_fs_stat ...                                               */
/*---------------------------------------------------------------------*/
obj_t
bgl_uv_fs_stat( char *path, obj_t proc, bgl_uv_loop_t bloop ) {
   uv_loop_t *loop = (uv_loop_t *)bloop->BgL_z42builtinz42;

   if( bgl_check_fs_cb( proc, 1, "uv_fs_lstat" ) ) {
      uv_fs_t *req = (uv_fs_t *)malloc( sizeof( uv_fs_t ) );
      req->data = proc;
      gc_mark( proc );
      
      uv_fs_stat( loop, req, path, &bgl_uv_fs_fstat_cb );
      
      return BUNSPEC;
   } else {
      uv_fs_t req;

      if( uv_fs_stat( loop, &req, path, 0L ) < 0 ) {
	 uv_fs_req_cleanup( &req );
	 return BINT( req.result );
      } else {
	 obj_t res = bgl_uv_fstat( req.statbuf );

	 uv_fs_req_cleanup( &req );

	 return res;
      }
   }
}

/*---------------------------------------------------------------------*/
/*    int                                                              */
/*    bgl_uv_fs_link ...                                               */
/*---------------------------------------------------------------------*/
int
bgl_uv_fs_link( char *oldp, char *newp, obj_t proc, bgl_uv_loop_t bloop ) {
   BGL_UV_FS_WRAPPER1( uv_fs_link, oldp, newp )
}

/*---------------------------------------------------------------------*/
/*    int                                                              */
/*    bgl_uv_fs_symlink ...                                            */
/*---------------------------------------------------------------------*/
int
bgl_uv_fs_symlink( char *oldp, char *newp, obj_t proc, bgl_uv_loop_t bloop ) {
   BGL_UV_FS_WRAPPER1( uv_fs_link, oldp, newp )
}

/*---------------------------------------------------------------------*/
/*    static void                                                      */
/*    bgl_uv_fs_readlink_cb ...                                        */
/*---------------------------------------------------------------------*/
static void
bgl_uv_fs_readlink_cb( uv_fs_t *req ) {
   obj_t p = (obj_t)req->data;

   gc_unmark( p );

   if( req->result < 0 ) {
      PROCEDURE_ENTRY( p )( p, BINT( req->result ), BEOA );
   } else {
      PROCEDURE_ENTRY( p )( p, string_to_bstring( (char *)req->ptr ), BEOA );
   }

   uv_fs_req_cleanup( req );
   free( req );
}
   
/*---------------------------------------------------------------------*/
/*    obj_t                                                            */
/*    bgl_uv_fs_readlink ...                                           */
/*---------------------------------------------------------------------*/
obj_t
bgl_uv_fs_readlink( char *path, obj_t proc, bgl_uv_loop_t bloop ) {
   uv_loop_t *loop = (uv_loop_t *)bloop->BgL_z42builtinz42;

   if( bgl_check_fs_cb( proc, 1, "uv_fs_readlink" ) ) {
      uv_fs_t *req = (uv_fs_t *)malloc( sizeof( uv_fs_t ) );
      req->data = proc;
      gc_mark( proc );
      
      uv_fs_readlink( loop, req, path, &bgl_uv_fs_readlink_cb );
      
      return BUNSPEC;
   } else {
      uv_fs_t req;

      if( uv_fs_readlink( loop, &req, path, 0L ) < 0 ) {
	 uv_fs_req_cleanup( &req );
	 return BINT( req.result );
      } else {
	 obj_t res = string_to_bstring( (char *)req.ptr );

	 uv_fs_req_cleanup( &req );

	 return res;
      }
   }
}

/*---------------------------------------------------------------------*/
/*    int                                                              */
/*    bgl_uv_fs_unlink ...                                             */
/*---------------------------------------------------------------------*/
int
bgl_uv_fs_unlink( char *path, obj_t proc, bgl_uv_loop_t bloop ) {
   BGL_UV_FS_WRAPPER0( uv_fs_unlink, path )
}

/*---------------------------------------------------------------------*/
/*    int                                                              */
/*    bgl_uv_fs_rmdir ...                                              */
/*---------------------------------------------------------------------*/
int
bgl_uv_fs_rmdir( char *path, obj_t proc, bgl_uv_loop_t bloop ) {
   BGL_UV_FS_WRAPPER0( uv_fs_rmdir, path )
}

/*---------------------------------------------------------------------*/
/*    int                                                              */
/*    bgl_uv_fs_mkdir ...                                              */
/*---------------------------------------------------------------------*/
int
bgl_uv_fs_mkdir( char *path, int mod, obj_t proc, bgl_uv_loop_t bloop ) {
   BGL_UV_FS_WRAPPER1( uv_fs_mkdir, path, mod )
}

/*---------------------------------------------------------------------*/
/*    int                                                              */
/*    bgl_uv_fs_fsync ...                                              */
/*---------------------------------------------------------------------*/
int
bgl_uv_fs_fsync( obj_t port, obj_t proc, bgl_uv_loop_t bloop ) {
   int fd = ((bgl_uv_file_t)port)->BgL_fdz00;

   BGL_UV_FS_WRAPPER0( uv_fs_fsync, fd )
}

/*---------------------------------------------------------------------*/
/*    int                                                              */
/*    bgl_uv_fs_fdatasync ...                                          */
/*---------------------------------------------------------------------*/
int
bgl_uv_fs_fdatasync( obj_t port, obj_t proc, bgl_uv_loop_t bloop ) {
   int fd = ((bgl_uv_file_t)port)->BgL_fdz00;

   BGL_UV_FS_WRAPPER0( uv_fs_fdatasync, fd )
}

/*---------------------------------------------------------------------*/
/*    int                                                              */
/*    bgl_uv_fs_futime ...                                             */
/*---------------------------------------------------------------------*/
int
bgl_uv_fs_futime( obj_t port, double atime, double mtime, obj_t proc, bgl_uv_loop_t bloop ) {
   int fd = ((bgl_uv_file_t)port)->BgL_fdz00;

   BGL_UV_FS_WRAPPER2( uv_fs_futime, fd, atime, mtime )
}

/*---------------------------------------------------------------------*/
/*    int                                                              */
/*    bgl_uv_fs_utime ...                                             */
/*---------------------------------------------------------------------*/
int
bgl_uv_fs_utime( char *path, double atime, double mtime, obj_t proc, bgl_uv_loop_t bloop ) {
   BGL_UV_FS_WRAPPER2( uv_fs_utime, path, atime, mtime )
}

/*---------------------------------------------------------------------*/
/*    static void                                                      */
/*    bgl_uv_fs_readwrite_cb ...                                       */
/*---------------------------------------------------------------------*/
static void
bgl_uv_fs_readwrite_cb( uv_fs_t *req ) {
   obj_t p = (obj_t)req->data;

   gc_unmark( p );

   PROCEDURE_ENTRY( p )( p, BINT( req->result ), BEOA );
}

void debug_buf( char *fun, uv_buf_t *buf ) {
   int i;
   int len = buf->len;

   fprintf( stderr, "---- %s debug_buf (%s) len=%d ------\n", __FILE__, fun, len );
   for( i = 0; i < (len < 160 ? len : 160); i++ ) {
      fprintf( stderr, "%02x ", ((unsigned char *)(buf->base))[ i ] );
   }
   for( i = 0; i < (len < 160 ? len : 160); i++ ) {
      char c = ((unsigned char *)(buf->base))[ i ];
      fprintf( stderr, "%c ", isalpha( c ) ? c : '.' );
   }

   fprintf( stderr, "\n" );
}

/*---------------------------------------------------------------------*/
/*    int                                                              */
/*    bgl_uv_fs_write ...                                              */
/*---------------------------------------------------------------------*/
int
bgl_uv_fs_write( obj_t port, obj_t buffer, long offset, long length, long position, obj_t proc, bgl_uv_loop_t bloop ) {
   uv_loop_t *loop = (uv_loop_t *)bloop->BgL_z42builtinz42;
   bgl_uv_file_t file = (bgl_uv_file_t)port;
   uv_fs_t *req = file->BgL_z52readreqz52;
   uv_buf_t *buf = (uv_buf_t *)(&(file->BgL_z52rbufz52));
   int fd = file->BgL_fdz00;

   if( length + offset > STRING_LENGTH( buffer ) ) {
      C_SYSTEM_FAILURE( BGL_INDEX_OUT_OF_BOUND_ERROR, "uv-fs-write",
			"offset+length out of buffer range",
			BINT( STRING_LENGTH( buffer ) ) );
   }

   if( bgl_check_fs_cb( proc, 1, "uv_fs_write" ) ) {
      /* uv_buf_init inlined */
      file->BgL_z52rbufz52 = &(STRING_REF( buffer, offset ));
      file->BgL_z52rbuflenz52 = length;

/*    void* buf = (void *)&(STRING_REF( buffer, offset ));             */
/*    uv_buf_t iov;                                                    */
/*                                                                     */
/*    iov = uv_buf_init( buf, length );                                */

      req->data = proc;
      gc_mark( proc );

      uv_fs_write( loop, req, fd, buf, 1, position, &bgl_uv_fs_readwrite_cb );
      uv_fs_req_cleanup( req );
   } else {
      return pwrite( fd, &(STRING_REF( buffer, offset )), length, offset );
   }
}
      
/*---------------------------------------------------------------------*/
/*    int                                                              */
/*    bgl_uv_fs_read ...                                               */
/*---------------------------------------------------------------------*/
int
bgl_uv_fs_read( obj_t port, obj_t buffer, long offset, long length, long position, obj_t proc, bgl_uv_loop_t bloop ) {
   uv_loop_t *loop = (uv_loop_t *)bloop->BgL_z42builtinz42;
   bgl_uv_file_t file = (bgl_uv_file_t)port;
   uv_buf_t *buf = (uv_buf_t *)(&(file->BgL_z52rbufz52));
   int fd = file->BgL_fdz00;
   int len = 0;

   if( length + offset > STRING_LENGTH( buffer ) ) {
      C_SYSTEM_FAILURE( BGL_INDEX_OUT_OF_BOUND_ERROR, "uv-fs-read",
			"offset+length out of buffer range",
			BINT( len ) );
   }

   /* uv_buf_init inlined */
   file->BgL_z52rbufz52 = (void *)&(STRING_REF( buffer, offset ));
   file->BgL_z52rbuflenz52 = length;

/*    void* buf = (void *)&(STRING_REF( buffer, offset ));             */
/*    uv_buf_t iov;                                                    */
/*                                                                     */
/*    iov = uv_buf_init( buf, length );                                */

   if( bgl_check_fs_cb( proc, 1, "uv_fs_read" ) ) {
      uv_fs_t *req = file->BgL_z52readreqz52;
      
      req->data = proc;
      gc_mark( proc );

      uv_fs_read( loop, req, fd, buf, 1, position, &bgl_uv_fs_readwrite_cb );
   } else {
      uv_fs_t req;

      int r = uv_fs_read( loop, &req, fd, buf, 1, position, 0L );
      uv_fs_req_cleanup( &req );

      return r;
   }
}

/* {*---------------------------------------------------------------------*} */
/* {*    static int                                                       *} */
/* {*    bgl_uv_close_file ...                                            *} */
/* {*---------------------------------------------------------------------*} */
/* static int                                                          */
/* bgl_uv_close_file( int file ) {                                     */
/*    uv_fs_t *req = (uv_fs_t *)malloc( sizeof( uv_fs_t ) );           */
/*    uv_loop_t *loop = uv_default_loop();                             */
/*    int res = uv_fs_close( loop, req, (uv_file)file, 0L );           */
/*                                                                     */
/*    uv_fs_req_cleanup( req );                                        */
/*    free( req );                                                     */
/*    return res;                                                      */
/* }                                                                   */
/*                                                                     */
/* {*---------------------------------------------------------------------*} */
/* {*    static long                                                      *} */
/* {*    bgl_uv_sync_read ...                                             *} */
/* {*---------------------------------------------------------------------*} */
/* static long                                                         */
/* bgl_uv_sync_read( obj_t port, void *ptr, size_t num ) {             */
/*    int fd = (long)PORT_FILE( port );                                */
/*    long n;                                                          */
/*                                                                     */
/*  loop:                                                              */
/*    if( (n = read( fd, ptr, num ) ) <= 0 ) {                         */
/*       if( n == 0 ) {                                                */
/* 	 INPUT_PORT( port ).eof = 1;                                   */
/*       } else if( errno == EINTR ) {                                 */
/* 	 goto loop;                                                    */
/*       }                                                             */
/*    }                                                                */
/*                                                                     */
/*    return n;                                                        */
/* }                                                                   */
/*                                                                     */
/* {*---------------------------------------------------------------------*} */
/* {*    static void                                                      *} */
/* {*    bgl_uv_file_seek ...                                             *} */
/* {*---------------------------------------------------------------------*} */
/* static void                                                         */
/* bgl_uv_file_seek( obj_t port, long pos ) {                          */
/*    int fd = (long)PORT_FILE( port );                                */
/*                                                                     */
/*    if( lseek( fd, pos, SEEK_SET ) == -1 ) {                         */
/*       C_SYSTEM_FAILURE( BGL_IO_PORT_ERROR, "set-input-port-position!", */
/* 			strerror( errno ),                             */
/* 			port );                                        */
/*    }                                                                */
/*                                                                     */
/*    INPUT_PORT( port ).filepos = pos;                                */
/*    INPUT_PORT( port ).eof = 0;                                      */
/*    INPUT_PORT( port ).matchstart = 0;                               */
/*    INPUT_PORT( port ).matchstop = 0;                                */
/*    INPUT_PORT( port ).forward = 0;                                  */
/*    INPUT_PORT( port ).bufpos = 0;                                   */
/*    INPUT_PORT( port ).lastchar = '\n';                              */
/*                                                                     */
/*    RGC_BUFFER_SET( port, 0, '\0' );                                 */
/* }                                                                   */
/*                                                                     */
/* {*---------------------------------------------------------------------*} */
/* {*    static void                                                      *} */
/* {*    bgl_uv_fs_open_input_file_cb ...                                 *} */
/* {*---------------------------------------------------------------------*} */
/* static void                                                         */
/* bgl_uv_fs_open_input_file_cb( uv_fs_t* req ) {                      */
/*    obj_t proc = CAR( (obj_t)req->data );                            */
/*    obj_t buffer = CDR( (obj_t)req->data );                          */
/*    obj_t port;                                                      */
/*                                                                     */
/*    gc_unmark( req->data );                                          */
/*                                                                     */
/*    if( req->result == -1 ) {                                        */
/*       port = BINT( req->result );                                   */
/*    } else {                                                         */
/*       port = bgl_make_input_port(                                   */
/* 	 string_to_bstring( ( char*)(req->path) ), (FILE *)req->result, KINDOF_FILE, buffer ); */
/*                                                                     */
/*       INPUT_PORT( port ).port.userdata = GC_MALLOC( sizeof( uv_fs_t ) ); */
/*       INPUT_PORT( port ).sysread = &bgl_uv_sync_read;               */
/*       INPUT_PORT( port ).sysseek = &bgl_uv_file_seek;               */
/*       INPUT_PORT( port ).port.sysclose = &bgl_uv_close_file;        */
/*       BGL_INPUT_PORT_LENGTH_SET( port, bgl_file_size( (char *)req->path ) ); */
/*    }                                                                */
/*                                                                     */
/*    uv_fs_req_cleanup( req );                                        */
/*    free( req );                                                     */
/*                                                                     */
/*    PROCEDURE_ENTRY( proc )( proc, port, BEOA );                     */
/* }                                                                   */
/*                                                                     */
/* {*---------------------------------------------------------------------*} */
/* {*    obj_t                                                            *} */
/* {*    bgl_uv_open_input_file ...                                       *} */
/* {*---------------------------------------------------------------------*} */
/* obj_t                                                               */
/* bgl_uv_open_input_file( obj_t name, obj_t buffer, obj_t proc ) {    */
/*    uv_fs_t *req = (uv_fs_t *)malloc( sizeof( uv_fs_t ) );           */
/*    char *path = BSTRING_TO_STRING( name );                          */
/*    uv_loop_t *loop = uv_default_loop();                             */
/*    obj_t res;                                                       */
/*    int r;                                                           */
/*                                                                     */
/*    req->data = MAKE_PAIR( proc, buffer );                           */
/*    gc_mark( req->data );                                            */
/*                                                                     */
/*    if( r = uv_fs_open( loop, req, path, O_RDONLY, 0,                */
/* 		       PROCEDUREP( proc ) ? bgl_uv_fs_open_input_file_cb : 0L ) < 0 ) { */
/*       uv_fs_req_cleanup( req );                                     */
/*       free( req );                                                  */
/*       C_SYSTEM_FAILURE( BGL_IO_PORT_ERROR, "uv-open-input-file",    */
/* 			(char *)uv_strerror( req->result ),            */
/* 			name );                                        */
/*    } else {                                                         */
/*       obj_t port = bgl_make_input_port( name, (FILE *)req->result, KINDOF_FILE, buffer ); */
/*                                                                     */
/*       if( proc == BFALSE ) {                                        */
/* 	 uv_fs_req_cleanup( req );                                     */
/* 	 free( req );                                                  */
/*       }                                                             */
/*                                                                     */
/*       INPUT_PORT( port ).port.userdata = GC_MALLOC( sizeof( uv_fs_t ) ); */
/*       INPUT_PORT( port ).sysread = &bgl_uv_sync_read;               */
/*       INPUT_PORT( port ).sysseek = &bgl_uv_file_seek;               */
/*       INPUT_PORT( port ).port.sysclose = &bgl_uv_close_file;        */
/*                                                                     */
/*       return port;                                                  */
/*    }                                                                */
/* }                                                                   */

/*---------------------------------------------------------------------*/
/*    MAX_IP_LEN ...                                                   */
/*---------------------------------------------------------------------*/
#define MAX_IP_LEN \
      (INET6_ADDRSTRLEN > INET_ADDRSTRLEN ? INET6_ADDRSTRLEN : INET_ADDRSTRLEN)

/*---------------------------------------------------------------------*/
/*    static int                                                       */
/*    itof ...                                                         */
/*---------------------------------------------------------------------*/
static int
itof( int family ) {
   switch( family ) {
      case 6:
	 return AF_INET6;
	 break;

      case 4:
	 return AF_INET;

      default:
	 return AF_UNSPEC;
   }
}

/*---------------------------------------------------------------------*/
/*    static void                                                      */
/*    bgl_uv_getaddrinfo_cb ...                                        */
/*---------------------------------------------------------------------*/
static void
bgl_uv_getaddrinfo_cb( uv_getaddrinfo_t *req, int status, struct addrinfo *res ) {
   obj_t p = (obj_t)req->data;
   gc_unmark( p );

   if( status ) {
      PROCEDURE_ENTRY( p )( p, BINT( status ), BEOA );
   } else {
      char *addr;
      obj_t acc = BNIL;
      
      char ip[ MAX_IP_LEN ];
      
      for( ; res; res = res->ai_next ) {
	 switch( res->ai_family ) {
	    case AF_INET: {
	       // ipv4 address
	       addr = (char *)&((struct sockaddr_in *)res->ai_addr)->sin_addr;
	       int err = uv_inet_ntop( res->ai_family, addr,
				       ip, INET_ADDRSTRLEN );
	       if( err != 0 ) {
		  continue;
	       } else {
		  acc = MAKE_PAIR( string_to_bstring( ip ), acc );
	       }
	    }
	       break;
	       
	    case AF_INET6: {
	       // ipv6 address
	       addr = (char*)&((struct sockaddr_in6 *)res->ai_addr)->sin6_addr;
	       int err = uv_inet_ntop( res->ai_family, addr,
				       ip, INET6_ADDRSTRLEN );
	       if( err != 0 ) {
		  continue;
	       } else {
		  acc = MAKE_PAIR( string_to_bstring( ip ), acc );
	       }
	    }
	       break;
	 }
      }

      uv_freeaddrinfo( res );

      PROCEDURE_ENTRY( p )( p, bgl_reverse_bang( acc ), BEOA );
   }
}

/*---------------------------------------------------------------------*/
/*    int                                                              */
/*    bgl_uv_getaddrinfo ...                                           */
/*---------------------------------------------------------------------*/
int
bgl_uv_getaddrinfo( char *node, char *service, int family, obj_t proc, bgl_uv_loop_t bloop ) {
   if( !(PROCEDUREP( proc ) && (PROCEDURE_CORRECT_ARITYP( proc, 1 )) ) ) {
      C_SYSTEM_FAILURE( BGL_TYPE_ERROR, "uv-getaddrinfo",
			"wrong callback", proc );
   } else {
      uv_loop_t *loop = (uv_loop_t *)bloop->BgL_z42builtinz42;
      uv_getaddrinfo_t *resolver =
	 (uv_getaddrinfo_t *)malloc( sizeof( uv_getaddrinfo_t ) );
      struct addrinfo hints;
      int fam = itof( family );
      int r;

      resolver->data = proc;

      memset( &hints, 0, sizeof( struct addrinfo ) );
      hints.ai_family = fam;
      hints.ai_socktype = SOCK_STREAM;
   
      if( (r = uv_getaddrinfo( loop, resolver, bgl_uv_getaddrinfo_cb,
			       node, service, &hints )) < 0 ) {
	 free( resolver );
      } else {
	 gc_mark( proc );
      }

      return r;
   }
}

/*---------------------------------------------------------------------*/
/*    obj_t                                                            */
/*    bgl_uv_inet_pton ...                                             */
/*---------------------------------------------------------------------*/
obj_t
bgl_uv_inet_pton( char *addr, int family ) {
   char buf[ MAX_IP_LEN ];
   int fam = itof( family );
   int res = uv_inet_pton( fam, addr, &buf );

   if( res == 0 ) {
      return string_to_bstring( buf );
   } else {
      return BFALSE;
   }
}

/*---------------------------------------------------------------------*/
/*    uv_tcp_t *                                                       */
/*    bgl_uv_tcp_create ...                                            */
/*---------------------------------------------------------------------*/
uv_tcp_t *
bgl_uv_tcp_create( uv_loop_t *loop, obj_t obj ) {
   uv_tcp_t *tcp = (uv_tcp_t *)GC_MALLOC( sizeof( uv_tcp_t ) );

   uv_tcp_init( loop, tcp );

   tcp->data = obj;
   return tcp;
}

/*---------------------------------------------------------------------*/
/*    static void                                                      */
/*    uv_tcp_connect_cb ...                                            */
/*---------------------------------------------------------------------*/
static void
uv_tcp_connect_cb( uv_connect_t *req, int status ) {
   obj_t p = (obj_t)req->data;
   obj_t handle = req->handle->data;

   gc_unmark( p );

   free( req );
   
   PROCEDURE_ENTRY( p )( p, BINT( status ), handle, BEOA );
}

/*---------------------------------------------------------------------*/
/*    static int                                                       */
/*    bgl_uv_tcp_connect ...                                           */
/*---------------------------------------------------------------------*/
static int
bgl_uv_tcp_connectX( obj_t obj, struct sockaddr *address, obj_t proc, bgl_uv_loop_t bloop ) {
   if( !(PROCEDUREP( proc ) && (PROCEDURE_CORRECT_ARITYP( proc, 2 )) ) ) {
      C_SYSTEM_FAILURE( BGL_TYPE_ERROR, "uv-tcp-connect",
			"wrong callback", proc );
   } else {
      uv_connect_t *req = malloc( sizeof( uv_connect_t ) );
      ((bgl_uv_stream_t)obj)->BgL_z52writereqz52 =
	 GC_MALLOC( sizeof( uv_write_t ) );
      uv_tcp_t *handle =
	 (uv_tcp_t *)(((bgl_uv_handle_t)obj)->BgL_z42builtinz42);
      int r;

      req->data = proc;
      
      gc_mark( proc );

      r = uv_tcp_connect( req, handle, address, uv_tcp_connect_cb );

      if( r != 0 ) {
	 free( req );
      }

      return r;
   }
}

/*---------------------------------------------------------------------*/
/*    int                                                              */
/*    bgl_uv_tcp_connect ...                                           */
/*---------------------------------------------------------------------*/
int
bgl_uv_tcp_connect( obj_t obj, char *addr, int port, int family, obj_t proc, bgl_uv_loop_t bloop ) {
   union addr {
      struct sockaddr_in ip4;
      struct sockaddr_in6 ip6;
   } address;
   int r;
      
   if( family == 4 ) {
      r = uv_ip4_addr( addr, port, &(address.ip4) );
   } else {
      r = uv_ip6_addr( addr, port, &(address.ip6) );
   }

   if( r ) {
      return r;
   }

   return bgl_uv_tcp_connectX( obj, (struct sockaddr *)&address, proc, bloop );
}

/*---------------------------------------------------------------------*/
/*    static void                                                      */
/*    uv_listen_cb ...                                                 */
/*---------------------------------------------------------------------*/
static void
uv_listen_cb( uv_stream_t *handle, int status ) {
   obj_t data = (obj_t)handle->data;
   obj_t p, obj;

   gc_unmark( data );

   obj = CAR( data );
   p = CDR( data );

   PROCEDURE_ENTRY( p )( p, obj, BINT( status ), BEOA );
}

/*---------------------------------------------------------------------*/
/*    int                                                              */
/*    bgl_uv_listen ...                                                */
/*---------------------------------------------------------------------*/
int
bgl_uv_listen( obj_t obj, int backlog, obj_t proc, bgl_uv_loop_t bloop ) {
   if( !(PROCEDUREP( proc ) && (PROCEDURE_CORRECT_ARITYP( proc, 2 )) ) ) {
      C_SYSTEM_FAILURE( BGL_TYPE_ERROR, "uv-tcp-listen",
			"wrong callback", proc );
   } else {
      bgl_uv_stream_t stream = (bgl_uv_stream_t)obj;
      uv_stream_t *s = (uv_stream_t *)(stream->BgL_z42builtinz42);

      s->data = MAKE_PAIR( obj, proc );
      gc_mark( s->data );

      return uv_listen( s, backlog, uv_listen_cb );
   }
}

/*---------------------------------------------------------------------*/
/*    int                                                              */
/*    bgl_uv_tcp_bind ...                                              */
/*---------------------------------------------------------------------*/
int
bgl_uv_tcp_bind( uv_tcp_t *handle, char *addr, int port, int family ) {
   union addr {
      struct sockaddr_in ip4;
      struct sockaddr_in6 ip6;
   } address;
   int r;
      
   if( family == 4 ) {
      r = uv_ip4_addr( addr, port, &(address.ip4) );
   } else {
      r = uv_ip6_addr( addr, port, &(address.ip6) );
   }

   if( r ) {
      return r;
   }

   return uv_tcp_bind( handle, (struct sockaddr *)&address, 0 );
}

/*---------------------------------------------------------------------*/
/*    int                                                              */
/*    bgl_uv_tcp_bind6 ...                                             */
/*---------------------------------------------------------------------*/
int
bgl_uv_tcp_bind6( uv_tcp_t *handle, char *addr, int port ) {
   struct sockaddr_in6 address;
   int r = uv_ip6_addr( addr, port, &address );

   if( r ) {
      return r;
   }

   return uv_tcp_bind( handle, (struct sockaddr *)&address, UV_TCP_IPV6ONLY );
}

/*---------------------------------------------------------------------*/
/*    address symbols                                                  */
/*---------------------------------------------------------------------*/
static obj_t _address = BUNSPEC, _family, _port, _IPv4, _IPv6;

/*---------------------------------------------------------------------*/
/*    static void                                                      */
/*    uv_init_address ...                                              */
/*---------------------------------------------------------------------*/
static void
uv_init_address() {
   if( _address == BUNSPEC ) {
      _address = string_to_symbol( "address" );
      _family = string_to_symbol( "family" );
      _port = string_to_symbol( "port" );
      _IPv4 = string_to_symbol( "IPv4" );
      _IPv6 = string_to_symbol( "IPv6" );
   }
}
   
/*---------------------------------------------------------------------*/
/*    static obj_t                                                     */
/*    bgl_address ...                                                  */
/*---------------------------------------------------------------------*/
static obj_t
bgl_address( struct sockaddr *addr ) {
   obj_t res = BNIL;
   char ip[ MAX_IP_LEN ];

   uv_init_address();
   
   switch( addr->sa_family ) {
      case AF_INET6: {
	 struct sockaddr_in6 *a6 = (struct sockaddr_in6 *)addr;
	 uv_inet_ntop( AF_INET6, &a6->sin6_addr, ip, sizeof ip );
	 
	 res = MAKE_PAIR( MAKE_PAIR( _port, BINT( ntohs( a6->sin6_port ) ) ),
			  res );
	 res = MAKE_PAIR( MAKE_PAIR( _family, _IPv4 ), res );
	 res = MAKE_PAIR( MAKE_PAIR( _address, string_to_bstring( ip ) ), res );
	 break;
      }

      case AF_INET: {
	 struct sockaddr_in *a4 = (struct sockaddr_in *)addr;
	 uv_inet_ntop( AF_INET, &a4->sin_addr, ip, sizeof ip );
	 
	 res = MAKE_PAIR( MAKE_PAIR( _port, BINT( ntohs( a4->sin_port ) ) ), res );
	 res = MAKE_PAIR( MAKE_PAIR( _family, _IPv4 ), res );
	 res = MAKE_PAIR( MAKE_PAIR( _address, string_to_bstring( ip ) ), res );

	 break;
      }
   }
	 
   return res;
}

/*---------------------------------------------------------------------*/
/*    obj_t                                                            */
/*    bgl_uv_tcp_getsockname ...                                       */
/*---------------------------------------------------------------------*/
obj_t
bgl_uv_tcp_getsockname( uv_tcp_t *handle ) {
   struct sockaddr_storage address;
   int addrlen = sizeof( address );
   int r = uv_tcp_getsockname( handle, (struct sockaddr *)&address, &addrlen );

   if( r ) { 
      return BINT( r );
   } else {
      return bgl_address( (struct sockaddr *)&address );
   }
}

/*---------------------------------------------------------------------*/
/*    obj_t                                                            */
/*    bgl_uv_tcp_getpeername ...                                       */
/*---------------------------------------------------------------------*/
obj_t
bgl_uv_tcp_getpeername( uv_tcp_t *handle ) {
   struct sockaddr_storage address;
   int addrlen = sizeof( address );
   int r = uv_tcp_getpeername( handle, (struct sockaddr *)&address, &addrlen );

   if( r ) { 
      return BINT( r );
   } else {
      return bgl_address( (struct sockaddr *)&address );
   }
}

/*---------------------------------------------------------------------*/
/*    static void                                                      */
/*    bgl_uv_write_cb ...                                              */
/*---------------------------------------------------------------------*/
static void
bgl_uv_write_cb( uv_write_t *req, int status ) {
   obj_t p = (obj_t)req->data;

   gc_unmark( p );
   PROCEDURE_ENTRY( p )( p, BINT( status ), BEOA );
}

/*---------------------------------------------------------------------*/
/*    int                                                              */
/*    bgl_uv_write ...                                                 */
/*---------------------------------------------------------------------*/
int
bgl_uv_write( obj_t obj, char *buffer, long offset, long length, obj_t proc, bgl_uv_loop_t bloop ) {
   if( !(PROCEDUREP( proc ) && (PROCEDURE_CORRECT_ARITYP( proc, 1 )) ) ) {
      C_SYSTEM_FAILURE( BGL_TYPE_ERROR, "uv-stream-write",
			"wrong callback", proc );
   } else {
      uv_loop_t *loop = (uv_loop_t *)bloop->BgL_z42builtinz42;
      bgl_uv_stream_t stream = (bgl_uv_stream_t)obj;
      uv_write_t *req = (uv_write_t *)(stream->BgL_z52writereqz52);
      uv_stream_t *handle = (uv_stream_t *)(stream->BgL_z42builtinz42);
      uv_buf_t *buf = (uv_buf_t *)(&(stream->BgL_z52wbufz52));
      int r;

      req->data = proc;
      
      gc_mark( proc );
      
      stream->BgL_z52wbufz52 = buffer + offset;
      stream->BgL_z52wbuflenz52 = length;
      
      r = uv_write( req, handle, buf, 1, bgl_uv_write_cb );

      if( r ) {
	 fprintf( stderr, "ERR: %d %s\n", r, uv_strerror( r ) );
      }
      
      return r;
   }
}
   
/*---------------------------------------------------------------------*/
/*    static void                                                      */
/*    bgl_uv_read_cb ...                                               */
/*---------------------------------------------------------------------*/
static void
bgl_uv_read_cb( uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf ) {
   obj_t obj = (obj_t)stream->data;
   bgl_uv_stream_t sobj = (bgl_uv_stream_t)obj;
   obj_t p = sobj->BgL_z52proccz52;
   obj_t allocobj = sobj->BgL_z52allocz52;
   obj_t offset = sobj->BgL_z52offsetz52;

   sobj->BgL_z52allocz52 = BUNSPEC;
   fprintf( stderr, "OnRead (%s) nread=%d\n", __FILE__, nread );
   gc_unmark( obj );

   if( PROCEDUREP( p ) ) {
      if( nread > 0 ) {
/* 	 fprintf( stderr, "bgl_uv_read_start, read_cb (%s) offset=%d read=%d\n", */
/* 		  __FILE__, CINT( offset ), nread );                   */
	 PROCEDURE_ENTRY( p )( p, allocobj, offset, BINT( nread ), BEOA );
      } else if( nread < 0 ) {
	 PROCEDURE_ENTRY( p )( p, BINT( nread ), BINT( -1 ), BINT( -1 ), BEOA );
      }
   }
}

/*---------------------------------------------------------------------*/
/*    static void                                                      */
/*    bgl_uv_alloc_cb ...                                              */
/*---------------------------------------------------------------------*/
static void
bgl_uv_alloc_cb( uv_handle_t *hdl, size_t ssize, uv_buf_t *buf ) {
   bgl_uv_stream_t stream = (bgl_uv_stream_t)hdl->data;
   obj_t p = stream->BgL_z52procaz52;
   obj_t allocobj = PROCEDURE_ENTRY( p )( p, stream, BINT( ssize ) );
   obj_t chunk = BGL_MVALUES_VAL( 1 );
   obj_t offset = BGL_MVALUES_VAL( 2 );

   fprintf( stderr, "bgl_uv_read_start, alloc_cb (%s) offset=%d ssize=%d\n",
	    __FILE__, CINT( offset ), ssize );
   if( !STRINGP( chunk ) ) {
      C_SYSTEM_FAILURE( BGL_TYPE_ERROR, "uv-read-start, onalloc",
			"string",
			chunk );
   }
   stream->BgL_z52allocz52 = allocobj;
   stream->BgL_z52offsetz52 = offset;

   *buf = uv_buf_init( &(STRING_REF( chunk, CINT( offset ) )), ssize );
}

/*---------------------------------------------------------------------*/
/*    int                                                              */
/*    bgl_uv_read_start ...                                            */
/*---------------------------------------------------------------------*/
int
bgl_uv_read_start( obj_t obj, obj_t proca, obj_t procc, bgl_uv_loop_t bloop ) {
   fprintf( stderr, "bgl_uv_read_start...\n" );
   if( !PROCEDUREP( proca ) || (!PROCEDURE_CORRECT_ARITYP( proca, 2 )) ) {
      C_SYSTEM_FAILURE( BGL_TYPE_ERROR, "uv-read-start",
			"wrong onalloc", proca );
   } else {
      if( !(PROCEDUREP( procc ) && (PROCEDURE_CORRECT_ARITYP( procc, 3 )) ) ) {
	 C_SYSTEM_FAILURE( BGL_TYPE_ERROR, "uv-read-start",
			   "wrong callback", procc );
      } else {
	 bgl_uv_stream_t stream = (bgl_uv_stream_t)obj;
	 uv_stream_t *s = (uv_stream_t *)(stream->BgL_z42builtinz42);
	 int r;

	 stream->BgL_z52allocz52 = BUNSPEC;
	 stream->BgL_z52procaz52 = proca;
	 stream->BgL_z52proccz52 = procc;

	 gc_mark( obj );

	 r = uv_read_start( s, bgl_uv_alloc_cb, bgl_uv_read_cb );

	 fprintf( stderr, "bgl_uv_read_start r=%d\n", r );
	 return r;
      }
   }
}

/*---------------------------------------------------------------------*/
/*    static void                                                      */
/*    bgl_uv_shutdown_cb ...                                           */
/*---------------------------------------------------------------------*/
static void
bgl_uv_shutdown_cb( uv_shutdown_t* req, int status ) {
   obj_t p = (obj_t)req->data;
   obj_t handle = req->handle->data;

   gc_unmark( p );

   free( req );

   PROCEDURE_ENTRY( p )( p, BINT( status ), handle, BEOA );
}
   
/*---------------------------------------------------------------------*/
/*    int                                                              */
/*    bgl_uv_shutdown ...                                              */
/*---------------------------------------------------------------------*/
int
bgl_uv_shutdown( obj_t obj, obj_t proc, bgl_uv_loop_t bloop ) {
   if( !(PROCEDUREP( proc ) && (PROCEDURE_CORRECT_ARITYP( proc, 2 )) ) ) {
      C_SYSTEM_FAILURE( BGL_TYPE_ERROR, "uv-shutdown",
			"wrong callback", proc );
   } else {
      bgl_uv_stream_t stream = (bgl_uv_stream_t)obj;
      uv_stream_t *s = (uv_stream_t *)(stream->BgL_z42builtinz42);
      uv_shutdown_t *req = malloc( sizeof( uv_shutdown_t ) );
      int r;

      gc_mark( obj );

      req->data = proc;

      if( r = uv_shutdown( req, s, bgl_uv_shutdown_cb ) ) {
	 free( req );
      }

      return r;
   }
}
