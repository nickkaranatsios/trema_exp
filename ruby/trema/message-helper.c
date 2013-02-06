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


static VALUE
get_config_flags( VALUE self ) {
  return rb_iv_get( self, "@config_flags" );
}


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

  rb_define_module_function( mMessageHelper, "config_flags", get_config_flags, 0 );
  rb_define_module_function( mMessageHelper, "send_flow_mod", send_flow_mod, -1 );
  rb_define_module_function( mMessageHelper, "send_message", send_controller_message, 2 );

  rb_define_const( mMessageHelper, "OFPC_FRAG_NORMAL", INT2NUM( OFPC_FRAG_NORMAL ) );
  rb_define_const( mMessageHelper, "OFPC_FRAG_DROP", INT2NUM( OFPC_FRAG_DROP ) );
  rb_define_const( mMessageHelper, "OFPC_FRAG_REASM", INT2NUM( OFPC_FRAG_REASM ) );
  rb_define_const( mMessageHelper, "OFPC_FRAG_MASK", INT2NUM( OFPC_FRAG_MASK ) );

  VALUE config_flags = rb_range_new( INT2NUM( OFPC_FRAG_NORMAL ), INT2NUM( OFPC_FRAG_MASK ), false );
  rb_iv_set( mMessageHelper, "@config_flags", config_flags );

  // ofp_flow_mod_command
  rb_define_const( mMessageHelper, "OFPFC_ADD", UINT2NUM( OFPFC_ADD ) );
  rb_define_const( mMessageHelper, "OFPFC_MODIFY", UINT2NUM( OFPFC_MODIFY ) );
  rb_define_const( mMessageHelper, "OFPFC_MODIFY_STRICT", UINT2NUM( OFPFC_MODIFY_STRICT ) );
  rb_define_const( mMessageHelper, "OFPFC_DELETE", UINT2NUM( OFPFC_DELETE ) );
  rb_define_const( mMessageHelper, "OFPFC_DELETE_STRICT", UINT2NUM( OFPFC_DELETE_STRICT ) );


  // TODO to review all constants
  // ofp_flow_mod_flags constants
  rb_define_const( mMessageHelper, "OFPFF_SEND_FLOW_REM", INT2NUM( OFPFF_SEND_FLOW_REM ) );
  rb_define_const( mMessageHelper, "OFPFF_CHECK_OVERLAP", INT2NUM( OFPFF_CHECK_OVERLAP ) );
  rb_define_const( mMessageHelper, "OFPFF_RESET_COUNTS", INT2NUM( OFPFF_RESET_COUNTS ) );
  rb_define_const( mMessageHelper, "OFPFF_NO_PKT_COUNTS", INT2NUM( OFPFF_NO_PKT_COUNTS ) );
  rb_define_const( mMessageHelper, "OFPFF_NO_BYT_COUNTS", INT2NUM( OFPFF_NO_BYT_COUNTS ) );
  VALUE flow_mod_flags = rb_range_new( INT2NUM( OFPFF_SEND_FLOW_REM ), INT2NUM( OFPFF_NO_BYT_COUNTS ), false  );
  rb_iv_set( mMessageHelper, "@flow_mod_flags", flow_mod_flags );


  rb_define_const( mMessageHelper, "OFPP_CONTROLLER", UINT2NUM( OFPP_CONTROLLER ) );

  rb_define_const( mMessageHelper, "OFP_NO_BUFFER", UINT2NUM( OFP_NO_BUFFER ) );
  rb_define_const( mMessageHelper, "OFPCML_MAX", INT2NUM( OFPCML_MAX ) );
  rb_define_const( mMessageHelper, "OFPCML_NO_BUFFER", INT2NUM( OFPCML_NO_BUFFER ) );


  rb_define_const( mMessageHelper, "OFP_DEFAULT_PRIORITY", INT2NUM( OFP_DEFAULT_PRIORITY ) );
  rb_define_const( mMessageHelper, "OFP_HIGH_PRIORITY", INT2NUM( OFP_HIGH_PRIORITY ) );
  rb_define_const( mMessageHelper, "OFP_LOW_PRIORITY", INT2NUM( OFP_LOW_PRIORITY ) );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
