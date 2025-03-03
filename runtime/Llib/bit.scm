;*=====================================================================*/
;*    serrano/prgm/project/bigloo/bigloo/runtime/Llib/bit.scm          */
;*    -------------------------------------------------------------    */
;*    Author      :  Manuel Serrano                                    */
;*    Creation    :  Mon Mar 27 11:06:41 1995                          */
;*    Last change :  Thu Nov  4 18:55:03 2021 (serrano)                */
;*    -------------------------------------------------------------    */
;*    Bit management                                                   */
;*=====================================================================*/

;*---------------------------------------------------------------------*/
;*    The module                                                       */
;*---------------------------------------------------------------------*/
(module __bit
   
   (import  __error)
   
   (use     __type
	    __bigloo
	    __tvector
	    __bignum
	    
	    __r4_numbers_6_5_fixnum
	    __r4_numbers_6_5_flonum
	    __r4_equivalence_6_2
	    __r4_characters_6_6
	    __r4_vectors_6_8
	    __r4_booleans_6_1
	    __r4_pairs_and_lists_6_3
	    __r4_symbols_6_4
	    __r4_strings_6_7
	    
	    __evenv)
   
   (extern  (infix macro c-bitor::long (::long ::long) " | ")
	    (infix macro c-bitorelong::elong (::elong ::elong) " | ")
	    (infix macro c-bitorllong::llong (::llong ::llong) " | ")
	    (infix macro $bitors8::int8 (::int8 ::int8) " | ")
	    (infix macro $bitoru8::uint8 (::uint8 ::uint8) " | ")
	    (infix macro $bitors16::int16 (::int16 ::int16) " | ")
	    (infix macro $bitoru16::uint16 (::uint16 ::uint16) " | ")
	    (infix macro $bitors32::int32 (::int32 ::int32) " | ")
	    (infix macro $bitoru32::uint32 (::uint32 ::uint32) " | ")
	    (infix macro $bitors64::int64 (::int64 ::int64) " | ")
	    (infix macro $bitoru64::uint64 (::uint64 ::uint64) " | ")
	    
	    (infix macro c-bitand::long (::long ::long) " & ")
	    (infix macro c-bitandelong::elong (::elong ::elong) " & ")
	    (infix macro c-bitandllong::llong (::llong ::llong) " & ")
	    (infix macro $bitands8::int8 (::int8 ::int8) " & ")
	    (infix macro $bitandu8::uint8 (::uint8 ::uint8) " & ")
	    (infix macro $bitands16::int16 (::int16 ::int16) " & ")
	    (infix macro $bitandu16::uint16 (::uint16 ::uint16) " & ")
	    (infix macro $bitands32::int32 (::int32 ::int32) " & ")
	    (infix macro $bitandu32::uint32 (::uint32 ::uint32) " & ")
	    (infix macro $bitands64::int64 (::int64 ::int64) " & ")
	    (infix macro $bitandu64::uint64 (::uint64 ::uint64) " & ")
	    
	    (infix macro c-bitxor::long (::long ::long) " ^ ")
	    (infix macro c-bitxorelong::elong (::elong ::elong) " ^ ")
	    (infix macro c-bitxorllong::llong (::llong ::llong) " ^ ")
	    (infix macro $bitxors8::int8 (::int8 ::int8) " ^ ")
	    (infix macro $bitxoru8::uint8 (::uint8 ::uint8) " ^ ")
	    (infix macro $bitxors16::int16 (::int16 ::int16) " ^ ")
	    (infix macro $bitxoru16::uint16 (::uint16 ::uint16) " ^ ")
	    (infix macro $bitxors32::int32 (::int32 ::int32) " ^ ")
	    (infix macro $bitxoru32::uint32 (::uint32 ::uint32) " ^ ")
	    (infix macro $bitxors64::int64 (::int64 ::int64) " ^ ")
	    (infix macro $bitxoru64::uint64 (::uint64 ::uint64) " ^ ")
	    
	    (macro c-bitnot::long (::long) "~")
	    (macro c-bitnotelong::elong (::elong) "~")
	    (macro c-bitnotllong::llong (::llong) "~")
	    (macro $bitnots8::int8 (::int8) "~")
	    (macro $bitnotu8::uint8 (::uint8) "~")
	    (macro $bitnots16::int16 (::int16) "~")
	    (macro $bitnotu16::uint16 (::uint16) "~")
	    (macro $bitnots32::int32 (::int32) "~")
	    (macro $bitnotu32::uint32 (::uint32) "~")
	    (macro $bitnots64::int64 (::int64) "~")
	    (macro $bitnotu64::uint64 (::uint64) "~")
	    
	    (infix macro c-bitrsh::long (::long ::int) " >> ")
	    (infix macro c-bitrshelong::elong (::elong ::int) " >> ")
	    (infix macro c-bitrshllong::llong (::llong ::int) " >> ")
	    (infix macro $bitrshs8::int8 (::int8 ::int) " >> ")
	    (infix macro $bitrshu8::uint8 (::uint8 ::int) " >> ")
	    (infix macro $bitrshs16::int16 (::int16 ::int) " >> ")
	    (infix macro $bitrshu16::uint16 (::uint16 ::int) " >> ")
	    (infix macro $bitrshs32::int32 (::int32 ::int) " >> ")
	    (infix macro $bitrshu32::uint32 (::uint32 ::int) " >> ")
	    (infix macro $bitrshs64::int64 (::int64 ::int) " >> ")
	    (infix macro $bitrshu64::uint64 (::uint64 ::int) " >> ")
	    
	    (infix macro c-bitursh::ulong (::ulong ::int) " >> ")
	    (infix macro c-biturshelong::uelong (::uelong ::int) " >> ")
	    (infix macro c-biturshllong::ullong (::ullong ::int) " >> ")
	    (infix macro $biturshs8::int8 (::int8 ::int) " >> ")
	    (infix macro $biturshu8::uint8 (::uint8 ::int) " >> ")
	    (infix macro $biturshu16::uint16 (::uint16 ::int) " >> ")
	    (infix macro $biturshs16::int16 (::int16 ::int) " >> ")
	    (infix macro $biturshs32::int32 (::int32 ::int) " >> ")
	    (infix macro $biturshu32::uint32 (::uint32 ::int) " >> ")
	    (infix macro $biturshs64::int64 (::int64 ::int) " >> ")
	    (infix macro $biturshu64::uint64 (::uint64 ::int) " >> ")
	    
	    (infix macro c-bitlsh::long (::long ::int) " << ")
	    (infix macro c-bitlshelong::elong (::elong ::int) " << ")
	    (infix macro c-bitlshllong::llong (::llong ::int) " << ")
	    (infix macro $bitlshs8::int8 (::int8 ::int) " << ")
	    (infix macro $bitlshu8::uint8 (::uint8 ::int) " << ")
	    (infix macro $bitlshs16::int16 (::int16 ::int) " << ")
	    (infix macro $bitlshu16::uint16 (::uint16 ::int) " << ")
	    (infix macro $bitlshs32::int32 (::int32 ::int) " << ")
	    (infix macro $bitlshu32::uint32 (::uint32 ::int) " << ")
	    (infix macro $bitlshs64::int64 (::int64 ::int) " << ")
	    (infix macro $bitlshu64::uint64 (::uint64 ::int) " << ")
	    )
   
   (java    (class foreign
	       (method static c-bitor::long (::long ::long)
		  "BITOR")
	       (method static c-bitorelong::elong (::elong ::elong)
		  "BITORELONG")
	       (method static c-bitorllong::llong (::llong ::llong)
		  "BITORLLONG")
	       (method static $bitors8::int8 (::int8 ::int8)
		  "BITORINT8")
	       (method static $bitoru8::uint8 (::uint8 ::uint8)
		  "BITORINT8")
	       (method static $bitors16::int16 (::int16 ::int16)
		  "BITORINT16")
	       (method static $bitoru16::uint16 (::uint16 ::uint16)
		  "BITORINT16")
	       (method static $bitors32::int32 (::int32 ::int32)
		  "BITORINT32")
	       (method static $bitoru32::uint32 (::uint32 ::uint32)
		  "BITORINT32")
	       (method static $bitors64::int64 (::int64 ::int64)
		  "BITORINT64")
	       (method static $bitoru64::uint64 (::uint64 ::uint64)
		  "BITORINT64")
	       
	       (method static c-bitand::long (::long ::long)
		  "BITAND")
	       (method static c-bitandelong::elong (::elong ::elong)
		  "BITANDELONG")
	       (method static c-bitandllong::llong (::llong ::llong)
		  "BITANDLLONG")
	       (method static $bitands8::int8 (::int8 ::int8)
		  "BITANDINT8")
	       (method static $bitandu8::uint8 (::uint8 ::uint8)
		  "BITANDINT8")
	       (method static $bitands16::int16 (::int16 ::int16)
		  "BITANDINT16")
	       (method static $bitandu16::uint16 (::uint16 ::uint16)
		  "BITANDINT16")
	       (method static $bitands32::int32 (::int32 ::int32)
		  "BITANDINT32")
	       (method static $bitandu32::uint32 (::uint32 ::uint32)
		  "BITANDINT32")
	       (method static $bitands64::int64 (::int64 ::int64)
		  "BITANDINT64")
	       (method static $bitandu64::uint64 (::uint64 ::uint64)
		  "BITANDINT64")
	       
	       (method static c-bitxor::long (::long ::long)
		  "BITXOR")
	       (method static c-bitxorelong::elong (::elong ::elong)
		  "BITXORELONG")
	       (method static c-bitxorllong::llong (::llong ::llong)
		  "BITXORLLONG")
	       (method static $bitxors8::int8 (::int8 ::int8)
		  "BITXORINT8")
	       (method static $bitxoru8::uint8 (::uint8 ::uint8)
		  "BITXORINT8")
	       (method static $bitxors16::int16 (::int16 ::int16)
		  "BITXORINT16")
	       (method static $bitxoru16::uint16 (::uint16 ::uint16)
		  "BITXORINT16")
	       (method static $bitxors32::int32 (::int32 ::int32)
		  "BITXORINT32")
	       (method static $bitxoru32::uint32 (::uint32 ::uint32)
		  "BITXORINT32")
	       (method static $bitxors64::int64 (::int64 ::int64)
		  "BITXORINT64")
	       (method static $bitxoru64::uint64 (::uint64 ::uint64)
		  "BITXORINT64")
	       
	       (method static c-bitnot::long (::long)
		  "BITNOT")
	       (method static c-bitnotelong::elong (::elong)
		  "BITNOTELONG")
	       (method static c-bitnotllong::llong (::llong)
		  "BITNOTLLONG")
	       (method static $bitnots8::int8 (::int8)
		  "BITNOTINT8")
	       (method static $bitnotu8::uint8 (::uint8)
		  "BITNOTINT8")
	       (method static $bitnots16::int16 (::int16)
		  "BITNOTINT16")
	       (method static $bitnotu16::uint16 (::uint16)
		  "BITNOTINT16")
	       (method static $bitnots32::int32 (::int32)
		  "BITNOTINT32")
	       (method static $bitnotu32::uint32 (::uint32)
		  "BITNOTINT32")
	       (method static $bitnots64::int64 (::int64)
		  "BITNOTINT64")
	       (method static $bitnotu64::uint64 (::uint64)
		  "BITNOTINT64")
	       
	       (method static c-bitrsh::long (::long ::int)
		  "BITRSH")
	       (method static c-bitrshelong::elong (::elong ::int)
		  "BITRSHELONG")
	       (method static c-bitrshllong::llong (::llong ::int)
		  "BITRSHLLONG")
	       (method static $bitrshs8::int8 (::int8 ::int)
		  "BITRSHINT8")
	       (method static $bitrshu8::int8 (::int8 ::int)
		  "BITRSHINT8")
	       (method static $bitrshs16::int16 (::int16 ::int)
		  "BITRSHINT16")
	       (method static $bitrshu16::uint16 (::uint16 ::int)
		  "BITRSHINT16")
	       (method static $bitrshs32::int32 (::int32 ::int)
		  "BITRSHINT32")
	       (method static $bitrshu32::uint32 (::uint32 ::int)
		  "BITRSHINT32")
	       (method static $bitrshs64::int64 (::int64 ::int)
		  "BITRSHINT64")
	       (method static $bitrshu64::uint64 (::uint64 ::int)
		  "BITRSHINT64")
	       
	       (method static c-bitursh::ulong (::ulong ::int)
		  "BITURSH")
	       (method static c-biturshelong::uelong (::uelong ::int)
		  "BITURSHELONG")
	       (method static c-biturshllong::ullong (::ullong ::int)
		  "BITURSHLLONG")
	       (method static $biturshs8::int8 (::int8 ::int)
		  "BITURSHINT8")
	       (method static $biturshu8::uint8 (::uint8 ::int)
		  "BITURSHINT8")
	       (method static $biturshs16::int16 (::int16 ::int)
		  "BITURSHINT16")
	       (method static $biturshu16::uint16 (::uint16 ::int)
		  "BITURSHINT16")
	       (method static $biturshs32::int32 (::int32 ::int)
		  "BITURSHINT32")
	       (method static $biturshu32::uint32 (::uint32 ::int)
		  "BITURSHINT32")
	       (method static $biturshs64::int64 (::int64 ::int)
		  "BITURSHINT64")
	       (method static $biturshu64::uint64 (::uint64 ::int)
		  "BITURSHINT64")
	       
	       (method static c-bitlsh::long (::long ::int)
		  "BITLSH")
	       (method static c-bitlshelong::elong (::elong ::int)
		  "BITLSHELONG")
	       (method static c-bitlshllong::llong (::llong ::int)
		  "BITLSHLLONG")
	       (method static $bitlshu8::uint8 (::uint8 ::int)
		  "BITLSHINT8")
	       (method static $bitlshs8::int8 (::int8 ::int)
		  "BITLSHINT8")
	       (method static $bitlshu16::int16 (::uint16 ::int)
		  "BITLSHINT16")
	       (method static $bitlshs16::int16 (::int16 ::int)
		  "BITLSHINT16")
	       (method static $bitlshu32::uint32 (::uint32 ::uint32)
		  "BITLSHINT32")
	       (method static $bitlshs32::int32 (::int32 ::int)
		  "BITLSHINT32")
	       (method static $bitlshu64::int64 (::uint64 ::int)
		  "BITLSHINT64")
	       (method static $bitlshs64::int64 (::int64 ::int)
		  "BITLSHINT64")
	       ))
   
   (export  (inline bit-or::long ::long ::long)
	    (inline bit-orelong::elong ::elong ::elong)
	    (inline bit-orllong::llong ::llong ::llong)
	    (inline bit-ors8::int8 ::int8 ::int8)
	    (inline bit-oru8::uint8 ::uint8 ::uint8)
	    (inline bit-ors16::int16 ::int16 ::int16)
	    (inline bit-oru16::uint16 ::uint16 ::uint16)
	    (inline bit-ors32::int32 ::int32 ::int32)
	    (inline bit-oru32::uint32 ::uint32 ::uint32)
	    (inline bit-ors64::int64 ::int64 ::int64)
	    (inline bit-oru64::uint64 ::uint64 ::uint64)
	    (inline bit-orbx::bignum ::bignum ::bignum)
	    
	    (inline bit-and::long ::long ::long)
	    (inline bit-andelong::elong ::elong ::elong)
	    (inline bit-andllong::llong ::llong ::llong)
	    (inline bit-ands8::int8 ::int8 ::int8)
	    (inline bit-andu8::uint8 ::uint8 ::uint8)
	    (inline bit-ands16::int16 ::int16 ::int16)
	    (inline bit-andu16::uint16 ::uint16 ::uint16)
	    (inline bit-ands32::int32 ::int32 ::int32)
	    (inline bit-andu32::uint32 ::uint32 ::uint32)
	    (inline bit-ands64::int64 ::int64 ::int64)
	    (inline bit-andu64::uint64 ::uint64 ::uint64)
	    (inline bit-andbx::bignum ::bignum ::bignum)
	    (inline bit-maskbx::bignum ::bignum ::long)
	    
	    (inline bit-xor::long ::long ::long)
	    (inline bit-xorelong::elong ::elong ::elong)
	    (inline bit-xorllong::llong ::llong ::llong)
	    (inline bit-xors8::int8 ::int8 ::int8)
	    (inline bit-xoru8::uint8 ::uint8 ::uint8)
	    (inline bit-xors16::int16 ::int16 ::int16)
	    (inline bit-xoru16::uint16 ::uint16 ::uint16)
	    (inline bit-xors32::int32 ::int32 ::int32)
	    (inline bit-xoru32::uint32 ::uint32 ::uint32)
	    (inline bit-xors64::int64 ::int64 ::int64)
	    (inline bit-xoru64::uint64 ::uint64 ::uint64)
	    (inline bit-xorbx::bignum ::bignum ::bignum)
	    
	    (inline bit-not::long ::long)
	    (inline bit-notelong::elong ::elong)
	    (inline bit-notllong::llong ::llong)
	    (inline bit-nots8::int8 ::int8)
	    (inline bit-notu8::uint8 ::uint8)
	    (inline bit-nots16::int16 ::int16)
	    (inline bit-notu16::uint16 ::uint16)
	    (inline bit-nots32::int32 ::int32)
	    (inline bit-notu32::uint32 ::uint32)
	    (inline bit-nots64::int64 ::int64)
	    (inline bit-notu64::uint64 ::uint64)
	    (inline bit-notbx::bignum ::bignum)
	    
	    (inline bit-rsh::long ::long ::long)
	    (inline bit-rshelong::elong ::elong ::long)
	    (inline bit-rshllong::llong ::llong ::long)
	    (inline bit-rshs8::int8 ::int8 ::long)
	    (inline bit-rshu8::uint8 ::uint8 ::long)
	    (inline bit-rshs16::int16 ::int16 ::long)
	    (inline bit-rshu16::int16 ::int16 ::long)
	    (inline bit-rshs32::int32 ::int32 ::long)
	    (inline bit-rshu32::uint32 ::uint32 ::long)
	    (inline bit-rshs64::int64 ::int64 ::long)
	    (inline bit-rshu64::uint64 ::uint64 ::long)
	    
	    (inline bit-ursh::ulong ::ulong ::long)
	    (inline bit-urshelong::uelong ::uelong ::long)
	    (inline bit-urshllong::ullong ::ullong ::long)
	    (inline bit-urshs8::int8 ::int8 ::long)
	    (inline bit-urshu8::uint8 ::uint8 ::long)
	    (inline bit-urshs16::int16 ::int16 ::long)
	    (inline bit-urshu16::uint16 ::uint16 ::long)
	    (inline bit-urshs32::int32 ::int32 ::long)
	    (inline bit-urshu32::uint32 ::uint32 ::long)
	    (inline bit-urshs64::int64 ::int64 ::long)
	    (inline bit-urshu64::uint64 ::uint64 ::long)
	    
	    (inline bit-lsh::long ::long ::long)
	    (inline bit-lshelong::elong ::elong ::long)
	    (inline bit-lshllong::llong ::llong ::long)
	    (inline bit-lshs8::int8 ::int8 ::long)
	    (inline bit-lshu8::uint8 ::uint8 ::long)
	    (inline bit-lshs16::int16 ::int16 ::long)
	    (inline bit-lshu16::uint16 ::uint16 ::long)
	    (inline bit-lshs32::int32 ::int32 ::long)
	    (inline bit-lshu32::uint32 ::uint32 ::long)
	    (inline bit-lshs64::int64 ::int64 ::long)
	    (inline bit-lshu64::uint64 ::uint64 ::long)
	    (inline bit-lshbx::bignum ::bignum ::long)
	    (inline bit-rshbx::bignum ::bignum ::long)
	    )
   
   (pragma  (bit-or side-effect-free no-cfa-top nesting)
	    (bit-orelong side-effect-free no-cfa-top nesting)
	    (bit-orllong side-effect-free no-cfa-top nesting)
	    (bit-ors8 side-effect-free no-cfa-top nesting)
	    (bit-oru8 side-effect-free no-cfa-top nesting)
	    (bit-ors16 side-effect-free no-cfa-top nesting)
	    (bit-oru16 side-effect-free no-cfa-top nesting)
	    (bit-ors32 side-effect-free no-cfa-top nesting)
	    (bit-oru32 side-effect-free no-cfa-top nesting)
	    (bit-ors64 side-effect-free no-cfa-top nesting)
	    (bit-oru64 side-effect-free no-cfa-top nesting)
	    (bit-orbx side-effect-free no-cfa-top nesting)
	    (c-bitor side-effect-free no-cfa-top nesting args-safe)
	    (c-bitorelong side-effect-free no-cfa-top nesting args-safe)
	    (c-bitorllong side-effect-free no-cfa-top nesting args-safe)
	    ($bitors8 side-effect-free no-cfa-top nesting args-safe)
	    ($bitoru8 side-effect-free no-cfa-top nesting args-safe)
	    ($bitors16 side-effect-free no-cfa-top nesting args-safe)
	    ($bitoru16 side-effect-free no-cfa-top nesting args-safe)
	    ($bitors32 side-effect-free no-cfa-top nesting args-safe)
	    ($bitoru32 side-effect-free no-cfa-top nesting args-safe)
	    ($bitors64 side-effect-free no-cfa-top nesting args-safe)
	    ($bitoru64 side-effect-free no-cfa-top nesting args-safe)

	    (bit-and side-effect-free no-cfa-top nesting)
	    (bit-andelong side-effect-free no-cfa-top nesting)
	    (bit-andllong side-effect-free no-cfa-top nesting)
	    (bit-ands8 side-effect-free no-cfa-top nesting)
	    (bit-andu8 side-effect-free no-cfa-top nesting)
	    (bit-ands16 side-effect-free no-cfa-top nesting)
	    (bit-andu16 side-effect-free no-cfa-top nesting)
	    (bit-ands32 side-effect-free no-cfa-top nesting)
	    (bit-andu32 side-effect-free no-cfa-top nesting)
	    (bit-ands64 side-effect-free no-cfa-top nesting)
	    (bit-andu64 side-effect-free no-cfa-top nesting)
	    (bit-andbx side-effect-free no-cfa-top nesting)
	    (bit-maskbx side-effect-free no-cfa-top nesting)
	    (c-bitand side-effect-free no-cfa-top nesting args-safe)
	    (c-bitandelong side-effect-free no-cfa-top nesting args-safe)
	    (c-bitandllong side-effect-free no-cfa-top nesting args-safe)
	    ($bitands8 side-effect-free no-cfa-top nesting args-safe)
	    ($bitandu8 side-effect-free no-cfa-top nesting args-safe)
	    ($bitands16 side-effect-free no-cfa-top nesting args-safe)
	    ($bitandu16 side-effect-free no-cfa-top nesting args-safe)
	    ($bitands32 side-effect-free no-cfa-top nesting args-safe)
	    ($bitandu32 side-effect-free no-cfa-top nesting args-safe)
	    ($bitands64 side-effect-free no-cfa-top nesting args-safe)
	    ($bitandu64 side-effect-free no-cfa-top nesting args-safe)

	    (bit-xor side-effect-free no-cfa-top nesting)
	    (bit-xorelong side-effect-free no-cfa-top nesting)
	    (bit-xorllong side-effect-free no-cfa-top nesting)
	    (bit-xors8 side-effect-free no-cfa-top nesting)
	    (bit-xoru8 side-effect-free no-cfa-top nesting)
	    (bit-xors16 side-effect-free no-cfa-top nesting)
	    (bit-xoru16 side-effect-free no-cfa-top nesting)
	    (bit-xors32 side-effect-free no-cfa-top nesting)
	    (bit-xoru32 side-effect-free no-cfa-top nesting)
	    (bit-xors64 side-effect-free no-cfa-top nesting)
	    (bit-xoru64 side-effect-free no-cfa-top nesting)
	    (bit-xorbx side-effect-free no-cfa-top nesting)
	    (c-bitxor side-effect-free no-cfa-top nesting args-safe)
	    (c-bitxorelong side-effect-free no-cfa-top nesting args-safe)
	    (c-bitxorllong side-effect-free no-cfa-top nesting args-safe)
	    ($bitxors8 side-effect-free no-cfa-top nesting args-safe)
	    ($bitxoru8 side-effect-free no-cfa-top nesting args-safe)
	    ($bitxors16 side-effect-free no-cfa-top nesting args-safe)
	    ($bitxoru16 side-effect-free no-cfa-top nesting args-safe)
	    ($bitxors32 side-effect-free no-cfa-top nesting args-safe)
	    ($bitxoru32 side-effect-free no-cfa-top nesting args-safe)
	    ($bitxors64 side-effect-free no-cfa-top nesting args-safe)
	    ($bitxoru64 side-effect-free no-cfa-top nesting args-safe)

	    (bit-not side-effect-free no-cfa-top nesting)
	    (bit-notelong side-effect-free no-cfa-top nesting)
	    (bit-notllong side-effect-free no-cfa-top nesting)
	    (bit-nots8 side-effect-free no-cfa-top nesting)
	    (bit-notu8 side-effect-free no-cfa-top nesting)
	    (bit-nots16 side-effect-free no-cfa-top nesting)
	    (bit-notu16 side-effect-free no-cfa-top nesting)
	    (bit-nots32 side-effect-free no-cfa-top nesting)
	    (bit-notu32 side-effect-free no-cfa-top nesting)
	    (bit-nots64 side-effect-free no-cfa-top nesting)
	    (bit-notu64 side-effect-free no-cfa-top nesting)
	    (bit-notbx side-effect-free no-cfa-top nesting)
	    (c-bitnot side-effect-free no-cfa-top nesting args-safe)
	    (c-bitnotelong side-effect-free no-cfa-top nesting args-safe)
	    (c-bitnotllong side-effect-free no-cfa-top nesting args-safe)
	    ($bitnots8 side-effect-free no-cfa-top nesting args-safe)
	    ($bitnotu8 side-effect-free no-cfa-top nesting args-safe)
	    ($bitnots16 side-effect-free no-cfa-top nesting args-safe)
	    ($bitnotu16 side-effect-free no-cfa-top nesting args-safe)
	    ($bitnots32 side-effect-free no-cfa-top nesting args-safe)
	    ($bitnotu32 side-effect-free no-cfa-top nesting args-safe)
	    ($bitnots64 side-effect-free no-cfa-top nesting args-safe)
	    ($bitnotu64 side-effect-free no-cfa-top nesting args-safe)

	    (bit-rsh side-effect-free no-cfa-top nesting)
	    (bit-rshelong side-effect-free no-cfa-top nesting)
	    (bit-rshllong side-effect-free no-cfa-top nesting)
	    (bit-rshs8 side-effect-free no-cfa-top nesting)
	    (bit-rshu8 side-effect-free no-cfa-top nesting)
	    (bit-rshs16 side-effect-free no-cfa-top nesting)
	    (bit-rshu16 side-effect-free no-cfa-top nesting)
	    (bit-rshs32 side-effect-free no-cfa-top nesting)
	    (bit-rshu32 side-effect-free no-cfa-top nesting)
	    (bit-rshs64 side-effect-free no-cfa-top nesting)
	    (bit-rshu64 side-effect-free no-cfa-top nesting)
	    (c-bitrsh side-effect-free no-cfa-top nesting args-safe)
	    (c-bitrshelong side-effect-free no-cfa-top nesting args-safe)
	    (c-bitrshllong side-effect-free no-cfa-top nesting args-safe)
	    ($bitrshs8 side-effect-free no-cfa-top nesting args-safe)
	    ($bitrshu8 side-effect-free no-cfa-top nesting args-safe)
	    ($bitrshs16 side-effect-free no-cfa-top nesting args-safe)
	    ($bitrshu16 side-effect-free no-cfa-top nesting args-safe)
	    ($bitrshs32 side-effect-free no-cfa-top nesting args-safe)
	    ($bitrshu32 side-effect-free no-cfa-top nesting args-safe)
	    ($bitrshs64 side-effect-free no-cfa-top nesting args-safe)
	    ($bitrshu64 side-effect-free no-cfa-top nesting args-safe)

	    (bit-ursh side-effect-free no-cfa-top nesting)
	    (bit-urshelong side-effect-free no-cfa-top nesting)
	    (bit-urshllong side-effect-free no-cfa-top nesting)
	    (bit-urshu8 side-effect-free no-cfa-top nesting)
	    (bit-urshu16 side-effect-free no-cfa-top nesting)
	    (bit-urshu32 side-effect-free no-cfa-top nesting)
	    (bit-urshu64 side-effect-free no-cfa-top nesting)
	    (c-bitursh side-effect-free no-cfa-top nesting args-safe)
	    (c-biturshelong side-effect-free no-cfa-top nesting args-safe)
	    (c-biturshllong side-effect-free no-cfa-top nesting args-safe)
	    ($biturshs8 side-effect-free no-cfa-top nesting args-safe)
	    ($biturshu8 side-effect-free no-cfa-top nesting args-safe)
	    ($biturshs16 side-effect-free no-cfa-top nesting args-safe)
	    ($biturshu16 side-effect-free no-cfa-top nesting args-safe)
	    ($biturshs32 side-effect-free no-cfa-top nesting args-safe)
	    ($biturshu32 side-effect-free no-cfa-top nesting args-safe)
	    ($biturshs64 side-effect-free no-cfa-top nesting args-safe)
	    ($biturshu64 side-effect-free no-cfa-top nesting args-safe)

	    (bit-lsh side-effect-free no-cfa-top nesting)
	    (bit-lshelong side-effect-free no-cfa-top nesting)
	    (bit-lshllong side-effect-free no-cfa-top nesting)
	    (bit-lshs8 side-effect-free no-cfa-top nesting)
	    (bit-lshu8 side-effect-free no-cfa-top nesting)
	    (bit-lshs16 side-effect-free no-cfa-top nesting)
	    (bit-lshu16 side-effect-free no-cfa-top nesting)
	    (bit-lshs32 side-effect-free no-cfa-top nesting)
	    (bit-lshu32 side-effect-free no-cfa-top nesting)
	    (bit-lshs64 side-effect-free no-cfa-top nesting)
	    (bit-lshu64 side-effect-free no-cfa-top nesting)
	    (bit-lshbx side-effect-free no-cfa-top nesting)
	    (bit-rshbx side-effect-free no-cfa-top nesting)
	    (c-bitlsh side-effect-free no-cfa-top nesting args-safe)
	    (c-bitlshelong side-effect-free no-cfa-top nesting args-safe)
	    (c-bitlshllong side-effect-free no-cfa-top nesting args-safe)
	    ($bitlshs8 side-effect-free no-cfa-top nesting args-safe)
	    ($bitlshu8 side-effect-free no-cfa-top nesting args-safe)
	    ($bitlshs16 side-effect-free no-cfa-top nesting args-safe)
	    ($bitlshu16 side-effect-free no-cfa-top nesting args-safe)
	    ($bitlshs32 side-effect-free no-cfa-top nesting args-safe)
	    ($bitlshu32 side-effect-free no-cfa-top nesting args-safe)
	    ($bitlshs64 side-effect-free no-cfa-top nesting args-safe)
	    ($bitlshu64 side-effect-free no-cfa-top nesting args-safe)
	    ))

