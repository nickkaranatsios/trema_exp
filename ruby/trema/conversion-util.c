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


#include <assert.h>
#include "trema.h"
#include "ruby.h"
#include "oxm-helper.h"


buffer *
r_array_to_buffer( VALUE r_array ) {
  buffer *data = NULL;

  if ( r_array != Qnil ) {
    Check_Type( r_array, T_ARRAY );
    uint32_t length = ( uint32_t ) RARRAY_LEN( r_array );

    data = alloc_buffer_with_length( length );
    append_back_buffer( data, length );
    uint8_t *data_ptr = data->data;
    for ( uint32_t i = 0; i < length; i++ ) {
      data_ptr[ i ] = ( uint8_t ) FIX2INT( RARRAY_PTR( r_array ) [ i ] );
    }
  }
  return data;
}


VALUE
oxm_match_to_r_match( const oxm_matches *match ) {
  assert( match != NULL );

  VALUE options = rb_hash_new();

  for ( list_element *list = match->list; list != NULL; list = list->next ) {
    oxm_match_header *oxm = list->data;
    assign_match( oxm, options );
  }
  return rb_funcall( rb_eval_string( "Match" ), rb_intern( "new" ), 1, options );
}


void
r_match_to_oxm_match( VALUE r_match, oxm_matches *match ) {
  append_oxm_match_in_port( match, ( uint32_t ) NUM2UINT( rb_iv_get( r_match, "@in_port" ) ) );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */

