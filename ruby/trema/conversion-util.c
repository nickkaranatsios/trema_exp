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
#include "oxm-helper.h"
#include "action-common.h"
#include "hash-util.h"


struct in6_addr
ipv6_addr_to_in6_addr( VALUE ipv6_addr ) {
  struct in6_addr in6_addr;

  VALUE ipv6_addr_str = rb_funcall( ipv6_addr, rb_intern( "to_s" ), 0 );
  const char *dst = rb_string_value_cstr( &ipv6_addr_str );
  inet_pton( AF_INET6, dst, &in6_addr );
  return in6_addr;
}


VALUE
ipv6_addr_to_r( const struct in6_addr *addr ) {
  char ipv6_str[ INET6_ADDRSTRLEN ];
  memset( ipv6_str, '\0', sizeof( ipv6_str ) );

  VALUE r_ipv6_addr = Qnil;
  if ( inet_ntop( AF_INET6, addr, ipv6_str, sizeof( ipv6_str ) ) != NULL ) {
    r_ipv6_addr = rb_funcall( rb_eval_string( "IPAddr" ), rb_intern( "new" ), 1, rb_str_new2( ipv6_str ) );
  }
  return r_ipv6_addr;
}


VALUE
ipv4_addr_to_r( const uint32_t *addr ) {
  return rb_funcall( rb_eval_string( "IPAddr" ), rb_intern( "new" ), 2, UINT2NUM( *addr ), rb_eval_string( "Socket::AF_INET" )  );
}


VALUE
eth_addr_to_r( const uint8_t *addr ) {
  return rb_funcall( rb_eval_string( "Mac" ), rb_intern( "new" ), 1, ULL2NUM( mac_to_uint64( addr ) ) );
}


buffer *
r_array_to_buffer( VALUE r_array ) {
  buffer *data = NULL;

  if ( r_array != Qnil ) {
    Check_Type( r_array, T_ARRAY );
    uint32_t length = ( uint32_t ) RARRAY_LEN( r_array );

    data = alloc_buffer_with_length( length );
    append_back_buffer( data, length );
    uint8_t *data_ptr = data->data;
    for ( uint32_t i = 0; i < length; i++ ) {
      data_ptr[ i ] = ( uint8_t ) FIX2INT( RARRAY_PTR( r_array ) [ i ] );
    }
  }
  return data;
}


VALUE
buffer_to_r_array( const buffer *buffer ) {
  if ( buffer != NULL ) {
    if ( buffer->length ) {
      uint16_t length = ( uint16_t ) buffer->length;
      VALUE data_array = rb_ary_new2( ( long int ) length );
      uint8_t *data = ( uint8_t * ) ( ( char * ) buffer->data );
      long i;

      for ( i = 0; i < length; i++ ) {
        rb_ary_push( data_array, INT2FIX( data[ i ] ) );
      }
      return data_array;
    }
  }
  return Qnil;
}


VALUE
oxm_match_to_r_match( const oxm_matches *match ) {
  assert( match != NULL );

  VALUE options = rb_hash_new();

  for ( list_element *list = match->list; list != NULL; list = list->next ) {
    oxm_match_header *oxm = list->data;
    assign_match( oxm, options );
  }
  return rb_funcall( rb_eval_string( "Match" ), rb_intern( "new" ), 1, options );
}


static void
assign_metadata( const oxm_match_header *hdr, VALUE r_attributes ) {
  const uint64_t *value = ( const uint64_t * ) ( ( const char * ) hdr + sizeof ( oxm_match_header ) );

  if ( *hdr == OXM_OF_METADATA ) {
    HASH_SET( r_attributes, "metadata", ULL2NUM( *value ) );
  }
  if ( *hdr == OXM_OF_METADATA_W ) {
    value = ( const uint64_t * ) ( ( const char * ) hdr + sizeof ( oxm_match_header ) );
    const uint64_t *mask = ( const uint64_t * ) ( ( const char * ) value + sizeof ( uint64_t ) );
    HASH_SET( r_attributes, "metadata", ULL2NUM( *value ) );
    HASH_SET( r_attributes, "metadata_mask", ULL2NUM( *mask ) );
  }
}


static void
assign_eth_addr( const oxm_match_header *hdr, VALUE r_attributes ) {
  const uint8_t *addr = ( const uint8_t * ) hdr + sizeof ( oxm_match_header );
  const uint8_t *mask;

  VALUE r_addr = eth_addr_to_r( addr );
  switch( *hdr ) {
    case OXM_OF_ETH_DST:
      HASH_SET( r_attributes, "eth_dst", r_addr );
      break;
    case OXM_OF_ETH_DST_W: {
        HASH_SET( r_attributes, "eth_dst", r_addr );
        mask = addr + ( sizeof ( uint8_t ) * ETH_ADDRLEN );

        VALUE r_mask = eth_addr_to_r( mask );
        HASH_SET( r_attributes, "eth_dst_mask", r_mask );
      }
      break;
    case OXM_OF_ETH_SRC:
      HASH_SET( r_attributes, "eth_src", r_addr );
      break;
    case OXM_OF_ETH_SRC_W: {
        HASH_SET( r_attributes, "eth_src", r_addr );

        mask = addr + ( sizeof ( uint8_t ) * ETH_ADDRLEN );
        VALUE r_mask = eth_addr_to_r( mask );
        HASH_SET( r_attributes, "eth_src_mask", r_mask );
      }
      break;
    default:
      assert( 0 );
      break;
  }
}


static void
assign_vlan_vid( const oxm_match_header *hdr, VALUE r_attributes ) {
  const uint16_t *value = ( const uint16_t * ) ( ( const char * ) hdr + sizeof ( oxm_match_header ) );
  if ( *hdr == OXM_OF_VLAN_VID ) {
    if ( ( *value & OFPVID_PRESENT ) != 0 ) {
      HASH_SET( r_attributes, "vlan_vid", UINT2NUM( ( *value & ( uint16_t ) ~OFPVID_PRESENT ) ) );
    }
    else if ( *value == OFPVID_NONE ) {
      HASH_SET( r_attributes, "vlan_vid", UINT2NUM( 0 ) );
    }
    else {
      HASH_SET( r_attributes, "vlan_vid", UINT2NUM( *value ) );
    }
  }
  if ( *hdr == OXM_OF_VLAN_VID_W ) {
    const uint16_t *mask = ( const uint16_t * ) ( ( const char * ) value + sizeof ( uint16_t ) );
    if ( *value == OFPVID_PRESENT && *mask == OFPVID_PRESENT ) {
      HASH_SET( r_attributes, "vlan_vid", UINT2NUM( 0 ) );
      HASH_SET( r_attributes, "vlan_vid_mask", UINT2NUM( 0 ) );
    }
    else {
      HASH_SET( r_attributes, "vlan_vid", UINT2NUM( *value ) );
      HASH_SET( r_attributes, "vlan_vid_mask", UINT2NUM( *mask ) );
    }
  }
}


static void
assign_ipv4_addr( const oxm_match_header *hdr, VALUE r_attributes ) {
  const uint32_t *addr = ( const uint32_t * ) ( ( const uint8_t * ) hdr + sizeof ( oxm_match_header ) );
  const uint32_t *mask;

  VALUE r_addr = ipv4_addr_to_r( addr );
  switch ( *hdr ) {
    case OXM_OF_IPV4_SRC: {
      HASH_SET( r_attributes, "ipv4_src", r_addr );
    }
    break;
    case OXM_OF_IPV4_SRC_W: {
      mask = ( const uint32_t * ) ( ( const uint8_t * ) addr + sizeof( uint32_t ) );
      VALUE r_mask = ipv4_addr_to_r( mask );

      HASH_SET( r_attributes, "ipv4_src", r_addr );
      HASH_SET( r_attributes, "ipv4_src_mask", r_mask );
    }
    break;
    case OXM_OF_IPV4_DST: {
      HASH_SET( r_attributes, "ipv4_dst", r_addr );
    }
    break;
    case OXM_OF_IPV4_DST_W: {
      mask = ( const uint32_t * ) ( ( const uint8_t * ) addr + sizeof( uint32_t ) );
      VALUE r_mask = ipv4_addr_to_r( mask );

      HASH_SET( r_attributes, "ipv4_dst", r_addr );
      HASH_SET( r_attributes, "ipv4_dst_mask", r_mask );
    }
    break;
    default:
      assert( 0 );
    break;
  }
}


static void
assign_tcp_port( const oxm_match_header *hdr, VALUE r_attributes ) {
  const uint16_t *value = ( const uint16_t * ) ( ( const char * ) hdr + sizeof ( oxm_match_header ) );

  if ( *hdr == OXM_OF_TCP_SRC ) {
    HASH_SET( r_attributes, "tcp_src", UINT2NUM( *value ) );
  }
  if ( *hdr == OXM_OF_TCP_DST ) {
    HASH_SET( r_attributes, "tcp_dst", UINT2NUM( *value ) );
  }
}


