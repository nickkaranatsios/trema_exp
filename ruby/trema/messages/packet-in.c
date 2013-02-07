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


VALUE cPacketIn;
extern VALUE mMessageHandler;


#define PACKET_IN_RETURN_MAC( packet_member )                                          \
  {                                                                                    \
    VALUE ret = ULL2NUM( mac_to_uint64( get_packet_in_info( self )->packet_member ) ); \
    return rb_funcall( rb_eval_string( "Trema::Mac" ), rb_intern( "new" ), 1, ret );   \
  }

#define PACKET_IN_RETURN_IP( packet_member )                                        \
  {                                                                                 \
    VALUE ret = ULONG2NUM( get_packet_in_info( self )->packet_member );             \
    return rb_funcall( rb_eval_string( "Trema::IP" ), rb_intern( "new" ), 1, ret ); \
  }

#define PACKET_IN_RETURN_NUM( flag, func, packet_member )               \
  {                                                                     \
    if ( get_packet_in_info( self )->format & flag ) {                  \
      return func( get_packet_in_info( self )->packet_member );         \
    } else {                                                            \
      return Qnil;                                                      \
    }                                                                   \
  }


typedef struct rb_packet_in {
  packet_in packet_in;
  buffer *data;
} rb_packet_in;


static void
packet_in_free( rb_packet_in *_packet_in ) {
  free_buffer( _packet_in->data );
  xfree( _packet_in );
}


static packet_in *
get_packet_in( VALUE self ) {
  rb_packet_in *cpacket;
  Data_Get_Struct( self, rb_packet_in, cpacket );
  return &cpacket->packet_in;
}


static packet_info *
get_packet_in_info( VALUE self ) {
  packet_in *cpacket;
  Data_Get_Struct( self, packet_in, cpacket );
  return ( packet_info * ) cpacket->data->user_data;
}


static VALUE
packet_in_alloc( VALUE klass ) {
  rb_packet_in *_packet_in = xmalloc( sizeof( rb_packet_in ) );
  memset( &_packet_in->packet_in, 0, sizeof( packet_in ) );
  _packet_in->data = alloc_buffer_with_length( 1 );
  parse_packet( _packet_in->data );
  _packet_in->packet_in.data = _packet_in->data;
  return Data_Wrap_Struct( klass, 0, packet_in_free, _packet_in );
}


/*
 * Message originator identifier.
 *
 * @return [Number] the value of datapath_id.
 */
static VALUE
packet_in_datapath_id( VALUE self ) {
  return ULL2NUM( get_packet_in( self )->datapath_id );
}


/*
 * For this asynchronous message the transaction_id is set to zero.
 *
 * @return [Number] the value of transaction_id.
 */
static VALUE
packet_in_transaction_id( VALUE self ) {
  return ULONG2NUM( get_packet_in( self )->transaction_id );
}


/*
 * Buffer id value signifies if the entire frame (packet is not buffered) or
 * portion of it (packet is buffered) is included in the data field of
 * this +OFPT_PACKET_IN+ message.
 *
 * @return [Number] the value of buffer id.
 */
static VALUE
packet_in_buffer_id( VALUE self ) {
  return ULONG2NUM( get_packet_in( self )->buffer_id );
}


/*
 * The full length of the received frame.
 *
 * @return [Number] the value of total_len.
 */
static VALUE
packet_in_total_len( VALUE self ) {
  return UINT2NUM( get_packet_in( self )->total_len );
}


/*
 * The reason why the +OFPT_PACKET_IN+ message was sent.
 *
 * @return [Number] the value of reason.
 */
static VALUE
packet_in_reason( VALUE self ) {
  return UINT2NUM( ( uint32_t ) get_packet_in( self )->reason );
}


/*
 * The id of the table the packet was looked up.
 *
 * @return [Number] the value of table_id.
 */
