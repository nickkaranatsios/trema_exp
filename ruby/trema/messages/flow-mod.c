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
#include "action-common.h"
#include "conversion-util.h"


buffer *
pack_flow_mod( VALUE options ) {
  uint32_t xid = get_transaction_id();
  VALUE sym_transaction_id = ID2SYM( rb_intern( "transaction_id" ) );
  VALUE r_xid = rb_hash_aref( options, sym_transaction_id );
  if ( r_xid != Qnil ) {
    xid = NUM2UINT( r_xid );
  }

  uint64_t cookie = get_cookie();
  VALUE sym_cookie = ID2SYM( rb_intern( "cookie" ) );
  VALUE r_cookie = rb_hash_aref( options, sym_cookie );
  if ( r_cookie != Qnil ) {
    cookie = ( uint64_t ) NUM2ULL( r_cookie );
  }

  uint64_t cookie_mask = 0;
  VALUE sym_cookie_mask = ID2SYM( rb_intern( "cookie_mask" ) );
  VALUE r_cookie_mask = rb_hash_aref( options, sym_cookie_mask );
  if ( r_cookie_mask != Qnil ) {
    cookie_mask = ( uint64_t ) NUM2ULL( r_cookie_mask );
  }

  uint8_t table_id = 0;
  VALUE sym_table_id = ID2SYM( rb_intern( "table_id" ) );
  VALUE r_table_id = rb_hash_aref( options, sym_table_id );
  if ( r_table_id != Qnil ) {
    table_id = ( uint8_t ) NUM2UINT( r_table_id );
  }

  uint8_t command = OFPFC_ADD;
  VALUE sym_command = ID2SYM( rb_intern( "command" ) );
  VALUE r_command = rb_hash_aref( options, sym_command );
  if ( r_command != Qnil ) {
    command = ( uint8_t ) NUM2UINT( r_command );
  }

  uint16_t idle_timeout = 0;
  VALUE sym_idle_timeout = ID2SYM( rb_intern( "idle_timeout" ) );
  VALUE r_idle_timeout = rb_hash_aref( options, sym_idle_timeout );
  if ( r_idle_timeout != Qnil ) {
    idle_timeout = ( uint16_t ) NUM2UINT( r_idle_timeout );
  }

  uint16_t hard_timeout = 0;
  VALUE sym_hard_timeout = ID2SYM( rb_intern( "hard_timeout" ) );
  VALUE r_hard_timeout = rb_hash_aref( options, sym_hard_timeout );
  if ( r_hard_timeout != Qnil ) {
    hard_timeout = ( uint16_t ) NUM2UINT( r_hard_timeout );
  }

  uint16_t priority = 0;
  VALUE sym_priority = ID2SYM( rb_intern( "priority" ) );
  VALUE r_priority = rb_hash_aref( options, sym_priority );
  if ( r_priority != Qnil ) {
    priority = ( uint16_t ) NUM2UINT( r_priority );
  }

  uint32_t buffer_id = OFP_NO_BUFFER;
  VALUE sym_buffer_id = ID2SYM( rb_intern( "buffer_id" ) );
  VALUE r_buffer_id = rb_hash_aref( options, sym_buffer_id );
  if ( r_buffer_id != Qnil ) {
    buffer_id = NUM2UINT( r_buffer_id );
  }

  uint32_t out_port = 0;
  VALUE sym_out_port = ID2SYM( rb_intern( "out_port" ) );
  VALUE r_out_port = rb_hash_aref( options, sym_out_port );
  if ( r_out_port != Qnil ) {
    out_port = NUM2UINT( r_out_port );
  }

  uint32_t out_group = 0;
  VALUE sym_out_group = ID2SYM( rb_intern( "out_group" ) );
  VALUE r_out_group = rb_hash_aref( options, sym_out_group );
  if ( r_out_group != Qnil ) {
    out_group = NUM2UINT( r_out_group );
  }

  uint16_t flags = 0;
  VALUE sym_flags = ID2SYM( rb_intern( "flags" ) );
  VALUE r_flags = rb_hash_aref( options, sym_flags );
  if ( r_flags != Qnil ) {
    flags = ( uint16_t ) NUM2UINT( r_flags );
  }

  VALUE sym_match = ID2SYM( rb_intern( "match" ) );
  VALUE r_match = rb_hash_aref( options, sym_match );
  oxm_matches *oxm_match = NULL;
  if ( r_match != Qnil ) {
    oxm_match = create_oxm_matches();
    r_match_to_oxm_match( r_match, oxm_match );
  }
  
  VALUE sym_instructions = ID2SYM( rb_intern( "instructions" ) );
  openflow_instructions *instructions = NULL;
  VALUE r_instructions = rb_hash_aref( options, sym_instructions );
  if ( r_instructions != Qnil ) {
    instructions = pack_instruction( r_instructions );
  }
  buffer *flow_mod = create_flow_mod( xid, cookie, cookie_mask,
                                      table_id, command, idle_timeout,
                                      hard_timeout, priority, buffer_id,
                                      out_port, out_group, flags,
                                      oxm_match, instructions );
  if ( instructions != NULL ) {
    delete_instructions( instructions );
  }
  if (oxm_match != NULL ) {
    delete_oxm_matches( oxm_match );
  }
  return flow_mod;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
