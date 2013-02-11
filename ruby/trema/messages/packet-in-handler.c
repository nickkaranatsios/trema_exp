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
#include "oxm-helper.h"
#include "hash-util.h"


#define PACKET_INFO_MAC_ADDR( packet_member )                                          \
  {                                                                                    \
    VALUE ret = ULL2NUM( mac_to_uint64( ( ( packet_info * ) ( message->data->user_data ) )->packet_member ) ); \
    return rb_funcall( rb_eval_string( "Trema::Mac" ), rb_intern( "new" ), 1, ret );   \
  }


static packet_info *
get_packet_in_info( packet_in *message ) {
  return ( packet_info * ) ( message->data->user_data );
}




static VALUE
packet_in_match( packet_in *message ) {
  VALUE options = rb_hash_new();

  for ( list_element *list = message->match->list; list != NULL; list = list->next ) {
    oxm_match_header *oxm = list->data;
    assign_match( oxm, options );
  }
  VALUE match = rb_funcall( rb_eval_string( "Messages::Match" ), rb_intern( "new" ), 1, options );
  return match;
}


static VALUE
packet_in_data( packet_in *message ) {
  const buffer *data_frame = message->data;
  uint16_t length = ( uint16_t ) data_frame->length;

  if ( data_frame != NULL ) {
    if ( data_frame->length ) {
      VALUE data_array = rb_ary_new2( ( long int ) length );
      uint8_t *data = ( uint8_t * ) ( ( char * ) data_frame->data );
      long i;
      for ( i = 0; i < length; i++ ) {
        rb_ary_push( data_array, INT2FIX( data[ i ] ) );
      }
      return data_array;
    }
  }
  return Qnil;
}


static VALUE
packet_in_eth_type( packet_in *message ) {
  return UINT2NUM( get_packet_in_info( message )->eth_type );
}


static VALUE
packet_in_macsa( packet_in *message ) {
 PACKET_INFO_MAC_ADDR( eth_macsa );
}


static VALUE
packet_in_macda( packet_in *message ) {
  PACKET_INFO_MAC_ADDR( eth_macda );
}


static VALUE
packet_in_vtag( packet_in *message ) {
  if ( get_packet_in_info( message )->format & ETH_8021Q ) {
    return Qtrue;
  }
  return Qfalse;
}


/*
 * Is it an ARP packet?
 *
 * @return [Boolean] whether the packet is an ARP packet or not.
 */
static VALUE
packet_in_is_arp( packet_in *message ) {
  if ( ( get_packet_in_info( message )->format & NW_ARP ) ) {
    return Qtrue;
  }
  else {
    return Qfalse;
  }
}


static VALUE
decode_packet_in( packet_in *message ) {
  VALUE attributes = rb_hash_new();

  HASH_SET( attributes, "datapath_id", ULL2NUM( message->datapath_id ) );  
  HASH_SET( attributes, "transaction", UINT2NUM( message->transaction_id ) );
  HASH_SET( attributes, "buffer_id", UINT2NUM( message->buffer_id ) );
  HASH_SET( attributes, "total_len", UINT2NUM( message->total_len ) );
  HASH_SET( attributes, "reason", UINT2NUM( message->reason ) );
  HASH_SET( attributes, "table_id", UINT2NUM( message->table_id ) );
  HASH_SET( attributes, "cookie", ULL2NUM( message->cookie ) );
  HASH_SET( attributes, "match", packet_in_match( message ) );
  HASH_SET( attributes, "data", packet_in_data( message ) );
  // packet_info information
  HASH_SET( attributes, "eth_type", packet_in_eth_type( message ) );
  HASH_SET( attributes, "macsa", packet_in_macsa( message ) );
  HASH_SET( attributes, "macda", packet_in_macda( message ) );
  HASH_SET( attributes, "vtag", packet_in_vtag( message ) );
  HASH_SET( attributes, "arp", packet_in_is_arp( message ) );
  return rb_funcall( rb_eval_string( "Messages::PacketIn" ), rb_intern( "new" ), 1, attributes );
}


/*
 * Handler called when +OFPT_PACKET_IN+ message is received.
 */
void
handle_packet_in( uint64_t datapath_id, packet_in message ) {
  VALUE controller = ( VALUE ) message.user_data;
  if ( !rb_respond_to( controller, rb_intern( "packet_in" ) ) ) {
    return;
  }

  VALUE cPacketIn = decode_packet_in( &message );
  rb_funcall( controller, rb_intern( "packet_in" ), 2, ULL2NUM( datapath_id ), cPacketIn );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
