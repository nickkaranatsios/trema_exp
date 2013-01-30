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
pack_hello( VALUE self, VALUE options ) {
  UNUSED( self );
  
  uint32_t xid = get_transaction_id();
  Check_Type( options, T_HASH );
  VALUE xid_r = rb_hash_aref( options, ID2SYM( rb_intern( "transaction_id" ) ) );
  if ( xid_r != Qnil ) {
    xid = NUM2UINT( xid_r );
  }

  VALUE version_r = rb_hash_aref( options, ID2SYM( rb_intern( "version" ) ) );
  uint32_t ofp_version[ 1 ];
  if ( rb_obj_is_kind_of( version_r, rb_cArray ) )  {
    if ( RARRAY_LEN( version_r ) > 1 ) {
      rb_raise(rb_eArgError, "Currently only a single version is supported" );
    }
    else {
      ofp_version[ 0 ] = ( uint32_t ) NUM2UINT( RARRAY_PTR( version_r )[ 0 ] );
    }
  }
  return create_hello_elem_versionbitmap( xid, ofp_version, sizeof( ofp_version ) / sizeof( ofp_version[ 0 ] ) );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
