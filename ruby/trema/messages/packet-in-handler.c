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
#include "hash-util.h"
#include "conversion-util.h"


#define PACKET_INFO_MAC_ADDR( packet_member )                                          \
  {                                                                                    \
    VALUE ret = ULL2NUM( mac_to_uint64( ( ( packet_info * ) ( message->data->user_data ) )->packet_member ) ); \
    return rb_funcall( rb_eval_string( "Trema::Mac" ), rb_intern( "new" ), 1, ret );   \
  }


#define PACKET_INFO_IPv4_ADDR( packet_member )                                          \
  {                                                                                   \
    VALUE ret = UINT2NUM( ( ( packet_info * ) ( message->data->user_data ) )->packet_member ); \
    return rb_funcall( rb_eval_string( "IPAddr" ), rb_intern( "new" ), 2, ret, rb_eval_string( "Socket::AF_INET" )  );   \
  }


#define PACKET_INFO_IPv6_ADDR( packet_member ) \
  { \
      char ipv6_str[ INET6_ADDRSTRLEN ]; \
      memset( ipv6_str, '\0', sizeof( ipv6_str ) ); \
      if ( inet_ntop( AF_INET6, &( ( packet_info * ) ( message->data->user_data ) )->packet_member, ipv6_str, sizeof( ipv6_str ) ) != NULL ) { \
        return rb_funcall( rb_eval_string( "IPAddr" ), rb_intern( "new" ), 1, rb_str_new2( ipv6_str ) ); \
      } \
      return Qnil; \
  }


