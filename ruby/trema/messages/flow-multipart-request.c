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


buffer *
pack_flow_multipart_request( VALUE options ) {
  VALUE sym_transaction_id = ID2SYM( rb_intern( "transaction_id" ) );

  uint32_t xid = get_transaction_id();
  VALUE r_xid = rb_hash_aref( options, sym_transaction_id );
  if ( r_xid != Qnil ) {
    xid = NUM2UINT( r_xid );
  }

  uint8_t table_id = OFPTT_ALL;
  VALUE sym_table_id = ID2SYM( rb_intern( "table" ) );
  VALUE r_table_id = rb_hash_aref( options, sym_table_id );
  if ( r_table_id != Qnil ) {
    table_id = ( uint8_t ) NUM2UINT( r_table_id );
  }

  uint32_t out_port = OFPP_ANY;
  VALUE sym_out_port = ID2SYM( rb_intern( "out_port" ) );
  VALUE r_out_port = rb_hash_aref( options, sym_out_port );
  if ( r_out_port != Qnil ) {
    out_port = ( uint32_t ) NUM2UINT( r_out_port );
  }

  uint32_t out_group = OFPG_ANY;
  VALUE sym_out_group = ID2SYM( rb_intern( "out_group" ) );
  VALUE r_out_group = rb_hash_aref( options, sym_out_group );
  if ( r_out_group != Qnil ) {
    out_group = ( uint32_t ) NUM2UINT( r_out_group );
  }
  
  uint64_t cookie = 0;
  VALUE sym_cookie = ID2SYM( rb_intern( "cookie" ) );
  VALUE r_cookie = rb_hash_aref( options, sym_cookie );
  if ( r_cookie != Qnil ) {
    cookie = ( uint64_t ) NUM2ULL( r_cookie );
  }
  else {
    rb_raise( rb_eArgError, "Cookie option must be specified" );
  }

  uint64_t cookie_mask = 0;
  VALUE sym_cookie_mask = ID2SYM( rb_intern( "cookie_mask" ) );
  VALUE r_cookie_mask = rb_hash_aref( options, sym_cookie_mask );
  if ( r_cookie_mask != Qnil ) {
    cookie_mask = ( uint64_t ) NUM2ULL( r_cookie_mask );
  }
  
  VALUE sym_match = ID2SYM( rb_intern( "match" ) );
  VALUE r_match = rb_hash_aref( options, sym_match );
  oxm_matches *oxm_match = NULL;
  if ( r_match != Qnil ) {
    oxm_match = create_oxm_matches();
    r_match_to_oxm_match( r_match, oxm_match );
  }
  
  buffer *flow_multipart_request = create_flow_multipart_request( xid,
                                                                  OFPMP_FLOW,
                                                                  table_id,
                                                                  out_port,
                                                                  out_group,
                                                                  cookie,
                                                                  cookie_mask,
                                                                  oxm_match );

  if ( oxm_match != NULL ) {
    delete_oxm_matches( oxm_match );
  }
  return flow_multipart_request;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
