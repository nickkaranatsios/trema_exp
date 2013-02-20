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


struct in6_addr
ipv6_addr_to_in6_addr( VALUE ipv6_addr ) {
  struct in6_addr in6_addr;

  VALUE ipv6_addr_str = rb_funcall( ipv6_addr, rb_intern( "to_s" ), 0 );
  const char *dst = rb_string_value_cstr( &ipv6_addr_str );
  inet_pton( AF_INET6, dst, &in6_addr );
  return in6_addr;
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
  VALUE str = rb_inspect( options );
  printf( "match options %s\n", StringValuePtr( str ) );
  return rb_funcall( rb_eval_string( "Match" ), rb_intern( "new" ), 1, options );
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