static VALUE
packet_in_table_id( VALUE self ) {
  return UINT2NUM( ( uint32_t ) get_packet_in( self )->table_id );
}


static VALUE
packet_in_cookie( VALUE self ) {
  return ULL2NUM( get_packet_in( self )->cookie );
}


static VALUE
packet_in_in_port( VALUE self ) {
  return UINT2NUM( get_in_port_from_oxm_matches( get_packet_in( self )->match ) );
}


/*
 * The MAC source address.
 *
 * @return [Trema::Mac] macsa MAC source address.
 */
static VALUE
packet_in_macsa( VALUE self ) {
  PACKET_IN_RETURN_MAC( eth_macsa );
}


/*
 * The MAC destination address.
 *
 * @return [Trema::Mac] macda MAC destination address.
 */
static VALUE
packet_in_macda( VALUE self ) {
  PACKET_IN_RETURN_MAC( eth_macda );
}


static void
assign_metadata( const oxm_match_header *hdr, VALUE options ) {
  const uint64_t *value = ( const uint64_t * ) ( ( const char * ) hdr + sizeof ( oxm_match_header ) );

  if ( *hdr == OXM_OF_METADATA ) {
    rb_hash_aset( options, ID2SYM( rb_intern( "metadata" ) ), ULL2NUM( *value ) );
  }
  if ( *hdr == OXM_OF_METADATA_W ) {
    const uint64_t *mask = ( const uint64_t * ) ( ( const char * ) value + sizeof ( uint64_t ) );
    rb_hash_aset( options, ID2SYM( rb_intern( "metadata" ) ), ULL2NUM( *value ) );
    rb_hash_aset( options, ID2SYM( rb_intern( "metadata_mask" ) ), ULL2NUM( *mask ) );
  }
}


static void
assign_eth_addr( const oxm_match_header *hdr, VALUE options ) {
  const uint8_t *addr = ( const uint8_t * ) hdr + sizeof ( oxm_match_header );
  VALUE addr_r = ULL2NUM( mac_to_uint64( addr ) );
  const uint8_t *mask;

  switch( *hdr ) {
    case OXM_OF_ETH_DST:
      rb_hash_aset( options, ID2SYM( rb_intern( "eth_dst" ) ), addr_r );
    break;
    case OXM_OF_ETH_DST_W: {
      rb_hash_aset( options, ID2SYM( rb_intern( "eth_dst" ) ), addr_r );

      mask = addr + ( sizeof ( uint8_t ) * ETH_ADDRLEN );
      VALUE mask_r = ULL2NUM( mac_to_uint64( mask ) );

      rb_hash_aset( options, ID2SYM( rb_intern( "eth_dst_mask" ) ), mask_r );
    
    }
    break;
    case OXM_OF_ETH_SRC:
      rb_hash_aset( options, ID2SYM( rb_intern( "eth_src" ) ), addr_r );
    break;
    case OXM_OF_ETH_SRC_W: {
      rb_hash_aset( options, ID2SYM( rb_intern( "eth_src" ) ), addr_r );

      mask = addr + ( sizeof ( uint8_t ) * ETH_ADDRLEN );
      VALUE mask_r = ULL2NUM( mac_to_uint64( mask ) );

      rb_hash_aset( options, ID2SYM( rb_intern( "eth_src_mask" ) ), mask_r );
    }
    break;
    default:
      assert( 0 );
    break;
  }
}