static void
assign_udp_port( const oxm_match_header *hdr, VALUE r_attributes ) {
  const uint16_t *value = ( const uint16_t * ) ( ( const char * ) hdr + sizeof( oxm_match_header ) );

  if ( *hdr == OXM_OF_UDP_SRC ) {
    HASH_SET( r_attributes, "udp_src", UINT2NUM( *value ) );
  }
  if ( *hdr == OXM_OF_UDP_DST ) {
    HASH_SET( r_attributes, "udp_dst", UINT2NUM( *value ) );
  }
}


static void
assign_sctp_port( const oxm_match_header *hdr, VALUE r_attributes ) {
  const uint16_t *value = ( const uint16_t * ) ( ( const char * ) hdr + sizeof( oxm_match_header ) );

  if ( *hdr == OXM_OF_SCTP_SRC ) {
    HASH_SET( r_attributes, "sctp_src", UINT2NUM( *value ) );
  }
  if ( *hdr == OXM_OF_SCTP_DST ) {
    HASH_SET( r_attributes, "sctp_dst", UINT2NUM( *value ) );
  }
}


static void
assign_arp_protocol_addr( const oxm_match_header *hdr, VALUE r_attributes ) {
  const uint32_t *addr = ( const uint32_t * ) ( ( const uint8_t * ) hdr + sizeof ( oxm_match_header ) );
  const uint32_t *mask;

  VALUE r_addr = ipv4_addr_to_r( addr );

  if ( *hdr == OXM_OF_ARP_SPA ) {
    HASH_SET( r_attributes, "arp_spa", r_addr );
  }
  if ( *hdr == OXM_OF_ARP_SPA_W ) {
    mask = ( const uint32_t * ) ( ( const uint8_t * ) addr + sizeof ( uint32_t ) );
    VALUE r_mask = ipv4_addr_to_r( addr );

    HASH_SET( r_attributes, "arp_spa", r_addr );
    HASH_SET( r_attributes, "arp_spa_mask", r_mask );
  }
  if ( *hdr == OXM_OF_ARP_TPA ) {
    HASH_SET( r_attributes, "arp_tpa", r_addr );
  }
  if ( *hdr == OXM_OF_ARP_TPA_W ) {
    mask = ( const uint32_t * ) ( ( const uint8_t * ) addr + sizeof ( uint32_t ) );
    VALUE r_mask = ipv4_addr_to_r( addr );

    HASH_SET( r_attributes, "arp_tpa", r_addr );
    HASH_SET( r_attributes, "arp_tpa_mask", r_mask );
  }
}


static void
assign_arp_hardware_addr( const oxm_match_header *hdr, VALUE r_attributes ) {
  const uint8_t *addr = ( const uint8_t * ) hdr + sizeof( oxm_match_header );
  const uint8_t *mask;

  VALUE r_addr = eth_addr_to_r( addr );

  if ( *hdr == OXM_OF_ARP_SHA || *hdr == OXM_OF_ARP_SHA_W ) {
    HASH_SET( r_attributes, "arp_sha", r_addr );
    if ( *hdr == OXM_OF_ARP_SHA_W ) {
      mask = addr + ( sizeof ( uint8_t ) * ETH_ADDRLEN );
      VALUE r_mask = eth_addr_to_r( mask );
      HASH_SET( r_attributes, "arp_sha_mask", r_mask );
    }
  }
  if ( *hdr == OXM_OF_ARP_THA || *hdr == OXM_OF_ARP_THA_W ) {
    HASH_SET( r_attributes, "arp_tha", r_addr );
    if ( *hdr == OXM_OF_ARP_THA_W ) {
      mask = addr + ( sizeof ( uint8_t ) * ETH_ADDRLEN );
      VALUE r_mask = eth_addr_to_r( mask );
      HASH_SET( r_attributes, "arp_tha_mask", r_mask );
    }
  }
}


static void
assign_ipv6_addr( const oxm_match_header *hdr, VALUE r_attributes ) {
  const struct in6_addr *addr = ( const struct in6_addr * ) ( ( const char * ) hdr + sizeof( oxm_match_header ) );
  const struct in6_addr *mask;


  VALUE r_ipv6_addr = ipv6_addr_to_r( addr );;

  if ( *hdr == OXM_OF_IPV6_SRC || *hdr == OXM_OF_IPV6_SRC_W ) {
    HASH_SET( r_attributes, "ipv6_src", r_ipv6_addr );

    if ( *hdr == OXM_OF_IPV6_SRC_W ) {
      mask = ( const struct in6_addr * ) ( ( const char * ) addr + sizeof ( struct in6_addr ) );
      VALUE r_ipv6_mask = ipv6_addr_to_r( mask );

      HASH_SET( r_attributes, "ipv6_src_mask", r_ipv6_mask );
    }
  }
  if ( *hdr == OXM_OF_IPV6_DST || *hdr == OXM_OF_IPV6_DST_W ) {
    HASH_SET( r_attributes, "ipv6_dst", r_ipv6_addr );
    if ( *hdr == OXM_OF_IPV6_DST_W ) {
      mask = ( const struct in6_addr * ) ( ( const char * ) addr + sizeof ( struct in6_addr ) );

      VALUE r_ipv6_mask = ipv6_addr_to_r( mask );
      HASH_SET( r_attributes, "ipv6_dst_mask", r_ipv6_mask );
    }
  }
}