;*---------------------------------------------------------------------*/
;*    bit-or ...                                                       */
;*---------------------------------------------------------------------*/
(define-inline (bit-or x y) (c-bitor x y))
(define-inline (bit-orelong x y) (c-bitorelong x y))
(define-inline (bit-orllong x y) (c-bitorllong x y))
(define-inline (bit-ors8 x y) ($bitors8 x y))
(define-inline (bit-oru8 x y) ($bitoru8 x y))
(define-inline (bit-ors16 x y) ($bitors16 x y))
(define-inline (bit-oru16 x y) ($bitoru16 x y))
(define-inline (bit-ors32 x y) ($bitors32 x y))
(define-inline (bit-oru32 x y) ($bitoru32 x y))
(define-inline (bit-ors64 x y) ($bitors64 x y))
(define-inline (bit-oru64 x y) ($bitoru64 x y))
(define-inline (bit-orbx x y) ($bitorbx x y))

;*---------------------------------------------------------------------*/
;*    bit-and ...                                                      */
;*---------------------------------------------------------------------*/
(define-inline (bit-and x y) (c-bitand x y))
(define-inline (bit-andelong x y) (c-bitandelong x y))
(define-inline (bit-andllong x y) (c-bitandllong x y))
(define-inline (bit-ands8 x y) ($bitands8 x y))
(define-inline (bit-andu8 x y) ($bitandu8 x y))
(define-inline (bit-ands16 x y) ($bitands16 x y))
(define-inline (bit-andu16 x y) ($bitandu16 x y))
(define-inline (bit-ands32 x y) ($bitands32 x y))
(define-inline (bit-andu32 x y) ($bitandu32 x y))
(define-inline (bit-ands64 x y) ($bitands64 x y))
(define-inline (bit-andu64 x y) ($bitandu64 x y))
(define-inline (bit-andbx x y) ($bitandbx x y))