static void
assign_vlan_vid( const oxm_match_header *hdr, VALUE options ) {
  const uint16_t *value = ( const uint16_t * ) ( ( const char * ) hdr + sizeof ( oxm_match_header ) );
  VALUE sym_vlan_vid = ID2SYM( rb_intern( "vlan_vid" ) );
  VALUE sym_vlan_vid_mask = ID2SYM( rb_intern( "vlan_vid_mask" ) );

  if ( *hdr == OXM_OF_VLAN_VID ) {
    if ( ( *value & OFPVID_PRESENT ) != 0 ) {
      rb_hash_aset( options, sym_vlan_vid, UINT2NUM( ( *value & ( uint16_t ) ~OFPVID_PRESENT ) ) );
    }
    else if ( *value == OFPVID_NONE ) {
      rb_hash_aset( options, sym_vlan_vid, UINT2NUM( 0 ) );
    }
    else {
      rb_hash_aset( options, sym_vlan_vid, UINT2NUM( *value ) );
    }
  }
  if ( *hdr == OXM_OF_VLAN_VID_W ) {
    const uint16_t *mask = ( const uint16_t * ) ( ( const char * ) value + sizeof ( uint16_t ) );
    if ( *value == OFPVID_PRESENT && *mask == OFPVID_PRESENT ) {
      rb_hash_aset( options, sym_vlan_vid, UINT2NUM( 0 ) );
      rb_hash_aset( options, sym_vlan_vid_mask, UINT2NUM( 0 ) );
    }
    else {
      rb_hash_aset( options, sym_vlan_vid, UINT2NUM( *value ) );
      rb_hash_aset( options, sym_vlan_vid_mask, UINT2NUM( *mask ) );
    }
  }
}


static void
assign_ipv4_addr( const oxm_match_header *hdr, VALUE options ) {
  const uint32_t *addr = ( const uint32_t * ) ( ( const uint8_t * ) hdr + sizeof ( oxm_match_header ) );
  const uint32_t *mask;

  switch ( *hdr ) {
    case OXM_OF_IPV4_SRC: {
      rb_hash_aset( options, ID2SYM( rb_intern( "ipv4_src" ) ), UINT2NUM( *addr ) );
    }
    break;
    case OXM_OF_IPV4_SRC_W: {
      mask = ( const uint32_t * ) ( ( const uint8_t * ) addr + sizeof( uint32_t ) );
      rb_hash_aset( options, ID2SYM( rb_intern( "ipv4_src" ) ), UINT2NUM( *addr ) );
      rb_hash_aset( options, ID2SYM( rb_intern( "ipv4_src_mask" ) ), UINT2NUM( *mask ) );
    }
    break;
    case OXM_OF_IPV4_DST: {
      rb_hash_aset( options, ID2SYM( rb_intern( "ipv4_dst" ) ), UINT2NUM( *addr ) );
    }
    break;
    case OXM_OF_IPV4_DST_W: {
      mask = ( const uint32_t * ) ( ( const uint8_t * ) addr + sizeof( uint32_t ) );
      rb_hash_aset( options, ID2SYM( rb_intern( "ipv4_dst" ) ), UINT2NUM( *addr ) );
      rb_hash_aset( options, ID2SYM( rb_intern( "ipv4_dst_mask" ) ), UINT2NUM( *mask ) );
    }
    break;
    default:
      assert( 0 );
    break;
  }
}


static void
assign_tcp_port( const oxm_match_header *hdr, VALUE options ) {
  const uint16_t *value = ( const uint16_t * ) ( ( const char * ) hdr + sizeof ( oxm_match_header ) );

  if ( *hdr == OXM_OF_TCP_SRC ) {
    rb_hash_aset( options, ID2SYM( rb_intern( "tcp_src" ) ), UINT2NUM( *value ) );
  }
  if ( *hdr == OXM_OF_TCP_DST ) {
    rb_hash_aset( options, ID2SYM( rb_intern( "tcp_dst" ) ), UINT2NUM( *value ) );
  }
}


static void
assign_udp_port( const oxm_match_header *hdr, VALUE options ) {
  const uint16_t *value = ( const uint16_t * ) ( ( const char * ) hdr + sizeof( oxm_match_header ) );

  if ( *hdr == OXM_OF_UDP_SRC ) {
    rb_hash_aset( options, ID2SYM( rb_intern( "udp_src" ) ), UINT2NUM( *value ) );
  }
  if ( *hdr == OXM_OF_UDP_DST ) {
    rb_hash_aset( options, ID2SYM( rb_intern( "udp_dst" ) ), UINT2NUM( *value ) );
  }
}


