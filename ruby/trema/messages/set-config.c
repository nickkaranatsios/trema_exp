/*
 * Copyright (C) 2008-2013 NEC Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */


#include "trema.h"
#include "ruby.h"


buffer *
pack_set_config( VALUE options ) {
  VALUE sym_transaction_id = ID2SYM( rb_intern( "transaction_id" ) );
  VALUE sym_flags = ID2SYM( rb_intern( "flags" ) );
  VALUE sym_miss_send_len = ID2SYM( rb_intern( "miss_send_len" ) );

  uint32_t xid = get_transaction_id();
  VALUE xid_r = rb_hash_aref( options, sym_transaction_id );
  if ( xid_r != Qnil ) {
    xid = NUM2UINT( xid_r );
  }

  uint16_t flags = 0;
  VALUE flags_r = rb_hash_aref( options, sym_flags );
  if ( flags_r != Qnil ) {
    flags = ( uint16_t ) NUM2UINT( flags_r );
  }
 
  uint16_t miss_send_len;
  VALUE miss_send_len_r = rb_hash_aref( options, sym_miss_send_len );
  miss_send_len = ( uint16_t ) NUM2UINT( miss_send_len_r );

  return create_set_config( xid, flags, miss_send_len );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
