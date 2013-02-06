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


statif VALUE
send_flow_rem( VALUE self ) {
  UNUSED( self );
  return UINT2NUM( OFPFF_SEND_FLOW_REM );
}



void
Init_message_const() {
  mMessageConst = rb_define_module_under( mTrema, "MessageConst" );

  rb_define_module_function( mMessageConst, "frag_normal", frag_normal, 0 );
  rb_define_module_function( mMessageConst, "frag_drop", frag_drop, 0 );
  rb_define_module_function( mMessageConst, "frag_reasm", frag_reasm, 0 );
  rb_define_module_function( mMessageConst, "frag_mask", frag_mask, 0 );

  VALUE config_flags = rb_range_new( UINT2NUM( OFPC_FRAG_NORMAL ), UNT2NUM( OFPC_FRAG_MASK ), false );
  rb_iv_set( mMessageConst, "@config_flags", config_flags );


  // ofp_flow_mod_command
  rb_define_const( mMessageConst, "flow_mod_add", flow_mod_add );
  rb_define_const( mMessageConst, "flow_mod_modify", flow_mod_modify );
  rb_define_const( mMessageConst, "flow_mod_modify_strict", flow_mod_modify_strict );
  rb_define_const( mMessageConst, "flow_mod_delete", flow_mod_delete );
  rb_define_const( mMessageConst, "flow_mod_delete_strict", flow_mod_delete_strict );

  // ofp_flow_mod_flags
  rb_define_const( mMessageConst, "send_flow_rem", send_flow_rem );
  rb_define_const( mMessageConst, "OFPFF_CHECK_OVERLAP", INT2NUM( OFPFF_CHECK_OVERLAP ) );
  rb_define_const( mMessageConst, "OFPFF_RESET_COUNTS", INT2NUM( OFPFF_RESET_COUNTS ) );
  rb_define_const( mMessageConst, "OFPFF_NO_PKT_COUNTS", INT2NUM( OFPFF_NO_PKT_COUNTS ) );
  rb_define_const( mMessageConst, "OFPFF_NO_BYT_COUNTS", INT2NUM( OFPFF_NO_BYT_COUNTS ) );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
