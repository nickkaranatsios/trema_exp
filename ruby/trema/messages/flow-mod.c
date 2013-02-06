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


buffer *
pack_flow_mod( VALUE options ) {
  VALUE sym_transaction_id = ID2SYM( rb_intern( "transaction_id" ) );

  uint32_t xid = get_transaction_id();
  VALUE xid_r = rb_hash_aref( options, sym_transaction_id );
  if ( xid_r != Qnil ) {
    xid = NUM2UINT( xid_r );
  }

  VALUE sym_cookie = ID2SYM( rb_intern( "cookie" ) );
  uint64_t cookie = get_cookie();
  VALUE cookie_r = rb_hash_aref( options, sym_cookie );
  if ( cookie_r != Qnil ) {
    cookie = ( uint64_t ) NUM2ULL( cookie_r );
  }

  VALUE sym_cookie_mask = ID2SYM( rb_intern( "cookie_mask" ) );
  uint64_t cookie_mask = 0;
  VALUE cookie_mask_r = rb_hash_aref( options, sym_cookie_mask );
  if ( cookie_mask_r != Qnil ) {
    cookie_mask = ( uint64_t ) NUM2ULL( cookie_mask_r );
  }

  VALUE sym_table_id = ID2SYM( rb_intern( "table_id" ) );
  uint8_t table_id = 0;
  VALUE table_id_r = rb_hash_aref( options, sym_table_id );
  if ( table_id_r != Qnil ) {
    table_id_r = ( uint8_t ) NUM2UINT( table_id_r );
  }

  VALUE sym_command = ID2SYM( rb_intern( "command" ) );
  uint8_t command = OFPFC_ADD;
  VALUE command_r = rb_hash_aref( options, sym_command );
  if ( command_r != Qnil ) {
    command = ( uint8_t ) NUM2UINT( command_r );
  }

  VALUE sym_idle_timeout = ID2SYM( rb_intern( "idle_timeout" ) );
  uint16_t idle_timeout = 0;
  VALUE idle_timeout_r = rb_hash_aref( options, sym_idle_timeout );
  if ( idle_timeout_r != Qnil ) {
    idle_timeout = ( uint16_t ) NUM2UINT( idle_timeout_r );
  }

  VALUE sym_hard_timeout = ID2SYM( rb_intern( "hard_timeout" ) );
  uint16_t hard_timeout = 0;
  VALUE hard_timeout_r = rb_hash_aref( options, sym_hard_timeout );
  if ( hard_timeout_r != Qnil ) {
    hard_timeout = ( uint16_t ) NUM2UINT( hard_timeout_r );
  }

  VALUE sym_priority = ID2SYM( rb_intern( "priority" ) );
  uint16_t priority = 0;
  VALUE priority_r = rb_hash_aref( options, sym_priority );
  if ( priority_r != Qnil ) {
    priority = ( uint16_t ) NUM2UINT( priority_r );
  }

  VALUE sym_buffer_id = ID2SYM( rb_intern( "buffer_id" ) );
  uint32_t buffer_id = OFP_NO_BUFFER;
  VALUE buffer_id_r = rb_hash_aref( options, sym_buffer_id );
  if ( buffer_id_r != Qnil ) {
    buffer_id = NUM2UINT( buffer_id_r );
  }

  VALUE sym_out_port = ID2SYM( rb_intern( "out_port" ) );
  uint32_t out_port = 0;
  VALUE out_port_r = rb_hash_aref( options, sym_out_port );
  if ( out_port_r != Qnil ) {
    out_port = NUM2UINT( out_port_r );
  }

  VALUE sym_out_group = ID2SYM( rb_intern( "out_group" ) );
  uint32_t out_group = 0;
  VALUE out_group_r = rb_hash_aref( options, sym_out_group );
  if ( out_group_r != Qnil ) {
    out_group = NUM2UINT( out_group_r );
  }

  VALUE sym_flags = ID2SYM( rb_intern( "flags" ) );
  uint16_t flags = 0;
  VALUE flags_r = rb_hash_aref( options, sym_flags );
  if ( flags_r != Qnil ) {
    flags = ( uint16_t ) NUM2UINT( flags_r );
  }

  VALUE sym_instructions = ID2SYM( rb_intern( "instructions" ) );
  openflow_instructions *instructions = NULL;
  VALUE instructions_r = rb_hash_aref( options, sym_instructions );
  if ( instructions_r != Qnil ) {
    instructions = pack_instruction( instructions_r );
  }

printf("about to create a flow_mod\n");
  return create_flow_mod( xid, 
                          cookie,
                          cookie_mask,
                          table_id,
                          command,
                          idle_timeout,
                          hard_timeout,
                          priority,
                          buffer_id,
                          out_port,
                          out_group,
                          flags,
                          NULL,
                          instructions );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