static void
assign_ipv6_flabel( const oxm_match_header *hdr, VALUE r_attributes ) {
  const uint32_t *value = ( const uint32_t * ) ( ( const char * ) hdr + sizeof( oxm_match_header ) );

  if ( *hdr == OXM_OF_IPV6_FLABEL ) {
    HASH_SET( r_attributes, "ipv6_flabel", UINT2NUM( *value ) );
  }
  if ( *hdr == OXM_OF_IPV6_FLABEL_W ) {
    HASH_SET( r_attributes, "ipv6_flabel", UINT2NUM( *value ) );

    const uint32_t *mask = ( const uint32_t * ) ( ( const char * ) value + sizeof ( uint32_t ) );
    HASH_SET( r_attributes, "ipv6_flabel_mask", UINT2NUM( *mask ) );
  }
}


static void
assign_ipv6_nd_addr( const oxm_match_header *hdr, VALUE r_attributes ) {
  const uint8_t *addr = ( const uint8_t * ) hdr + sizeof( oxm_match_header );

  VALUE r_addr = eth_addr_to_r( addr );

  if ( *hdr == OXM_OF_IPV6_ND_SLL ) {
    HASH_SET( r_attributes, "ipv6_nd_sll", r_addr );
  }
  if ( *hdr == OXM_OF_IPV6_ND_TLL ) {
    HASH_SET( r_attributes, "ipv6_nd_tll", r_addr );
  }
}


static void
assign_tunnel_id( const oxm_match_header *hdr, VALUE r_attributes ) {
  const uint64_t *value = ( const uint64_t * ) ( ( const char * ) hdr + sizeof( oxm_match_header ) );

  if ( *hdr == OXM_OF_TUNNEL_ID ) {
    HASH_SET( r_attributes, "tunnel_id", ULL2NUM( *value ) );
  }
  if ( *hdr == OXM_OF_TUNNEL_ID_W ) {
    HASH_SET( r_attributes, "tunnel_id", ULL2NUM( *value ) );
    const uint64_t *mask = ( const uint64_t * ) ( ( const char * ) value + sizeof ( uint64_t ) );
    HASH_SET( r_attributes, "tunnel_id", ULL2NUM( *mask ) );
  }
}


static void
assign_ipv6_exthdr( const oxm_match_header *hdr, VALUE r_attributes ) {
  const uint16_t *value = ( const uint16_t * ) ( ( const char * ) hdr + sizeof( oxm_match_header ) );

  if ( *hdr == OXM_OF_IPV6_EXTHDR ) {
    HASH_SET( r_attributes, "ipv6_exthdr", UINT2NUM( *value ) );
  }
  if ( *hdr == OXM_OF_IPV6_EXTHDR_W ) {
    HASH_SET( r_attributes, "ipv6_exthdr", UINT2NUM( *value ) );

    const uint16_t *mask = ( const uint16_t * ) ( ( const char * ) value + sizeof ( uint16_t ) );
    HASH_SET( r_attributes, "ipv6_exthdr_mask", UINT2NUM( *mask ) );
  }
}


