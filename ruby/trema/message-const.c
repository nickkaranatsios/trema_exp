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


extern VALUE mTrema;
VALUE mMessageConst;


static VALUE
frag_normal( VALUE self ) {
  UNUSED( self );
  return UINT2NUM( OFPC_FRAG_NORMAL );
}


static VALUE
frag_drop( VALUE self ) {
  UNUSED( self );
  return UINT2NUM( OFPC_FRAG_DROP );
}


static VALUE
frag_reasm( VALUE self ) {
  UNUSED( self );
  return UINT2NUM( OFPC_FRAG_REASM );
}


static VALUE
frag_mask( VALUE self ) {
  UNUSED( self );
  return UINT2NUM( OFPC_FRAG_MASK );
}


static VALUE
flow_mod_add( VALUE self ) {
  UNUSED( self );
  return UINT2NUM( OFPFC_ADD );
}


static VALUE
flow_mod_modify( VALUE self ) {
  UNUSED( self );
  return UINT2NUM( OFPFC_MODIFY );
}


static VALUE
flow_mod_modify_strict( VALUE self ) {
  UNUSED( self );
  return UINT2NUM( OFPFC_MODIFY_STRICT );
}


static VALUE
flow_mod_delete( VALUE self ) {
  UNUSED( self );
  return UINT2NUM( OFPFC_DELETE );
}


static VALUE
flow_mod_delete_strict( VALUE self ) {
  UNUSED( self );
  return UINT2NUM( OFPFC_DELETE_STRICT );
}


static VALUE
send_flow_rem( VALUE self ) {
  UNUSED( self );
  return UINT2NUM( OFPFF_SEND_FLOW_REM );
}


static VALUE
check_overlap( VALUE self ) {
  UNUSED( self );
  return UINT2NUM( OFPFF_CHECK_OVERLAP );
}


static VALUE
reset_counts( VALUE self ) {
  UNUSED( self );
  return UINT2NUM( OFPFF_RESET_COUNTS );
}


static VALUE
no_pkt_counts( VALUE self ) {
  UNUSED( self );
  return UINT2NUM( OFPFF_NO_PKT_COUNTS );
}


static VALUE
no_byt_counts( VALUE self ) {
  UNUSED( self );
  return UINT2NUM( OFPFF_NO_BYT_COUNTS );
}


static VALUE
controller_port( VALUE self ) {
  UNUSED( self );
  return UINT2NUM( OFPP_CONTROLLER );
}


static VALUE
max_port( VALUE self ) {
  UNUSED( self );
  return UINT2NUM( OFPP_MAX );
}


static VALUE
in_port( VALUE self ) {
  UNUSED( self );
  return UINT2NUM( OFPP_IN_PORT );
}


static VALUE
table_port( VALUE self ) {
  UNUSED( self );
  return UINT2NUM( OFPP_TABLE );
}


static VALUE
normal_port( VALUE self ) {
  UNUSED( self );
  return UINT2NUM( OFPP_NORMAL );
}


static VALUE
flood_port( VALUE self ) {
  UNUSED( self );
  return UINT2NUM( OFPP_FLOOD );
}


static VALUE
all_ports( VALUE self ) {
  UNUSED( self );
  return UINT2NUM( OFPP_ALL );
}


static VALUE
local_port( VALUE self ) {
  UNUSED( self );
  return UINT2NUM( OFPP_LOCAL );
}


static VALUE
any_port( VALUE self ) {
  UNUSED( self );
  return UINT2NUM( OFPP_ANY );
}


static VALUE
controller_max_len_max( VALUE self ) {
  UNUSED( self );
  return UINT2NUM( OFPCML_MAX );
}


static VALUE
controller_max_len_no_buffer( VALUE self ) {
  UNUSED( self );
  return UINT2NUM( OFPCML_NO_BUFFER );
}


static VALUE
no_buffer( VALUE self ) {
  UNUSED( self );
  return UINT2NUM( OFP_NO_BUFFER );
}


static VALUE
default_priority( VALUE self ) {
  UNUSED( self );
  return UINT2NUM( OFP_DEFAULT_PRIORITY );
}


static VALUE
high_priority( VALUE self ) {
  UNUSED( self );
  return UINT2NUM( OFP_HIGH_PRIORITY );
}


static VALUE
low_priority( VALUE self ) {
  UNUSED( self );
  return UINT2NUM( OFP_LOW_PRIORITY );
}


void
Init_message_const() {
  mMessageConst = rb_define_module_under( mTrema, "MessageConst" );

  rb_define_module_function( mMessageConst, "frag_normal", frag_normal, 0 );
  rb_define_module_function( mMessageConst, "frag_drop", frag_drop, 0 );
  rb_define_module_function( mMessageConst, "frag_reasm", frag_reasm, 0 );
  rb_define_module_function( mMessageConst, "frag_mask", frag_mask, 0 );

  VALUE config_flags = rb_range_new( UINT2NUM( OFPC_FRAG_NORMAL ), UINT2NUM( OFPC_FRAG_MASK ), false );
  rb_iv_set( mMessageConst, "@config_flags", config_flags );


  // ofp_flow_mod_command
  rb_define_module_function( mMessageConst, "flow_mod_add", flow_mod_add, 0 );
  rb_define_module_function( mMessageConst, "flow_mod_modify", flow_mod_modify, 0 );
  rb_define_module_function( mMessageConst, "flow_mod_modify_strict", flow_mod_modify_strict, 0 );
  rb_define_module_function( mMessageConst, "flow_mod_delete", flow_mod_delete, 0 );
  rb_define_module_function( mMessageConst, "flow_mod_delete_strict", flow_mod_delete_strict, 0 );

  // ofp_flow_mod_flags
  rb_define_module_function( mMessageConst, "send_flow_rem", send_flow_rem, 0 );
  rb_define_module_function( mMessageConst, "check_overlap", check_overlap, 0 );
  rb_define_module_function( mMessageConst, "reset_counts", reset_counts, 0 );
  rb_define_module_function( mMessageConst, "no_pkt_counts", no_pkt_counts, 0 );
  rb_define_module_function( mMessageConst, "no_byt_counts", no_byt_counts, 0 );

  // ofp_ports
  rb_define_module_function( mMessageConst, "max_port", max_port, 0 );
  rb_define_module_function( mMessageConst, "in_port", in_port, 0 );
  rb_define_module_function( mMessageConst, "table_port", table_port, 0 );
  rb_define_module_function( mMessageConst, "normal_port", normal_port, 0 );
  rb_define_module_function( mMessageConst, "flood_port", flood_port, 0 );
  rb_define_module_function( mMessageConst, "all_ports", all_ports, 0 );
  rb_define_module_function( mMessageConst, "controller_port", controller_port, 0 );
  rb_define_module_function( mMessageConst, "local_port", local_port, 0 );
  rb_define_module_function( mMessageConst, "any_port", any_port, 0 );

  // controller_max_length
  rb_define_module_function( mMessageConst, "controller_max_len_max", controller_max_len_max, 0 );
  rb_define_module_function( mMessageConst, "controller_max_len_no_buffer", controller_max_len_no_buffer, 0 );

  // OFP_NO_BUFFER constant
  rb_define_module_function( mMessageConst, "no_buffer", no_buffer, 0 );

  // OFP_PRIORITY values
  rb_define_module_function( mMessageConst, "default_priority", default_priority, 0 );
  rb_define_module_function( mMessageConst, "high_priority", high_priority, 0 );
  rb_define_module_function( mMessageConst, "low_priority", low_priority, 0 );

  rb_require( "trema/message-const" );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