static void
assign_sctp_port( const oxm_match_header *hdr, VALUE options ) {
  const uint16_t *value = ( const uint16_t * ) ( ( const char * ) hdr + sizeof( oxm_match_header ) );

  if ( *hdr == OXM_OF_SCTP_SRC ) {
    rb_hash_aset( options, ID2SYM( rb_intern( "sctp_src" ) ), UINT2NUM( *value ) );
  }
  if ( *hdr == OXM_OF_SCTP_DST ) {
    rb_hash_aset( options, ID2SYM( rb_intern( "sctp_dst" ) ), UINT2NUM( *value ) );
  }
}


static void
assign_arp_protocol_addr( const oxm_match_header *hdr, VALUE options ) {
  const uint32_t *addr = ( const uint32_t * ) ( ( const char * ) hdr + sizeof ( oxm_match_header ) );
  const uint32_t *mask;

  if ( *hdr == OXM_OF_ARP_SPA ) {
    rb_hash_aset( options, ID2SYM( rb_intern( "arp_spa" ) ), UINT2NUM( *addr ) );
  }
  if ( *hdr == OXM_OF_ARP_SPA_W ) {
    rb_hash_aset( options, ID2SYM( rb_intern( "arp_spa" ) ), UINT2NUM( *addr ) );

    mask = ( const uint32_t * ) ( ( const uint8_t * ) addr + sizeof ( uint32_t ) );
    rb_hash_aset( options, ID2SYM( rb_intern( "arp_spa_mask" ) ), UINT2NUM( *mask ) );
  }
  if ( *hdr == OXM_OF_ARP_TPA ) {
    rb_hash_aset( options, ID2SYM( rb_intern( "arp_tpa" ) ), UINT2NUM( *addr ) );
  }
  if ( *hdr == OXM_OF_ARP_TPA_W ) {
    mask = ( const uint32_t * ) ( ( const uint8_t * ) addr + sizeof ( uint32_t ) );
    rb_hash_aset( options, ID2SYM( rb_intern( "arp_tpa" ) ), UINT2NUM( *addr ) );
    rb_hash_aset( options, ID2SYM( rb_intern( "arp_tpa_mask" ) ), UINT2NUM( *mask ) );
  }
}


static void
assign_arp_hardware_addr( const oxm_match_header *hdr, VALUE options ) {
  const uint8_t *addr = ( const uint8_t * ) hdr + sizeof( oxm_match_header );
  VALUE addr_r = ULL2NUM( mac_to_uint64( addr ) );

  const uint8_t *mask;

  if ( *hdr == OXM_OF_ARP_SHA || *hdr == OXM_OF_ARP_SHA_W ) {
    rb_hash_aset( options, ID2SYM( rb_intern( "arp_sha" ) ), addr_r );
    
    if ( *hdr == OXM_OF_ARP_SHA_W ) {
      mask = addr + ( sizeof ( uint8_t ) * ETH_ADDRLEN );
      VALUE mask_r = ULL2NUM( mac_to_uint64( mask ) );
      rb_hash_aset( options, ID2SYM( rb_intern( "arp_sha_mask" ) ), mask_r );
    }
  }
  if ( *hdr == OXM_OF_ARP_THA || *hdr == OXM_OF_ARP_THA_W ) {
    rb_hash_aset( options, ID2SYM( rb_intern( "arp_tha" ) ), addr_r );
    if ( *hdr == OXM_OF_ARP_THA_W ) {
      mask = addr + ( sizeof ( uint8_t ) * ETH_ADDRLEN );
      VALUE mask_r = ULL2NUM( mac_to_uint64( mask ) );
      
      rb_hash_aset( options, ID2SYM( rb_intern( "arp_tha_mask" ) ), mask_r );
    }
  }
}


