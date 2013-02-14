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


extern VALUE mActions;


static VALUE sym_port_number;
static VALUE sym_max_len;
static VALUE sym_group_id;
static VALUE sym_mpls_ttl;
static VALUE sym_ether_type;
static VALUE sym_queue_id;
static VALUE sym_ip_ttl;
static VALUE sym_experimenter;
static VALUE sym_body;


static openflow_actions *
openflow_actions_ptr( VALUE self ) {
  openflow_actions *actions;
  Data_Get_Struct( self, openflow_actions, actions );
  return actions;
}


static VALUE
pack_send_out_port( VALUE self, VALUE actions, VALUE options ) {
  VALUE r_port_number = rb_hash_aref( options, sym_port_number );
  VALUE r_max_len = rb_hash_aref( options, sym_max_len );
  append_action_output( openflow_actions_ptr( actions ), ( const uint32_t ) NUM2UINT( r_port_number ), ( const uint16_t ) NUM2UINT( r_max_len ) );
  return self;
}


static VALUE
pack_group_action( VALUE self, VALUE actions, VALUE options ) {
  VALUE group_id = rb_hash_aref( options, sym_group_id );
  append_action_group( openflow_actions_ptr( actions ), NUM2UINT( group_id ) );
  return self;
}


static VALUE
pack_copy_ttl_in( VALUE self, VALUE actions, VALUE options ) {
  UNUSED( options );
  append_action_copy_ttl_in( openflow_actions_ptr( actions ) );
  return self;
}


static VALUE
pack_copy_ttl_out( VALUE self, VALUE actions, VALUE options ) {
  UNUSED( options );
  append_action_copy_ttl_out( openflow_actions_ptr( actions ) );
  return self;
}


static VALUE
pack_set_mpls_ttl( VALUE self, VALUE actions, VALUE options ) {
  VALUE mpls_ttl = rb_hash_aref( options, sym_mpls_ttl );
  append_action_set_mpls_ttl( openflow_actions_ptr( actions ), ( const uint8_t ) NUM2UINT( mpls_ttl ) );
  return self;
}


static VALUE
append_dec_mpls_ttl( VALUE self, VALUE actions, VALUE options ) {
  UNUSED( options );
  append_action_dec_mpls_ttl( openflow_actions_ptr( actions ) );
  return self;
}


static VALUE
pack_push_vlan( VALUE self, VALUE actions, VALUE options ) {
  VALUE ether_type = rb_hash_aref( options, sym_ether_type );
  append_action_push_vlan( openflow_actions_ptr( actions ), ( const uint16_t ) NUM2UINT( ether_type ) );
  return self;
}


static VALUE
pack_pop_vlan( VALUE self, VALUE actions, VALUE options ) {
  UNUSED( options );
  append_action_pop_vlan( openflow_actions_ptr( actions ) );
  return self;
}


static VALUE
pack_push_mpls( VALUE self, VALUE actions, VALUE options ) {
  VALUE ether_type = rb_hash_aref( options, sym_ether_type );
  append_action_push_mpls( openflow_actions_ptr( actions ), ( const uint16_t ) NUM2UINT( ether_type ) );
  return self;
}


static VALUE
push_pop_mpls( VALUE self, VALUE actions, VALUE options ) {
  VALUE ether_type = rb_hash_aref( options, sym_ether_type );
  append_action_pop_mpls( openflow_actions_ptr( actions ), ( const uint16_t ) NUM2UINT( ether_type ) );
  return self;
}


static VALUE
pack_set_queue( VALUE self, VALUE actions, VALUE options ) {
  VALUE queue_id = rb_hash_aref( options, sym_queue_id );
  append_action_set_queue( openflow_actions_ptr( actions ), NUM2UINT( queue_id ) );
  return self;
}


static VALUE
pack_set_ip_ttl( VALUE self, VALUE actions, VALUE options ) {
  VALUE ip_ttl = rb_hash_aref( options, sym_ip_ttl );
  append_action_set_nw_ttl( openflow_actions_ptr( actions ), ( const uint8_t ) NUM2UINT( ip_ttl ) );
  return self;
}


