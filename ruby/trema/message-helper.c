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
      case T_ARRAY: {
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



static VALUE
send_packet_out( int argc, VALUE *argv, VALUE self ) {
  VALUE datapath_id = Qnil;
  VALUE options = Qnil;
  rb_scan_args( argc, argv, "11", &datapath_id, &options );

  uint32_t buffer_id = OFP_NO_BUFFER;
  buffer *data = NULL;
  buffer *allocated_data = NULL;
  openflow_actions *actions = NULL;
  uint32_t in_port = OFPP_ANY;
  VALUE opt_zero_padding = Qnil;

  if ( options != Qnil ) {
    VALUE opt_message = rb_hash_aref( options, ID2SYM( rb_intern( "packet_in" ) ) );

    VALUE opt_action = rb_hash_aref( options, ID2SYM( rb_intern( "actions" ) ) );
    if ( opt_action != Qnil ) {
      actions = pack_basic_action( opt_action );
    }

    opt_zero_padding = rb_hash_aref( options, ID2SYM( rb_intern( "zero_padding" ) ) );
    if ( opt_zero_padding != Qnil ) {
      if ( TYPE( opt_zero_padding ) != T_TRUE && TYPE( opt_zero_padding ) != T_FALSE ) {
        rb_raise( rb_eTypeError, ":zero_padding must be true or false" );
      }
    }

    if ( opt_message != Qnil ) {

      if ( datapath_id == rb_iv_get( opt_message, "@datapath_id" ) ) {
        buffer_id = NUM2UINT( rb_iv_get( opt_message, "@buffer_id" ) );
        VALUE match = rb_iv_get( opt_message, "@match" );
        in_port = NUM2UINT( rb_iv_get( match, "@in_port" ) );
      }
       
      VALUE data_ary = rb_iv_get( opt_message, "@data" );
      if ( data_ary != Qnil ) {
        Check_Type( data_ary, T_ARRAY );
        uint32_t length = ( uint32_t ) RARRAY_LEN( data_ary );

        data = alloc_buffer_with_length( length );
        append_back_buffer( data, length );
        uint8_t *data_ptr = data->data;
        for ( uint32_t i = 0; i < length; i++ ) {
          data_ptr[ i ] = ( uint8_t ) FIX2INT( RARRAY_PTR( data_ary ) [ i ] );
        }
      }
    }


    if ( data != NULL && data->length + ETH_FCS_LENGTH < ETH_MINIMUM_LENGTH &&
      opt_zero_padding != Qnil && TYPE( opt_zero_padding ) == T_TRUE ) {
      if ( allocated_data == NULL ) {
        allocated_data = duplicate_buffer( data );
        data = allocated_data;
      }
      fill_ether_padding( allocated_data );
    }

    buffer *packet_out = create_packet_out(
      get_transaction_id(),
      buffer_id, 
      in_port,
      actions,
      data
    );
    send_openflow_message( NUM2ULL( datapath_id ), packet_out );

    if ( allocated_data != NULL ) {
      free_buffer( allocated_data );
    }
    free_buffer( packet_out );
    delete_actions( actions );
  }
  return self;
}


void
Init_message_helper() {
  mMessageHelper = rb_define_module_under( mTrema, "MessageHelper" );

  rb_define_module_function( mMessageHelper, "send_flow_mod", send_flow_mod, -1 );
  rb_define_module_function( mMessageHelper, "send_message", send_controller_message, 2 );
  rb_define_module_function( mMessageHelper, "send_packet_out", send_packet_out, - 1 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