static void
assign_ipv6_addr( const oxm_match_header *hdr, VALUE options ) {
  const struct in6_addr *addr = ( const struct in6_addr * ) ( ( const char * ) hdr + sizeof( oxm_match_header ) );
  const struct in6_addr *mask;

  if ( *hdr == OXM_OF_IPV6_SRC || *hdr == OXM_OF_IPV6_SRC_W ) {
    rb_hash_aset( options, ID2SYM( rb_intern( "ipv6_src" ) ), rb_str_new2( ( const char * ) addr->s6_addr ) );
    if ( *hdr == OXM_OF_IPV6_SRC_W ) {
      mask = ( const struct in6_addr * ) ( ( const char * ) addr + sizeof ( struct in6_addr ) );
      rb_hash_aset( options, ID2SYM( rb_intern( "ipv6_src_mask" ) ), rb_str_new2( ( const char * ) mask->s6_addr ) );
    }
  }
  if ( *hdr == OXM_OF_IPV6_DST || *hdr == OXM_OF_IPV6_DST_W ) {
    rb_hash_aset( options, ID2SYM( rb_intern( "ipv6_dst" ) ), rb_str_new2( ( const char * ) addr->s6_addr ) );
    if ( *hdr == OXM_OF_IPV6_DST_W ) {
      mask = ( const struct in6_addr * ) ( ( const char * ) addr + sizeof ( struct in6_addr ) );
      rb_hash_aset( options, ID2SYM( rb_intern( "ipv6_dst_mask" ) ), rb_str_new2( ( const char * ) mask->s6_addr ) );
    }
  }
}


static void
assign_ipv6_flabel( const oxm_match_header *hdr, VALUE options ) {
  const uint32_t *value = ( const uint32_t * ) ( ( const char * ) hdr + sizeof( oxm_match_header ) );

  if ( *hdr == OXM_OF_IPV6_FLABEL ) {
    rb_hash_aset( options, ID2SYM( rb_intern( "ipv6_flabel" ) ), UINT2NUM( *value ) );
  }
  if ( *hdr == OXM_OF_IPV6_FLABEL_W ) {
    const uint32_t *mask = ( const uint32_t * ) ( ( const char * ) value + sizeof ( uint32_t ) );
    rb_hash_aset( options, ID2SYM( rb_intern( "ipv6_flabel" ) ), UINT2NUM( *value ) );
    rb_hash_aset( options, ID2SYM( rb_intern( "ipv6_flabel_mask" ) ), UINT2NUM( *mask ) );
  }
}


static void
assign_ipv6_nd_addr( const oxm_match_header *hdr, VALUE options ) {
  const uint8_t *addr = ( const uint8_t * ) hdr + sizeof( oxm_match_header );
  VALUE addr_r = ULL2NUM( mac_to_uint64( addr ) );

  if ( *hdr == OXM_OF_IPV6_ND_SLL ) {
    rb_hash_aset( options, ID2SYM( rb_intern( "ipv6_nd_sll" ) ), addr_r );
  }
  if ( *hdr == OXM_OF_IPV6_ND_TLL ) {
    rb_hash_aset( options, ID2SYM( rb_intern( "ipv6_nd_tll" ) ), addr_r );
  }
}


static void
assign_tunnel_id( const oxm_match_header *hdr, VALUE options ) {
  const uint64_t *value = ( const uint64_t * ) ( ( const char * ) hdr + sizeof( oxm_match_header ) );

  if ( *hdr == OXM_OF_TUNNEL_ID ) {
    rb_hash_aset( options, ID2SYM( rb_intern( "tunnel_id" ) ), ULL2NUM( *value ) );
  }
  if ( *hdr == OXM_OF_TUNNEL_ID_W ) {
    const uint64_t *mask = ( const uint64_t * ) ( ( const char * ) value + sizeof ( uint64_t ) );
    rb_hash_aset( options, ID2SYM( rb_intern( "tunnel_id" ) ), ULL2NUM( *value ) );
    rb_hash_aset( options, ID2SYM( rb_intern( "tunnel_id_mask" ) ), ULL2NUM( *mask ) );
  }
}


