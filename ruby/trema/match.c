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
#include "conversion-util.h"


extern VALUE mTrema;
VALUE cMatch;


VALUE
get_match_from( VALUE self, VALUE message ) {
  UNUSED( self );
  VALUE r_match = Qnil;

  if ( rb_obj_is_instance_of( message, rb_eval_string( "Messages::PacketIn" ) ) ) {
   
    VALUE r_match = rb_iv_get( message, "@match" );
    uint32_t in_port = NUM2UINT( rb_iv_get( r_match, "@in_port" ) );
    VALUE r_data = rb_iv_get( message, "@data" );
    buffer *packet = r_array_to_buffer( r_data );

    oxm_matches *match = create_oxm_matches();
    set_match_from_packet( match, in_port, NULL, packet ); 
    r_match = oxm_match_to_r_match( match );
  }
  return r_match;
}


void
Init_match( void ) {
  cMatch = rb_define_class_under( mTrema, "Match", rb_eval_string( "Trema::Message" ) );
  rb_define_singleton_method( cMatch, "get_match_from", get_match_from, 1 );
  rb_require( "trema/match" );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
