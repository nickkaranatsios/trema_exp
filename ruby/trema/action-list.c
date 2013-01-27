/*
 * Copyright (C) 2008-2012 NEC Corporation
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


extern VALUE mTrema;
VALUE mActionList;


static openflow_actions *
openflow_actions_ptr( VALUE self ) {
  openflow_actions *actions;
  Data_Get_Struct( self, openflow_actions, actions );
  return actions;
}


static VALUE
append_output( VALUE self, VALUE r_actions, VALUE options ) {
  VALUE port_number = rb_hash_aref( options, ID2SYM( rb_intern( "port_number" ) ) );
  VALUE max_len = rb_hash_aref( options, ID2SYM( rb_intern( "max_len" ) ) );
printf( "port(%u) max_len(%u)\n", NUM2UINT(port_number), NUM2UINT(max_len) );
  append_action_output( openflow_actions_ptr( r_actions ), ( const uint16_t ) NUM2UINT( port_number ), ( const uint16_t ) NUM2UINT( max_len ) );
  return self;
}


static VALUE
append_group( VALUE self, VALUE r_actions, VALUE options ) {
  VALUE group_id = rb_hash_aref( options, ID2SYM( rb_intern( "group_id" ) ) );
  append_action_group( openflow_actions_ptr( r_actions ), NUM2UINT( group_id ) );
  return self;
}


static VALUE
append_copy_ttl_in( VALUE self, VALUE r_actions, VALUE options ) {
  UNUSED( options );
  append_action_copy_ttl_in( openflow_actions_ptr( r_actions ) );
  return self;
}


void
Init_action_list() {
  mActionList = rb_define_module_under( mTrema, "ActionList" );
  rb_define_module_function( mActionList, "append_send_out_port", append_output, 2 );
  rb_define_module_function( mActionList, "append_group_action", append_group, 2 );
  rb_define_module_function( mActionList, "append_copy_ttl_in", append_copy_ttl_in, 2 );
  rb_require( "trema/send-out-port" );
  rb_require( "trema/group-action" );
  rb_require( "trema/copy-ttl-in" );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