static void
assign_ipv6_exthdr( const oxm_match_header *hdr, VALUE options ) {
  const uint16_t *value = ( const uint16_t * ) ( ( const char * ) hdr + sizeof( oxm_match_header ) );

  if ( *hdr == OXM_OF_IPV6_EXTHDR ) {
    rb_hash_aset( options, ID2SYM( rb_intern( "ipv6_exthdr" ) ), UINT2NUM( *value ) );
  }
  if ( *hdr == OXM_OF_IPV6_EXTHDR_W ) {
    const uint16_t *mask = ( const uint16_t * ) ( ( const char * ) value + sizeof ( uint16_t ) );
    rb_hash_aset( options, ID2SYM( rb_intern( "ipv6_exthdr" ) ), UINT2NUM( *value ) );
    rb_hash_aset( options, ID2SYM( rb_intern( "ipv6_exthdr_mask" ) ), UINT2NUM( *mask ) );
  }
}


static void
assign_match( const oxm_match_header *hdr, VALUE options ) {
  switch( *hdr ) {
    case OXM_OF_IN_PORT: {
      const uint32_t *value = ( const uint32_t * ) ( ( const char * ) hdr + sizeof( oxm_match_header ) ); 
      rb_hash_aset( options, ID2SYM( rb_intern( "in_port" ) ), UINT2NUM( *value ) );
    }
    break;
    case OXM_OF_IN_PHY_PORT: {
      const uint32_t *value = ( const uint32_t * ) ( ( const char * ) hdr + sizeof( oxm_match_header ) ); 
      rb_hash_aset( options, ID2SYM( rb_intern( "in_phy_port" ) ), UINT2NUM( *value ) );
    }
    break;
    case OXM_OF_METADATA:
    case OXM_OF_METADATA_W: {
      assign_metadata( hdr, options );
    }
    break;
    case OXM_OF_ETH_DST:
    case OXM_OF_ETH_DST_W:
    case OXM_OF_ETH_SRC:
    case OXM_OF_ETH_SRC_W: {
      assign_eth_addr( hdr, options );
    }
    break;
    case OXM_OF_ETH_TYPE: {
      const uint16_t *value = ( const uint16_t * ) ( ( const char * ) hdr + sizeof( oxm_match_header ) );
      rb_hash_aset( options, ID2SYM( rb_intern( "ether_type" ) ), UINT2NUM( *value ) );
    }
    break;
    case OXM_OF_VLAN_VID:
    case OXM_OF_VLAN_VID_W: {
      assign_vlan_vid( hdr, options );
    }
    break;
    case OXM_OF_VLAN_PCP: {
      const uint8_t *value = ( const uint8_t * ) ( ( const char * ) hdr + sizeof( oxm_match_header ) );
      rb_hash_aset( options, ID2SYM( rb_intern( "vlan_priority" ) ), UINT2NUM( *value ) );
    }
    break;
    case OXM_OF_IP_DSCP: {
      const uint8_t *value = ( const uint8_t * ) ( ( const char * ) hdr + sizeof( oxm_match_header ) );
      rb_hash_aset( options, ID2SYM( rb_intern( "ip_dscp" ) ), UINT2NUM( *value ) );
    }
    break;
    case OXM_OF_IP_PROTO: {
      const uint8_t *value = ( const uint8_t * ) ( ( const char * ) hdr + sizeof( oxm_match_header ) );
      rb_hash_aset( options, ID2SYM( rb_intern( "ip_proto" ) ), UINT2NUM( *value ) );
    }
    break;
    case OXM_OF_IPV4_SRC:
    case OXM_OF_IPV4_SRC_W:
    case OXM_OF_IPV4_DST:
    case OXM_OF_IPV4_DST_W: {
      assign_ipv4_addr( hdr, options );
    }
    break;
    case OXM_OF_TCP_SRC:
    case OXM_OF_TCP_DST: {
      assign_tcp_port( hdr, options );
    }
    break;
    case OXM_OF_UDP_SRC:
    case OXM_OF_UDP_DST: {
      assign_udp_port( hdr, options );
    }
    break;
    case OXM_OF_SCTP_SRC:
    case OXM_OF_SCTP_DST: {
      assign_sctp_port( hdr, options );
    }
    break;
    case OXM_OF_ICMPV4_TYPE: {
      const uint8_t *value = ( const uint8_t * ) ( ( const char * ) hdr + sizeof( oxm_match_header ) );
      rb_hash_aset( options, ID2SYM( rb_intern( "icmpv4_type" ) ), UINT2NUM( *value ) );
    }
    break;
    case OXM_OF_ICMPV4_CODE: {
      const uint8_t *value = ( const uint8_t * ) ( ( const char * ) hdr + sizeof( oxm_match_header ) );
      rb_hash_aset( options, ID2SYM( rb_intern( "icmpv4_code" ) ), UINT2NUM( *value ) );
    }
    break;
    case OXM_OF_ARP_OP: {
      const uint16_t *value = ( const uint16_t * ) ( ( const char * ) hdr + sizeof ( oxm_match_header ) );
      rb_hash_aset( options, ID2SYM( rb_intern( "arp_op" ) ), UINT2NUM( *value ) );
    }
    break;
    case OXM_OF_ARP_SPA:
    case OXM_OF_ARP_SPA_W:
    case OXM_OF_ARP_TPA:
    case OXM_OF_ARP_TPA_W: {
      assign_arp_protocol_addr( hdr, options );
    }
    break;
    case OXM_OF_ARP_SHA:
    case OXM_OF_ARP_SHA_W:
    case OXM_OF_ARP_THA:
    case OXM_OF_ARP_THA_W: {
      assign_arp_hardware_addr( hdr, options );
    }
    break;
    case OXM_OF_IPV6_SRC:
    case OXM_OF_IPV6_SRC_W:
    case OXM_OF_IPV6_DST:
    case OXM_OF_IPV6_DST_W: {
      assign_ipv6_addr( hdr, options );
    }
    break;
    case OXM_OF_IPV6_FLABEL:
    case OXM_OF_IPV6_FLABEL_W: {
      assign_ipv6_flabel( hdr, options );
    }
    break;
    case OXM_OF_ICMPV6_TYPE: {
      const uint8_t *value = ( const uint8_t * ) ( ( const char * ) hdr + sizeof( oxm_match_header ) );
      rb_hash_aset( options, ID2SYM( rb_intern( "icmpv6_type" ) ), UINT2NUM( *value ) );
    }
    break;
    case OXM_OF_ICMPV6_CODE: {
      const uint8_t *value = ( const uint8_t * ) ( ( const char * ) hdr + sizeof( oxm_match_header ) );
      rb_hash_aset( options, ID2SYM( rb_intern( "icmpv6_code" ) ),  *value );
    }
    break;
    case OXM_OF_IPV6_ND_TARGET: {
      const struct in6_addr *addr = ( const struct in6_addr * ) ( ( const char * ) hdr + sizeof( oxm_match_header ) );
      rb_hash_aset( options, ID2SYM( rb_intern( "ipv6_nd_target" ) ), rb_str_new2( ( const char * ) addr->s6_addr ) );
    }
    break;
    case OXM_OF_IPV6_ND_SLL:
    case OXM_OF_IPV6_ND_TLL: {
      assign_ipv6_nd_addr( hdr, options );
    }
    break;
    case OXM_OF_MPLS_LABEL: {
      const uint32_t *value = ( const uint32_t * ) ( ( const char * ) hdr + sizeof ( oxm_match_header ) );
      rb_hash_aset( options, ID2SYM( rb_intern( "mpls_label" ) ), UINT2NUM( *value ) );
    }
    break;
    case OXM_OF_MPLS_TC: {
      const uint8_t *value = ( const uint8_t * ) ( ( const char * ) hdr + sizeof( oxm_match_header ) );
      rb_hash_aset( options, ID2SYM( rb_intern( "mpls_tc" ) ), UINT2NUM( *value ) );
    }
    break;
    case OXM_OF_MPLS_BOS: {
      const uint8_t *value = ( const uint8_t * ) ( ( const char * ) hdr + sizeof( oxm_match_header ) );
      rb_hash_aset( options, ID2SYM( rb_intern( "mpls_bos" ) ), UINT2NUM( *value ) );
    }
    break;
    case OXM_OF_TUNNEL_ID:
    case OXM_OF_TUNNEL_ID_W: {
      assign_tunnel_id( hdr, options );
    }
    break;
    case OXM_OF_IPV6_EXTHDR:
    case OXM_OF_IPV6_EXTHDR_W: {
      assign_ipv6_exthdr( hdr, options );
    }
    break;
    default:
      error( "Undefined oxm type ( header = %#x, type = %#x, has_mask = %u, length = %u ). ",
              *hdr, OXM_TYPE( *hdr ), OXM_HASMASK( *hdr ), OXM_LENGTH( *hdr ) );
    break;
  }
}