static VALUE
pack_push_pbb( VALUE self, VALUE actions, VALUE options ) {
  VALUE ether_type = rb_hash_aref( options, sym_ether_type );
  append_action_push_pbb( openflow_actions_ptr( actions ), ( const uint16_t ) NUM2UINT( ether_type ) );
  return self;
}


static VALUE
pack_pop_pbb( VALUE self, VALUE actions, VALUE options ) {
  UNUSED( options );
  append_action_pop_pbb( openflow_actions_ptr( actions ) );
  return self;
}

  
static VALUE
pack_experimenter( VALUE self, VALUE actions, VALUE options ) {
  VALUE experimenter = rb_hash_aref( options, sym_experimenter );
  VALUE body_r = Qnil;

  if ( ( body_r = rb_hash_aref( options, sym_body ) ) != Qnil ) {
    Check_Type( body_r, T_ARRAY );
    uint16_t length = ( uint16_t ) RARRAY_LEN( body_r );
    buffer *body = alloc_buffer_with_length( length );
    void *p = append_back_buffer( body, length );
    for ( int i = 0; i < length; i++ ) {
      ( ( uint8_t * ) p )[ i ] = ( uint8_t ) FIX2INT( RARRAY_PTR( body_r )[ i ] );
    }
    append_action_experimenter( openflow_actions_ptr( actions ), NUM2UINT( experimenter ), body );
    free_buffer( body );
  }
  else {
    append_action_experimenter( openflow_actions_ptr( actions ), NUM2UINT( experimenter ), NULL );
  }
  return self;
}


void
Init_basic_action() {
  sym_port_number = ID2SYM( rb_intern( "port_number" ) );
  sym_max_len = ID2SYM( rb_intern( "max_len" ) );
  sym_group_id = ID2SYM( rb_intern( "group_id" ) );
  sym_mpls_ttl = ID2SYM( rb_intern( "mpls_ttl" ) );
  sym_ether_type = ID2SYM( rb_intern( "ether_type" ) );
  sym_queue_id = ID2SYM( rb_intern( "queue_id" ) );
  sym_ip_ttl = ID2SYM( rb_intern( "ip_ttl" ) );
  sym_experimenter = ID2SYM( rb_intern( "experimenter" ) );
  sym_body = ID2SYM( rb_intern( "body" ) );


  rb_define_module_function( mActions, "pack_send_out_port", pack_send_out_port, 2 );
  rb_define_module_function( mActions, "pack_group_action", pack_group_action, 2 );
  rb_define_module_function( mActions, "pack_copy_ttl_in", pack_copy_ttl_in, 2 );
  rb_define_module_function( mActions, "pack_copy_ttl_out", pack_copy_ttl_out, 2 );
  rb_define_module_function( mActions, "pack_set_mpls_ttl", pack_set_mpls_ttl, 2 );
  rb_define_module_function( mActions, "pack_dec_mpls_ttl", append_dec_mpls_ttl, 2 );
  rb_define_module_function( mActions, "pack_push_vlan", pack_push_vlan, 2 );
  rb_define_module_function( mActions, "pack_pop_vlan", pack_pop_vlan, 2 );
  rb_define_module_function( mActions, "pack_push_mpls", pack_push_mpls, 2 );
  rb_define_module_function( mActions, "pack_pop_mpls", push_pop_mpls, 2 );
  rb_define_module_function( mActions, "pack_set_queue", pack_set_queue, 2 );
  rb_define_module_function( mActions, "pack_set_ip_ttl", pack_set_ip_ttl, 2 );
  rb_define_module_function( mActions, "pack_push_pbb", pack_push_pbb, 2 );
  rb_define_module_function( mActions, "pack_pop_pbb", pack_pop_pbb, 2 );
  rb_define_module_function( mActions, "pack_experimenter", pack_experimenter, 2 );
  rb_require( "trema/basic-action" );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