static void
assign_r_match( const oxm_match_header *hdr, VALUE r_attributes ) {
  switch( *hdr ) {
    case OXM_OF_IN_PORT: {
      const uint32_t *value = ( const uint32_t * ) ( ( const char * ) hdr + sizeof( oxm_match_header ) );
      HASH_SET( r_attributes, "in_port", UINT2NUM( *value ) );
    }
    break;
    case OXM_OF_IN_PHY_PORT: {
      const uint32_t *value = ( const uint32_t * ) ( ( const char * ) hdr + sizeof( oxm_match_header ) );
      HASH_SET( r_attributes, "in_phy_port", UINT2NUM( *value ) );
    }
    break;
    case OXM_OF_METADATA:
    case OXM_OF_METADATA_W: {
      assign_metadata( hdr, r_attributes );
    }
    break;
    case OXM_OF_ETH_DST:
    case OXM_OF_ETH_DST_W:
    case OXM_OF_ETH_SRC:
    case OXM_OF_ETH_SRC_W: {
      assign_eth_addr( hdr, r_attributes );
    }
    break;
    case OXM_OF_ETH_TYPE: {
      const uint16_t *value = ( const uint16_t * ) ( ( const char * ) hdr + sizeof( oxm_match_header ) );
      HASH_SET( r_attributes, "eth_type", UINT2NUM( *value ) );
    }
    break;
    case OXM_OF_VLAN_VID:
    case OXM_OF_VLAN_VID_W: {
      assign_vlan_vid( hdr, r_attributes );
    }
    break;
    case OXM_OF_VLAN_PCP: {
      const uint8_t *value = ( const uint8_t * ) ( ( const char * ) hdr + sizeof( oxm_match_header ) );
      HASH_SET( r_attributes, "vlan_pcp", UINT2NUM( *value ) );
    }
    break;
    case OXM_OF_IP_DSCP: {
      const uint8_t *value = ( const uint8_t * ) ( ( const char * ) hdr + sizeof( oxm_match_header ) );
      HASH_SET( r_attributes, "ip_dscp", UINT2NUM( *value ) );
    }
    break;
    case OXM_OF_IP_ECN: {
      const uint8_t *value = ( const uint8_t * ) ( ( const char * ) hdr + sizeof( oxm_match_header ) );
      HASH_SET( r_attributes, "ip_ecn", UINT2NUM( *value ) );
    }
    break;
    case OXM_OF_IP_PROTO: {
      const uint8_t *value = ( const uint8_t * ) ( ( const char * ) hdr + sizeof( oxm_match_header ) );
      HASH_SET( r_attributes, "ip_proto", UINT2NUM( *value ) );
    }
    break;
    case OXM_OF_IPV4_SRC:
    case OXM_OF_IPV4_SRC_W:
    case OXM_OF_IPV4_DST:
    case OXM_OF_IPV4_DST_W: {
      assign_ipv4_addr( hdr, r_attributes );
    }
    break;
    case OXM_OF_TCP_SRC:
    case OXM_OF_TCP_DST: {
      assign_tcp_port( hdr, r_attributes );
    }
    break;
    case OXM_OF_UDP_SRC:
    case OXM_OF_UDP_DST: {
      assign_udp_port( hdr, r_attributes );
    }
    break;
    case OXM_OF_SCTP_SRC:
    case OXM_OF_SCTP_DST: {
      assign_sctp_port( hdr, r_attributes );
    }
    break;
    case OXM_OF_ICMPV4_TYPE: {
      const uint8_t *value = ( const uint8_t * ) ( ( const char * ) hdr + sizeof( oxm_match_header ) );
      HASH_SET( r_attributes, "icmpv4_type", UINT2NUM( *value ) );
    }
    break;
    case OXM_OF_ICMPV4_CODE: {
      const uint8_t *value = ( const uint8_t * ) ( ( const char * ) hdr + sizeof( oxm_match_header ) );
      HASH_SET( r_attributes, "icmpv4_code", UINT2NUM( *value ) );
    }
    break;
    case OXM_OF_ARP_OP: {
      const uint16_t *value = ( const uint16_t * ) ( ( const char * ) hdr + sizeof ( oxm_match_header ) );
      HASH_SET( r_attributes, "arp_op", UINT2NUM( *value ) );
    }
    break;
    case OXM_OF_ARP_SPA:
    case OXM_OF_ARP_SPA_W:
    case OXM_OF_ARP_TPA:
    case OXM_OF_ARP_TPA_W: {
      assign_arp_protocol_addr( hdr, r_attributes );
    }
    break;
    case OXM_OF_ARP_SHA:
    case OXM_OF_ARP_SHA_W:
    case OXM_OF_ARP_THA:
    case OXM_OF_ARP_THA_W: {
      assign_arp_hardware_addr( hdr, r_attributes );
    }
    break;
    case OXM_OF_IPV6_SRC:
    case OXM_OF_IPV6_SRC_W:
    case OXM_OF_IPV6_DST:
    case OXM_OF_IPV6_DST_W: {
      assign_ipv6_addr( hdr, r_attributes );
    }
    break;
    case OXM_OF_IPV6_FLABEL:
    case OXM_OF_IPV6_FLABEL_W: {
      assign_ipv6_flabel( hdr, r_attributes );
    }
    break;
    case OXM_OF_ICMPV6_TYPE: {
      const uint8_t *value = ( const uint8_t * ) ( ( const char * ) hdr + sizeof( oxm_match_header ) );
      HASH_SET( r_attributes, "icmpv6_type", UINT2NUM( *value ) );
    }
    break;
    case OXM_OF_ICMPV6_CODE: {
      const uint8_t *value = ( const uint8_t * ) ( ( const char * ) hdr + sizeof( oxm_match_header ) );
      HASH_SET( r_attributes, "icmpv6_code", UINT2NUM( *value ) );
    }
    break;
    case OXM_OF_IPV6_ND_TARGET: {
      const struct in6_addr *addr = ( const struct in6_addr * ) ( ( const char * ) hdr + sizeof( oxm_match_header ) );
      VALUE r_addr = ipv6_addr_to_r( addr );
      HASH_SET( r_attributes, "ipv6_nd_target", r_addr );
    }
    break;
    case OXM_OF_IPV6_ND_SLL:
    case OXM_OF_IPV6_ND_TLL: {
      assign_ipv6_nd_addr( hdr, r_attributes );
    }
    break;
    case OXM_OF_MPLS_LABEL: {
      const uint32_t *value = ( const uint32_t * ) ( ( const char * ) hdr + sizeof ( oxm_match_header ) );
      HASH_SET( r_attributes, "mpls_label", UINT2NUM( *value ) );
    }
    break;
    case OXM_OF_MPLS_TC: {
      const uint8_t *value = ( const uint8_t * ) ( ( const char * ) hdr + sizeof( oxm_match_header ) );
      HASH_SET( r_attributes, "mpls_tc", UINT2NUM( *value ) );
    }
    break;
    case OXM_OF_MPLS_BOS: {
      const uint8_t *value = ( const uint8_t * ) ( ( const char * ) hdr + sizeof( oxm_match_header ) );
      HASH_SET( r_attributes, "mpls_bos", UINT2NUM( *value ) );
    }
    break;
    case OXM_OF_TUNNEL_ID:
    case OXM_OF_TUNNEL_ID_W: {
      assign_tunnel_id( hdr, r_attributes );
    }
    break;
    case OXM_OF_IPV6_EXTHDR:
    case OXM_OF_IPV6_EXTHDR_W: {
      assign_ipv6_exthdr( hdr, r_attributes );
    }
    break;
    default:
      error( "Undefined oxm type ( header = %#x, type = %#x, has_mask = %u, length = %u ). ",
              *hdr, OXM_TYPE( *hdr ), OXM_HASMASK( *hdr ), OXM_LENGTH( *hdr ) );
    break;
  }
}


