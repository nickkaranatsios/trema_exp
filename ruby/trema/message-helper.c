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
VALUE mMessageHelper;


/*
 * @overload send_message(datapath_id, message)
 *   Sends an OpenFlow message to the datapath.
 *
 *   @example
 *     send_message datapath_id, FeaturesRequest.new
 *
 *
 *   @param [Number] datapath_id
 *     the datapath to which a message is sent.
 *
 *   @param [FeaturesRequest] message
 *     the message to be sent.
 */
static VALUE
send_controller_message( VALUE self, VALUE datapath_id, VALUE message ) {
  VALUE id_pack_msg = rb_intern( "pack_msg" );

  if ( message != Qnil ) {
    switch ( TYPE( message ) ) {
      case T_ARRAY:
      {
          VALUE *each = RARRAY_PTR( message );
          
          for ( int i = 0; i < RARRAY_LEN( message ); i++ ) {
            if ( rb_respond_to( each[ i ], id_pack_msg ) ) {
              rb_funcall( each[ i ], id_pack_msg, 1, datapath_id );
            }
          }
      }
      break;
      case T_OBJECT:
        if ( rb_respond_to( message, id_pack_msg ) ) {
          rb_funcall( message, id_pack_msg, 1, datapath_id );
        }
      break;
      default:
        rb_raise( rb_eTypeError, "Message argument must be an Array or a Message object" );
      break;
    }
  }
  return self;
}


static VALUE
send_flow_mod( int argc, VALUE *argv, VALUE self ) {
  VALUE datapath_id = Qnil;
  VALUE options = Qnil;
  rb_scan_args( argc, argv, "11", &datapath_id, &options );

  if ( options != Qnil ) {
    VALUE flow_mod = rb_funcall( rb_eval_string( "Messages::FlowMod" ), rb_intern( "new" ), 1, options );

    // the two lines below are for temporary debug
    VALUE str = rb_inspect( flow_mod );
    printf( "flow_mod %s\n", StringValuePtr( str ) );
    
    send_controller_message( self, datapath_id, flow_mod );
  }

  return self;
}


void
Init_message_helper() {
  mMessageHelper = rb_define_module_under( mTrema, "MessageHelper" );

  rb_define_module_function( mMessageHelper, "send_flow_mod", send_flow_mod, -1 );
  rb_define_module_function( mMessageHelper, "send_message", send_controller_message, 2 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
