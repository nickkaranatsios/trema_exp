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


#include <assert.h>
#include "trema.h"
#include "ruby.h"
#include "hash-util.h"
#include "conversion-util.h"


#define PACKET_INFO_MAC_ADDR( packet_member )                                          \
  {                                                                                    \
    VALUE ret = ULL2NUM( mac_to_uint64( ( ( packet_info * ) ( frame->user_data ) )->packet_member ) ); \
    return rb_funcall( rb_eval_string( "Trema::Mac" ), rb_intern( "new" ), 1, ret );   \
  }


#define PACKET_INFO_IPv4_ADDR( packet_member )                                          \
  {                                                                                   \
    VALUE ret = UINT2NUM( ( ( packet_info * ) ( frame->user_data ) )->packet_member ); \
    return rb_funcall( rb_eval_string( "IPAddr" ), rb_intern( "new" ), 2, ret, rb_eval_string( "Socket::AF_INET" )  );   \
  }


#define PACKET_INFO_IPv6_ADDR( packet_member ) \
  { \
      char ipv6_str[ INET6_ADDRSTRLEN ]; \
      memset( ipv6_str, '\0', sizeof( ipv6_str ) ); \
      if ( inet_ntop( AF_INET6, &( ( packet_info * ) ( frame->user_data ) )->packet_member, ipv6_str, sizeof( ipv6_str ) ) != NULL ) { \
        return rb_funcall( rb_eval_string( "IPAddr" ), rb_intern( "new" ), 1, rb_str_new2( ipv6_str ) ); \
      } \
      return Qnil; \
  }


static packet_info *
get_packet_in_info( const buffer *frame ) {
  return ( packet_info * ) ( frame->user_data );
}




