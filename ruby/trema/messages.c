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
#include "messages/hello.h"
#include "messages/echo-request.h"
#include "messages/features-request.h"
#include "messages/get-config-request.h"
#include "messages/set-config.h"
#include "messages/flow-mod.h"


extern VALUE mTrema;
VALUE mMessages;


static uint64_t
datapath_id( VALUE options ) {
  VALUE sym_datapath_id = ID2SYM( rb_intern( "datapath_id" ) );
  VALUE dpid_ruby = rb_hash_aref( options, sym_datapath_id );
  return NUM2ULL( dpid_ruby );
}


static void
send_msg( uint64_t datapath_id, buffer *msg ) {
  send_openflow_message( datapath_id, msg );
}


static VALUE
pack_hello_msg( VALUE self, VALUE options ) {
  buffer *msg = pack_hello( options );
  send_msg( datapath_id( options ), msg );
  return self;
}


static VALUE
pack_echo_request_msg( VALUE self, VALUE options ) {
  buffer *msg = pack_echo_request( options );
  send_msg( datapath_id( options ), msg );
  return self;
}


static VALUE
pack_features_request_msg( VALUE self, VALUE options ) {
  buffer *msg = pack_features_request( options );
  send_msg( datapath_id( options ), msg );
  return self;
}


static VALUE
pack_get_config_request_msg( VALUE self, VALUE options ) {
  buffer *msg = pack_get_config_request( options );
  send_msg( datapath_id( options ), msg );
  return self;
}


static VALUE
pack_set_config_msg( VALUE self, VALUE options ) {
  buffer *msg = pack_set_config( options );
  send_msg( datapath_id( options ), msg );
  return self;
}


static VALUE
pack_flow_mod_msg( VALUE self, VALUE options ) {
  buffer *msg = pack_flow_mod( options );
  send_msg( datapath_id( options ), msg );
  return self;
}


void
Init_messages() {
  mMessages = rb_define_module_under( mTrema, "Messages" );
  rb_define_module_function( mMessages, "pack_hello_msg", pack_hello_msg, 1 );
  rb_define_module_function( mMessages, "pack_echo_request_msg", pack_echo_request_msg, 1 );
  rb_define_module_function( mMessages, "pack_features_request_msg", pack_features_request_msg, 1 );
  rb_define_module_function( mMessages, "pack_get_config_request_msg", pack_get_config_request_msg, 1 );
  rb_define_module_function( mMessages, "pack_set_config_msg", pack_set_config_msg, 1 );
  rb_define_module_function( mMessages, "pack_flow_mod_msg", pack_flow_mod_msg, 1 );

  rb_require( "trema/messages" );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