VALUE
ofp_match_to_r_match( const struct ofp_match *match ) {
  assert( match != NULL );
  assert( match->length >= offsetof( struct ofp_match, oxm_fields ) );

  uint16_t oxms_len = 0;
  uint16_t oxm_len = 0;
  oxm_match_header *src;

  uint16_t offset = offsetof( struct ofp_match, oxm_fields );
  oxms_len = ( uint16_t ) ( ntohs( match->length ) - offset );
  src = ( oxm_match_header * ) ( ( char * ) match + offset );
  VALUE r_attributes = rb_hash_new();

  while ( oxms_len > sizeof( oxm_match_header ) ) {
    oxm_len = OXM_LENGTH( *src );
    assign_r_match( src, r_attributes );

    offset = ( uint16_t ) ( sizeof( oxm_match_header ) + oxm_len );
    if ( oxms_len < offset ) {
      break;
    }
    oxms_len = ( uint16_t ) ( oxms_len - offset );
    src = ( oxm_match_header * ) ( ( char * ) src + offset );
  }
  return rb_funcall( rb_eval_string( "Match" ), rb_intern( "new" ), 1, r_attributes );
}


void
r_match_to_oxm_match( VALUE r_match, oxm_matches *match ) {
  append_oxm_match_in_port( match, ( uint32_t ) NUM2UINT( rb_iv_get( r_match, "@in_port" ) ) );

  uint8_t tmp_mac_mask[ OFP_ETH_ALEN ];
  memset( tmp_mac_mask, 0, sizeof( tmp_mac_mask ) );

  VALUE r_eth_src = rb_iv_get( r_match, "@eth_src" );
  if ( r_eth_src != Qnil ) {
    append_oxm_match_eth_src( match, mac_addr_to_cstr( r_eth_src ), tmp_mac_mask );
  }
  VALUE r_eth_dst = rb_iv_get( r_match, "@eth_dst" );
  if ( r_eth_dst != Qnil ) {
    append_oxm_match_eth_dst( match, mac_addr_to_cstr( r_eth_dst ), tmp_mac_mask );
  }

  VALUE r_vlan_vid = rb_iv_get( r_match, "@vlan_vid" );
  if ( r_vlan_vid != Qnil ) {
    uint16_t vlan_vid = ( uint16_t ) NUM2UINT( r_vlan_vid );
    append_oxm_match_vlan_vid( match, vlan_vid, 0 );
  }

  VALUE r_vlan_pcp = rb_iv_get( r_match, "@vlan_pcp" );
  if ( r_vlan_pcp != Qnil ) {
    append_oxm_match_vlan_pcp( match, ( uint8_t ) NUM2UINT( r_vlan_pcp ) );
  }
  
  VALUE r_eth_type = rb_iv_get( r_match, "@eth_type" );
  if ( r_eth_type != Qnil ) {
    append_oxm_match_eth_type( match, ( uint16_t ) NUM2UINT( r_eth_type ) );
  }
  
  VALUE r_ip_dscp = rb_iv_get( r_match, "@ip_dscp" );
  if ( r_ip_dscp != Qnil ) {
    append_oxm_match_ip_dscp( match, ( uint8_t ) NUM2UINT( r_ip_dscp ) );
  }

  VALUE r_ip_ecn = rb_iv_get( r_match, "@ip_ecn" );
  if( r_ip_ecn != Qnil ) {
    append_oxm_match_ip_ecn( match, ( uint8_t ) NUM2UINT( r_ip_ecn ) );
  }

  VALUE r_ip_proto = rb_iv_get( r_match, "@ip_proto" );
  if ( r_ip_proto != Qnil ) {
    append_oxm_match_ip_proto( match, ( uint8_t ) NUM2UINT( r_ip_proto ) );
  }

  VALUE r_ipv4_src = rb_iv_get( r_match, "@ipv4_src" );
  if ( r_ipv4_src != Qnil ) {
    append_oxm_match_ipv4_src( match, nw_addr_to_i( r_ipv4_src ), 0 );
  }

  VALUE r_ipv4_dst = rb_iv_get( r_match, "@ipv4_dst" );
  if ( r_ipv4_dst != Qnil ) {
    append_oxm_match_ipv4_dst( match, nw_addr_to_i( r_ipv4_dst ), 0 );
  }

  struct in6_addr tmp_ipv6_mask;
  memset( &tmp_ipv6_mask, 0, sizeof( tmp_ipv6_mask ) );

  VALUE r_ipv6_src = rb_iv_get( r_match, "@ipv6_src" );
  if ( r_ipv6_src != Qnil ) {
    append_oxm_match_ipv6_src( match, ipv6_addr_to_in6_addr( r_ipv6_src ), tmp_ipv6_mask );
  }

  VALUE r_ipv6_dst = rb_iv_get( r_match, "@ipv6_dst" );
  if ( r_ipv6_dst != Qnil ) {
    append_oxm_match_ipv6_dst( match, ipv6_addr_to_in6_addr( r_ipv6_dst ), tmp_ipv6_mask );
  }

  VALUE r_ipv6_flabel = rb_iv_get( r_match, "@ipv6_flabel" );
  if ( r_ipv6_flabel != Qnil ) {
    append_oxm_match_ipv6_flabel( match, NUM2UINT( r_ipv6_flabel ), 0 );
  }

  VALUE r_ipv6_exthdr = rb_iv_get( r_match, "@ipv6_exthdr" );
  if ( r_ipv6_exthdr != Qnil ) {
    append_oxm_match_ipv6_exthdr( match , ( uint16_t ) NUM2UINT( r_ipv6_exthdr ), 0 );
  }

  VALUE r_arp_op = rb_iv_get( r_match, "@arp_op" );
  if ( r_arp_op != Qnil ) {
    append_oxm_match_arp_op( match, ( uint16_t ) NUM2UINT( r_arp_op ) );
  }

  VALUE r_arp_spa = rb_iv_get( r_match, "@arp_spa" );
  if ( r_arp_spa != Qnil ) {
    append_oxm_match_arp_spa( match, nw_addr_to_i( r_arp_spa  ), 0 );
  }

  VALUE r_arp_tpa = rb_iv_get( r_match, "@arp_tpa" );
  if ( r_arp_tpa != Qnil ) {
    append_oxm_match_arp_tpa( match, nw_addr_to_i( r_arp_tpa ), 0 );
  }

  VALUE r_arp_sha = rb_iv_get( r_match, "@arp_sha" );
  if ( r_arp_sha != Qnil ) {
    append_oxm_match_arp_sha( match, mac_addr_to_cstr( r_arp_sha ), tmp_mac_mask );
  }

  VALUE r_arp_tha = rb_iv_get( r_match, "@arp_tha" );
  if ( r_arp_tha != Qnil ) {
    append_oxm_match_arp_tha( match, mac_addr_to_cstr( r_arp_tha ), tmp_mac_mask );
  }

  VALUE r_mpls_label = rb_iv_get( r_match, "@mpls_label" );
  if ( r_mpls_label != Qnil ) {
    append_oxm_match_mpls_label( match, NUM2UINT( r_mpls_label ) );
  }

  VALUE r_mpls_tc = rb_iv_get( r_match, "@mpls_tc" );
  if ( r_mpls_tc != Qnil ) {
    append_oxm_match_mpls_tc( match, ( uint8_t ) NUM2UINT( r_mpls_tc ) );
  }

  VALUE r_mpls_bos = rb_iv_get( r_match, "@mpls_bos" );
  if ( r_mpls_bos != Qnil ) {
    append_oxm_match_mpls_bos( match, ( uint8_t ) NUM2UINT( r_mpls_bos ) );
  }

  VALUE r_icmpv4_type = rb_iv_get( r_match, "@icmpv4_type" );
  if ( r_icmpv4_type != Qnil ) {
    append_oxm_match_icmpv4_type( match, ( uint8_t ) NUM2UINT( r_icmpv4_type ) );
  }

  VALUE r_icmpv4_code = rb_iv_get( r_match, "@icmpv4_code" );
  if ( r_icmpv4_code != Qnil ) {
    append_oxm_match_icmpv4_code( match, ( uint8_t ) NUM2UINT( r_icmpv4_code ) );
  }

  VALUE r_tcp_src = rb_iv_get( r_match, "@tcp_src" );
  if ( r_tcp_src != Qnil ) {
    append_oxm_match_tcp_src( match, ( uint16_t ) NUM2UINT( r_tcp_src ) );
  }

  VALUE r_tcp_dst = rb_iv_get( r_match, "@tcp_dst" );
  if ( r_tcp_dst != Qnil ) {
    append_oxm_match_tcp_dst( match, ( uint16_t ) NUM2UINT( r_tcp_dst ) );
  }

  VALUE r_udp_src = rb_iv_get( r_match, "@udp_src" );
  if ( r_udp_src != Qnil ) {
    append_oxm_match_udp_src( match, ( uint16_t ) NUM2UINT( r_udp_src ) );
  }

  VALUE r_udp_dst = rb_iv_get( r_match, "@udp_dst" );
  if ( r_udp_dst != Qnil ) {
    append_oxm_match_udp_dst( match, ( uint16_t ) NUM2UINT( r_udp_dst ) );
  }

  VALUE r_sctp_src = rb_iv_get( r_match, "@sctp_src" );
  if ( r_sctp_src != Qnil ) {
    append_oxm_match_sctp_src( match, ( uint16_t ) NUM2UINT( r_sctp_src ) );
  }

  VALUE r_sctp_dst = rb_iv_get( r_match, "@sctp_dst" );
  if ( r_sctp_dst != Qnil ) {
    append_oxm_match_sctp_dst( match, ( uint16_t ) NUM2UINT( r_sctp_dst ) );
  }


  VALUE r_icmpv6_type = rb_iv_get( r_match, "@icmpv6_type" );
  if ( r_icmpv6_type != Qnil ) {
    uint8_t icmpv6_type = ( uint8_t ) NUM2UINT( r_icmpv6_type );
    append_oxm_match_icmpv6_type( match, icmpv6_type );

    VALUE r_icmpv6_code = rb_iv_get( r_match, "@icmpv6_code" );
    if ( r_icmpv6_code != Qnil ) {
      append_oxm_match_icmpv6_code( match, ( uint8_t ) NUM2UINT( r_icmpv6_code ) );
    }

    if ( icmpv6_type == 135 || icmpv6_type == 136 ) {
      VALUE r_ipv6_nd_target = rb_iv_get( r_match, "@ipv6_nd_target" );
      if ( r_ipv6_nd_target != Qnil ) {
        append_oxm_match_ipv6_nd_target( match, ipv6_addr_to_in6_addr( r_ipv6_nd_target ) );
      }

      VALUE r_ipv6_nd_sll = rb_iv_get( r_match, "@ipv6_nd_sll" );
      if ( r_ipv6_nd_sll != Qnil ) {
        append_oxm_match_ipv6_nd_sll( match, mac_addr_to_cstr( r_ipv6_nd_sll ) );
      }

      VALUE r_ipv6_nd_tll = rb_iv_get( r_match, "@ipv6_nd_tll" );
      if ( r_ipv6_nd_tll != Qnil ) {
        append_oxm_match_ipv6_nd_tll( match, mac_addr_to_cstr( r_ipv6_nd_tll ) );
      }
    }
  }
}



/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */

