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
#include "bucket-helper.h"


buffer *
pack_group_mod( VALUE options ) {
  VALUE sym_transaction_id = ID2SYM( rb_intern( "transaction_id" ) );

  uint32_t xid = get_transaction_id();
  VALUE r_xid = rb_hash_aref( options, sym_transaction_id );
  if ( r_xid != Qnil ) {
    xid = NUM2UINT( r_xid );
  }

  VALUE sym_group_type = ID2SYM( rb_intern( "type" ) );
  uint8_t group_type = OFPGT_ALL;
  VALUE r_group_type = rb_hash_aref( options, sym_group_type );
  if ( r_group_type != Qnil ) {
    group_type = ( uint8_t ) NUM2UINT( r_group_type );
  }

  VALUE sym_group_id = ID2SYM( rb_intern( "group_id" ) );
  uint32_t group_id = 0;
  VALUE r_group_id = rb_hash_aref( options, sym_group_id );
  if ( r_group_id != Qnil ) {
    group_id = ( uint32_t ) NUM2UINT( r_group_id );
  }

  VALUE sym_command = ID2SYM( rb_intern( "command" ) );
  uint16_t command = OFPGC_ADD;
  VALUE r_command = rb_hash_aref( options, sym_command );
  if ( r_command != Qnil ) {
    command = ( uint16_t ) NUM2UINT( r_command );
  }

  VALUE sym_buckets = ID2SYM( rb_intern( "buckets" ) );
  openflow_buckets *buckets = NULL;
  VALUE r_bucket = rb_hash_aref( options, sym_buckets );
  if ( r_bucket != Qnil ) {
    buckets = pack_buckets( r_bucket );
  }
  buffer *group_mod = create_group_mod( xid, command, group_type,
                                        group_id, buckets );
  if ( buckets != NULL ) {
    delete_buckets( buckets );
  }
  return group_mod;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
