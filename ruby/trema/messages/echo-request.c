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
pack_echo_request( VALUE self, VALUE options ) {
  UNUSED( self );
  
  uint32_t xid = get_transaction_id();
  Check_Type( options, T_HASH );
  VALUE xid_r = rb_hash_aref( options, ID2SYM( rb_intern( "transaction_id" ) ) );
  if ( xid_r != Qnil ) {
    xid = NUM2UINT( xid_r );
  }

  VALUE body_r = rb_hash_aref( options, ID2SYM( rb_intern( "user_data" ) ) );
  buffer *body = NULL;
  if ( body_r != Qnil ) {
    if ( TYPE( body_r ) == T_ARRAY ) {
        uint16_t buffer_len = ( uint16_t ) RARRAY_LEN( body_r );

        body = alloc_buffer_with_length( ( size_t ) RARRAY_LEN( body_r ) );
        append_back_buffer( body, buffer_len );
        uint8_t *buf = body->data;

        
        for ( int i = 0; i < buffer_len && i < RARRAY_LEN( body_r ); i++ ) {
          buf[ i ]= ( uint8_t ) FIX2INT( RARRAY_PTR( body_r )[ i ] );
        }
    }
    else {
      rb_raise( rb_eTypeError, "echo request user data must be specified as an array of bytes" );
    }
  }
  return create_echo_request( xid, body );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