(define-inline (bit-maskbx x n) ($bitmaskbx x n))

;*---------------------------------------------------------------------*/
;*    bit-xor ...                                                      */
;*---------------------------------------------------------------------*/
(define-inline (bit-xor x y) (c-bitxor x y))
(define-inline (bit-xorelong x y) (c-bitxorelong x y))
(define-inline (bit-xorllong x y) (c-bitxorllong x y))
(define-inline (bit-xors8 x y) ($bitxors8 x y))
(define-inline (bit-xoru8 x y) ($bitxoru8 x y))
(define-inline (bit-xors16 x y) ($bitxors16 x y))
(define-inline (bit-xoru16 x y) ($bitxoru16 x y))
(define-inline (bit-xors32 x y) ($bitxors32 x y))
(define-inline (bit-xoru32 x y) ($bitxoru32 x y))
(define-inline (bit-xors64 x y) ($bitxors64 x y))
(define-inline (bit-xoru64 x y) ($bitxoru64 x y))
(define-inline (bit-xorbx x y) ($bitxorbx x y))

;*---------------------------------------------------------------------*/
;*    bit-not ...                                                      */
;*---------------------------------------------------------------------*/
(define-inline (bit-not x) (c-bitnot x))
(define-inline (bit-notelong x) (c-bitnotelong x))
(define-inline (bit-notllong x) (c-bitnotllong x))
(define-inline (bit-nots8 x) ($bitnots8 x))
(define-inline (bit-notu8 x) ($bitnotu8 x))
(define-inline (bit-nots16 x) ($bitnots16 x))
(define-inline (bit-notu16 x) ($bitnotu16 x))
(define-inline (bit-nots32 x) ($bitnots32 x))
(define-inline (bit-notu32 x) ($bitnotu32 x))
(define-inline (bit-nots64 x) ($bitnots64 x))
(define-inline (bit-notu64 x) ($bitnotu64 x))
(define-inline (bit-notbx x) ($bitnotbx x))
   
