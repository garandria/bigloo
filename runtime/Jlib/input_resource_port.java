package bigloo;

import java.io.*;
import java.util.*;
import java.net.*;

public class input_resource_port extends input_port {
   public InputStream in;

   public input_resource_port( String resource, final byte[] buf )
      throws IOException {
      super( resource, buf );
      in = java.lang.ClassLoader.getSystemResourceAsStream( name.replace( '\\', '/' ) );
   }

   public static boolean exists( String name ) {
      return java.lang.ClassLoader.getSystemResource( name.replace( '\\', '/' ) ) != null;
   }

   public static int file_size( String name ) {
      try {
	 InputStream in = java.lang.ClassLoader.getSystemResourceAsStream( name.replace( '\\', '/' ) );
	 int sz = in.available();
	 in.close();
	 return sz;
      } catch( Exception _ ) {
	 return -1;
      }
   }

   public static byte[] readline( InputStream in ) throws Exception {
      int c = in.read();

      if( c == -1 ) {
	 return null;
      } else {
	 byte[] buf = new byte[ 255 ];
	 int l = 1;
	 buf[ 0 ] = (byte)c;

	 while( true ) {
	    c = in.read();

	    if( (c == '\n') || (c == -1) ) {
	       byte[] res = new byte[ l ];
	       System.arraycopy( buf, 0, res, 0, l );
	       return res;
	    }

	    buf[ l++ ] = (byte)c;
	 }
      }
   }
      
   public static Object directory_to_list( String name ) {
      try {
	 String dir = name.replace( '\\', '/' );
	 String cname = dir + "/.list";
	 
	 if( input_resource_port.exists( cname ) ) {
	    InputStream in;
	    Object res = bigloo.foreign.BNIL;
	    in = java.lang.ClassLoader.getSystemResourceAsStream( cname );
	    byte[] o;

	    while( (o = readline( in )) != null ) {
	       res = new pair( o, res );
	    }

	    in.close();

	    return res;
	 } else {
	    return bigloo.foreign.BNIL;
	 }
      } catch( Exception _ ) {
	 return bigloo.foreign.BNIL;
      }
   }

   public static boolean directoryp( String name ) {
      String dir = name.replace( '\\', '/' );
      String cname = dir + "/.list";

      return input_resource_port.exists( cname );
   }
   
   public void close() {
      eof = true;
      other_eof = true;
      try {
	 in.close();
      } catch( Throwable _ ) {
	 ;
      }
      super.close();
   }

   public boolean rgc_charready() {
      if (eof || other_eof)
	 return false;
      else
	 return true;
   }

   public boolean rgc_fill_buffer() throws IOException {
      final int bufsize = this.bufsiz;
      int bufpose = this.bufpos;
      final int matchstart = this.matchstart;
      final byte[] buffer = this.buffer;

      if (0 < matchstart) {
	 // we shift the buffer left and we fill the buffer */
	 final int movesize = bufpose-matchstart;

	 for ( int i= 0 ; i < movesize ; ++i )
	    buffer[i] = buffer[matchstart+i];

	 bufpose -= matchstart;
	 this.matchstart = 0;
	 this.matchstop -= matchstart;
	 this.forward -= matchstart;
	 this.lastchar = buffer[matchstart-1];

	 return rgc_size_fill_resource_buffer( bufpose, bufsize-bufpose );
      }

      if (bufpose < bufsize)
	 return rgc_size_fill_resource_buffer( bufpose, bufsize-bufpose );

      // we current token is too large for the buffer */
      // we have to enlarge it.                       */
      rgc_double_buffer();

      return rgc_fill_buffer();
   }

   final boolean rgc_size_fill_resource_buffer( int bufpose, final int  size )
      throws IOException {
      final int nbread = in.read( buffer, bufpose-1, size );

      if (nbread == -1)
	 eof = true;
      else
	 bufpose += nbread;

      this.bufpos = bufpose;

      if (0 < bufpose) {
	 buffer[bufpose-1] = 0;
	 return true;
      }

      return false;
   }

   Object bgl_input_port_seek( final int  pos ) throws IOException {
      return bigloo.foreign.BFALSE;
   }

   Object bgl_input_port_reopen() throws IOException {
      in.close();

      in = java.lang.ClassLoader.getSystemClassLoader().getResourceAsStream( name );

      filepos = 0;
      eof = false;
      matchstart = 0;
      matchstop = 0;
      forward = 0;
      bufpos = 1;
      lastchar = (byte)'\n';
      buffer[0] = 0;

      return bigloo.foreign.BTRUE;
   }

   public void write( final output_port  p ) {
      p.write( "#<input_resource_port:" + name + ">" );
   }
}