static VALUE
packet_in_match( packet_in *message ) {
  return oxm_match_to_r_match( message->match );
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
packet_in_eth_type( const buffer *frame ) {
  return UINT2NUM( get_packet_in_info( frame )->eth_type );
}


static VALUE
packet_in_macsa( const buffer *frame ) {
  PACKET_INFO_MAC_ADDR( eth_macsa )
}


static VALUE
packet_in_macda( const buffer *frame ) {
  PACKET_INFO_MAC_ADDR( eth_macda )
}


static VALUE
packet_in_vlan_vid( const buffer *frame ) {
  if ( packet_type_eth_vtag( frame ) ) {
    return UINT2NUM( get_packet_in_info( frame )->vlan_vid );
  }
  return Qnil;
}


static VALUE
packet_in_vlan_prio( const buffer *frame ) {
  if ( packet_type_eth_vtag( frame ) ) {
    return UINT2NUM( get_packet_in_info( frame )->vlan_prio );
  }
  return Qnil;
}


static VALUE
packet_in_vlan_cfi( const buffer *frame ) {
  if ( packet_type_eth_vtag( frame ) ) {
    return UINT2NUM( get_packet_in_info( frame )->vlan_cfi );
  }
  return Qnil;
}


static VALUE
packet_in_vlan_tci( const buffer *frame ) {
  if ( packet_type_eth_vtag( frame ) ) {
    return UINT2NUM( get_packet_in_info( frame )->vlan_tci );
  }
  return Qnil;
}


static VALUE
packet_in_vlan_tpid( const buffer *frame ) {
  if ( packet_type_eth_vtag( frame ) ) {
    return UINT2NUM( get_packet_in_info( frame )->vlan_tpid );
  }
  return Qnil;
}


static VALUE
packet_in_arp_op( const buffer *frame ) {
  if ( packet_type_arp( frame ) ) {
    return UINT2NUM( get_packet_in_info( frame )->arp_ar_op );
  }
  return Qnil;
}


static VALUE
packet_in_arp_sha( const buffer *frame ) {
  if ( packet_type_arp( frame ) ) {
    PACKET_INFO_MAC_ADDR( arp_sha )
  }
  return Qnil;
}


static VALUE
packet_in_arp_spa( const buffer *frame ) {
  if ( packet_type_arp( frame ) ) {
    PACKET_INFO_IPv4_ADDR( arp_spa )
  }
  return Qnil;
}


static VALUE
packet_in_arp_tha( const buffer *frame ) {
  if ( packet_type_arp( frame ) ) {
    PACKET_INFO_MAC_ADDR( arp_tha )
  }
  return Qnil;
}


static VALUE
packet_in_arp_tpa( const buffer *frame ) {
  if ( packet_type_arp( frame ) ) {
    PACKET_INFO_IPv4_ADDR( arp_tpa )
  }
  return Qnil;
}


static VALUE
packet_in_ipv6_src( const buffer *frame ) {
  if ( packet_type_ipv6( frame ) ) {
    PACKET_INFO_IPv6_ADDR( ipv6_saddr )
  }
  return Qnil;
}


static VALUE
packet_in_ipv6_dst( const buffer *frame ) {
  if ( packet_type_ipv6( frame ) ) {
    PACKET_INFO_IPv6_ADDR( ipv6_daddr )
  }
  return Qnil;
}


static VALUE
packet_in_ipv6_flabel( const buffer *frame ) {
  if ( packet_type_ipv6( frame ) ) {
     return UINT2NUM( get_packet_in_info( frame )->ipv6_flowlabel );
  }
  return Qnil;
}


static VALUE
packet_in_icmpv4( const buffer *frame ) {
  if ( packet_type_icmpv4( frame ) ) {
    return Qtrue;
  }
  return Qfalse;
}


static VALUE
packet_in_icmpv6( const buffer *frame ) {
  if ( packet_type_icmpv6( frame ) ) {
    return Qtrue;
  }
  return Qtrue;
}


static VALUE
packet_in_ip_dscp( const buffer *frame ) {
  if ( packet_type_ipv4( frame ) ) {
    return UINT2NUM( get_packet_in_info( frame )->ip_dscp );
  }
  return Qnil;
}


static VALUE
packet_in_arp( const buffer *frame ) {
  if ( packet_type_arp( frame ) ) {
    return Qtrue;
  }
  return Qfalse;
}


static VALUE
packet_in_vtag( const buffer *frame ) {
  if ( packet_type_eth_vtag( frame ) ) {
    return Qtrue;
  }
  return Qfalse;
}


static VALUE
packet_in_ipv4( const buffer *frame ) {
  if ( packet_type_ipv4( frame ) ) {
    return Qtrue;
  }
  return Qfalse;
}


static VALUE
packet_in_igmp( const buffer *frame ) {
  if ( packet_type_igmp( frame ) ) {
    return Qtrue;
  }
  return Qfalse;
}


static VALUE
packet_in_ipv4_tcp( const buffer *frame ) {
  if ( packet_type_ipv4_tcp( frame ) ) {
    return Qtrue;
  }
  return Qfalse;
}


static VALUE
packet_in_ipv4_udp( const buffer *frame ) {
  if ( packet_type_ipv4_udp( frame ) ) {
    return Qtrue;
  }
  return Qfalse;
}


static VALUE
packet_in_ipv6_tcp( const buffer *frame ) {
  if ( packet_type_ipv6_tcp( frame ) ) {
    return Qtrue;
  }
  return Qfalse;
}


static VALUE
packet_in_ipv6_udp( const buffer *frame ) {
  if ( packet_type_ipv6_udp( frame ) ) {
    return Qtrue;
  }
  return Qfalse;
}


static VALUE
packet_in_arp_request( const buffer *frame ) {
  if ( packet_type_arp_request( frame ) ) {
    return Qtrue;
  }
  return Qfalse;
}


static VALUE
packet_in_arp_reply( const buffer *frame ) {
  if ( packet_type_arp_reply( frame ) ) {
    return Qtrue;
  }
  return Qfalse;
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
  VALUE pi_attributes = rb_hash_new();
  assert( message->data );
  HASH_SET( pi_attributes, "eth_type", packet_in_eth_type( message->data ) );
  HASH_SET( pi_attributes, "macsa", packet_in_macsa( message->data ) );
  HASH_SET( pi_attributes, "macda", packet_in_macda( message->data ) );

  HASH_SET( pi_attributes, "ipv4", packet_in_ipv4( message->data ) );
  HASH_SET( pi_attributes, "ipv4_tcp", packet_in_ipv4_tcp( message->data ) );
  HASH_SET( pi_attributes, "ipv4_udp", packet_in_ipv4_udp( message->data ) );
  HASH_SET( pi_attributes, "ipv6_tcp", packet_in_ipv6_tcp( message->data ) );
  HASH_SET( pi_attributes, "ipv6_udp", packet_in_ipv6_udp( message->data ) );

  HASH_SET( pi_attributes, "ip_dscp", packet_in_ip_dscp( message->data ) );

  HASH_SET( pi_attributes, "vlan_tci", packet_in_vlan_tci( message->data ) );
  HASH_SET( pi_attributes, "vlan_vid", packet_in_vlan_vid( message->data ) );
  HASH_SET( pi_attributes, "vlan_prio", packet_in_vlan_prio( message->data ) );
  HASH_SET( pi_attributes, "vlan_tpid", packet_in_vlan_tpid( message->data ) );
  HASH_SET( pi_attributes, "vlan_cfi", packet_in_vlan_cfi( message->data ) );
  HASH_SET( pi_attributes, "vtag", packet_in_vtag( message->data ) );

  HASH_SET( pi_attributes, "arp", packet_in_arp( message->data ) );
  HASH_SET( pi_attributes, "arp_request", packet_in_arp_request( message->data ) );
  HASH_SET( pi_attributes, "arp_reply", packet_in_arp_reply( message->data ) );
  HASH_SET( pi_attributes, "arp_op", packet_in_arp_op( message->data ) );
  HASH_SET( pi_attributes, "arp_sha", packet_in_arp_sha( message->data ) );
  HASH_SET( pi_attributes, "arp_spa", packet_in_arp_spa( message->data ) );
  HASH_SET( pi_attributes, "arp_tha", packet_in_arp_tha( message->data ) );
  HASH_SET( pi_attributes, "arp_tpa", packet_in_arp_tpa( message->data ) );


  HASH_SET( pi_attributes, "icmpv4", packet_in_icmpv4( message->data ) );

  HASH_SET( pi_attributes, "icmpv6", packet_in_icmpv6( message->data ) );

  HASH_SET( pi_attributes, "ipv6_src", packet_in_ipv6_src( message->data ) );
  HASH_SET( pi_attributes, "ipv6_dst", packet_in_ipv6_dst( message->data ) );
  HASH_SET( pi_attributes, "ipv6_flabel", packet_in_ipv6_flabel( message->data ) );

  HASH_SET( pi_attributes, "igmp", packet_in_igmp( message->data ) );

  VALUE cPacketInfo = rb_funcall( rb_eval_string( "Messages::PacketInfo" ), rb_intern( "new" ), 1, pi_attributes );
  HASH_SET( attributes, "packet_info", cPacketInfo );

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