static VALUE
packet_in_match( VALUE self ) {
  VALUE options = rb_hash_new();

  for ( list_element *list = get_packet_in( self )->match->list; list != NULL; list = list->next ) {
    oxm_match_header *oxm = list->data;
    assign_match( oxm, options );
  }
  VALUE match = rb_funcall( rb_eval_string( "Messages::Match" ), rb_intern( "new" ), 1, options );
  return match;
}


static VALUE
packet_in_data( VALUE self ) {
  UNUSED( self );
  const buffer *data_frame = get_packet_in( self )->data;
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


void
Init_packet_in() {
  rb_require( "trema/ip" );
  rb_require( "trema/mac" );
  cPacketIn = rb_define_class_under( mMessageHandler, "PacketIn", rb_cObject );
  rb_define_alloc_func( cPacketIn, packet_in_alloc );

  rb_define_method( cPacketIn, "datapath_id", packet_in_datapath_id, 0 );
  rb_define_method( cPacketIn, "transaction_id", packet_in_transaction_id, 0 );
  rb_define_method( cPacketIn, "buffer_id", packet_in_buffer_id, 0 );
  rb_define_method( cPacketIn, "total_len", packet_in_total_len, 0 );
  rb_define_method( cPacketIn, "reason", packet_in_reason, 0 );
  rb_define_method( cPacketIn, "table_id", packet_in_table_id, 0 );
  rb_define_method( cPacketIn, "cookie", packet_in_cookie, 0 );
  rb_define_method( cPacketIn, "in_port", packet_in_in_port, 0 );
  rb_define_method( cPacketIn, "data", packet_in_data, 0 );
  rb_define_method( cPacketIn, "macsa", packet_in_macsa, 0 );
  rb_define_method( cPacketIn, "macda", packet_in_macda, 0 );
  rb_define_method( cPacketIn, "match", packet_in_match, 0 );
  
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

  VALUE r_message = rb_funcall( cPacketIn, rb_intern( "new" ), 0 );
  packet_in *tmp = NULL;
  Data_Get_Struct( r_message, packet_in, tmp );
  memcpy( tmp, &message, sizeof( packet_in ) );

  rb_funcall( controller, rb_intern( "packet_in" ), 2, ULL2NUM( datapath_id ), r_message );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