;*---------------------------------------------------------------------*/
;*    bit-rsh ...                                                      */
;*---------------------------------------------------------------------*/
(define-inline (bit-rsh x y) (c-bitrsh x y))
(define-inline (bit-rshelong x y) (c-bitrshelong x y))
(define-inline (bit-rshllong x y) (c-bitrshllong x y))
(define-inline (bit-rshs8 x y) ($bitrshs8 x y))
(define-inline (bit-rshu8 x y) ($bitrshu8 x y))
(define-inline (bit-rshs16 x y) ($bitrshs16 x y))
(define-inline (bit-rshu16 x y) ($bitrshu16 x y))
(define-inline (bit-rshs32 x y) ($bitrshs32 x y))
(define-inline (bit-rshu32 x y) ($bitrshu32 x y))
(define-inline (bit-rshs64 x y) ($bitrshs64 x y))
(define-inline (bit-rshu64 x y) ($bitrshu64 x y))
(define-inline (bit-rshbx x y) ($bitrshbx x y))

;*---------------------------------------------------------------------*/
;*    bit-ursh ...                                                     */
;*---------------------------------------------------------------------*/
(define-inline (bit-ursh x y) (c-bitursh x y))
(define-inline (bit-urshelong x y) (c-biturshelong x y))
(define-inline (bit-urshllong x y) (c-biturshllong x y))
(define-inline (bit-urshu8 x y) ($biturshu8 x y))
(define-inline (bit-urshs8 x y) ($biturshs8 x y))
(define-inline (bit-urshs16 x y) ($biturshs16 x y))
(define-inline (bit-urshu16 x y) ($biturshu16 x y))
(define-inline (bit-urshs32 x y) ($biturshs32 x y))
(define-inline (bit-urshu32 x y) ($biturshu32 x y))
(define-inline (bit-urshs64 x y) ($biturshs64 x y))
(define-inline (bit-urshu64 x y) ($biturshu64 x y))
       
;*---------------------------------------------------------------------*/
;*    bit-lsh ...                                                      */
;*---------------------------------------------------------------------*/
(define-inline (bit-lsh x y) (c-bitlsh x y))
(define-inline (bit-lshelong x y) (c-bitlshelong x y))
(define-inline (bit-lshllong x y) (c-bitlshllong x y))
(define-inline (bit-lshs8 x y) ($bitlshs8 x y))
(define-inline (bit-lshu8 x y) ($bitlshu8 x y))
(define-inline (bit-lshs16 x y) ($bitlshs16 x y))
(define-inline (bit-lshu16 x y) ($bitlshu16 x y))
(define-inline (bit-lshs32 x y) ($bitlshs32 x y))
(define-inline (bit-lshu32 x y) ($bitlshu32 x y))
(define-inline (bit-lshs64 x y) ($bitlshs64 x y))
(define-inline (bit-lshu64 x y) ($bitlshu64 x y))
(define-inline (bit-lshbx x y) ($bitlshbx x y))



