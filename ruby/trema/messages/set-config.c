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
  VALUE sym_r_miss_send_len = ID2SYM( rb_intern( "r_miss_send_len" ) );

  uint32_t xid = get_transaction_id();
  VALUE r_xid = rb_hash_aref( options, sym_transaction_id );
  if ( r_xid != Qnil ) {
    xid = NUM2UINT( r_xid );
  }

  uint16_t flags = 0;
  VALUE r_flags = rb_hash_aref( options, sym_flags );
  if ( r_flags != Qnil ) {
    flags = ( uint16_t ) NUM2UINT( r_flags );
  }
 
  uint16_t r_miss_send_len;
  VALUE r_miss_send_len_r = rb_hash_aref( options, sym_r_miss_send_len );
  r_miss_send_len = ( uint16_t ) NUM2UINT( r_miss_send_len_r );

  return create_set_config( xid, flags, r_miss_send_len );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