static packet_info *
get_packet_in_info( packet_in *message ) {
  return ( packet_info * ) ( message->data->user_data );
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
packet_in_kind( packet_in *message, uint32_t kind ) {
  if ( ( get_packet_in_info( message )->format & kind ) ) {
    return Qtrue;
  }
  return Qfalse;
}


static VALUE
packet_in_eth_type( packet_in *message ) {
  return UINT2NUM( get_packet_in_info( message )->eth_type );
}


static VALUE
packet_in_macsa( packet_in *message ) {
  PACKET_INFO_MAC_ADDR( eth_macsa )
}


static VALUE
packet_in_macda( packet_in *message ) {
  PACKET_INFO_MAC_ADDR( eth_macda )
}



static VALUE
packet_in_vtag( packet_in *message ) {
  return packet_in_kind( message, ETH_8021Q );
}


static VALUE
packet_in_vlan_vid( packet_in *message ) {
  if ( packet_in_vtag( message ) == Qtrue ) {
    return get_packet_in_info( message )->vlan_vid;
  }
  return Qnil;
}


static VALUE
packet_in_vlan_prio( packet_in *message ) {
  if ( packet_in_vtag( message ) == Qtrue ) {
    return get_packet_in_info( message )->vlan_prio;
  }
  return Qnil;
}


static VALUE
packet_in_vlan_cfi( packet_in *message ) {
  if ( packet_in_vtag( message ) == Qtrue ) {
    return get_packet_in_info( message )->vlan_cfi;
  }
  return Qnil;
}


static VALUE
packet_in_vlan_tci( packet_in *message ) {
  if ( packet_in_vtag( message ) == Qtrue ) {
    return get_packet_in_info( message )->vlan_tci;
  }
  return Qnil;
}


static VALUE
packet_in_vlan_tpid( packet_in *message ) {
  if ( packet_in_vtag( message ) == Qtrue ) {
    return get_packet_in_info( message )->vlan_tpid;
  }
  return Qnil;
}


/*
 * Is it an ARP packet?
 *
 * @return [Boolean] whether the packet is an ARP packet or not.
 */
static VALUE
packet_in_arp( packet_in *message ) {
  return packet_in_kind( message, NW_ARP );
}


static VALUE
packet_in_arp_oper( packet_in *message ) {
  if ( packet_in_arp( message ) == Qtrue ) {
    return get_packet_in_info( message )->arp_ar_op;
  }
  return Qnil;
}


static VALUE
packet_in_arp_sha( packet_in *message ) {
  if ( packet_in_arp( message ) == Qtrue ) {
    PACKET_INFO_MAC_ADDR( arp_sha )
  }
  return Qnil;
}


static VALUE
packet_in_arp_spa( packet_in *message ) {
  if ( packet_in_arp( message ) == Qtrue ) {
    PACKET_INFO_IPv4_ADDR( arp_spa )
  }
  return Qnil;
}


static VALUE
packet_in_arp_tha( packet_in *message ) {
  if ( packet_in_arp( message ) == Qtrue ) {
    PACKET_INFO_MAC_ADDR( arp_tha )
  }
  return Qnil;
}


static VALUE
packet_in_arp_tpa( packet_in *message ) {
  if ( packet_in_arp( message ) == Qtrue ) {
    PACKET_INFO_IPv4_ADDR( arp_tpa )
  }
  return Qnil;
}


static VALUE
packet_in_ipv6( packet_in *message ) {
  return packet_in_kind( message, NW_IPV6 );
}


static VALUE
packet_in_ipv4( packet_in *message ) {
  return packet_in_kind( message, NW_IPV4 );
}


static VALUE
packet_in_ipv6_src( packet_in *message ) {
  if ( packet_in_ipv6( message ) == Qtrue ) {
    PACKET_INFO_IPv6_ADDR( ipv6_saddr )
  }
  return Qnil;
}


static VALUE
packet_in_ipv6_dst( packet_in *message ) {
  if ( packet_in_ipv6( message ) == Qtrue ) {
    PACKET_INFO_IPv6_ADDR( ipv6_daddr )
  }
  return Qnil;
}


static VALUE
packet_in_ipv6_flabel( packet_in *message ) {
  if ( packet_in_ipv6( message ) == Qtrue ) {
     return get_packet_in_info( message )->ipv6_flowlabel;
  }
  return Qnil;
}


static VALUE
packet_in_icmpv4( packet_in *message ) {
  return packet_in_kind( message, NW_ICMPV4 );
}


static VALUE
packet_in_icmpv6( packet_in *message ) {
  return packet_in_kind( message, NW_ICMPV6 );
}


static VALUE
packet_in_ip_dscp( packet_in *message ) {
  if ( packet_in_ipv4( message ) == Qtrue ) {
    return get_packet_in_info( message )->ip_dscp;
  }
  return Qnil;
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

  HASH_SET( attributes, "ip_dscp", packet_in_ip_dscp( message ) );

  HASH_SET( attributes, "vlan_tci", packet_in_vlan_tci( message ) );
  HASH_SET( attributes, "vlan_vid", packet_in_vlan_vid( message ) );
  HASH_SET( attributes, "vlan_prio", packet_in_vlan_prio( message ) );
  HASH_SET( attributes, "vlan_tpid", packet_in_vlan_tpid( message ) );
  HASH_SET( attributes, "vlan_cfi", packet_in_vlan_cfi( message ) );
  HASH_SET( attributes, "vtag", packet_in_vtag( message ) );

  HASH_SET( attributes, "arp", packet_in_arp( message ) );
  HASH_SET( attributes, "arp_oper", packet_in_arp_oper( message ) );
  HASH_SET( attributes, "arp_sha", packet_in_arp_sha( message ) );
  HASH_SET( attributes, "arp_spa", packet_in_arp_spa( message ) );
  HASH_SET( attributes, "arp_tha", packet_in_arp_tha( message ) );
  HASH_SET( attributes, "arp_tpa", packet_in_arp_tpa( message ) );


  HASH_SET( attributes, "icmpv4", packet_in_icmpv4( message ) );

  HASH_SET( attributes, "icmpv6", packet_in_icmpv6( message ) );

  HASH_SET( attributes, "ipv6_src", packet_in_ipv6_src( message ) );
  HASH_SET( attributes, "ipv6_dst", packet_in_ipv6_dst( message ) );
  HASH_SET( attributes, "ipv6_flabel", packet_in_ipv6_flabel( message ) );
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
