/*
 * Copyright (C) 2008-2012 NEC Corporation
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
#include <arpa/inet.h>


extern VALUE mTrema;
VALUE mMatchSet;


static openflow_actions *
openflow_actions_ptr( VALUE self ) {
  openflow_actions *actions;
  Data_Get_Struct( self, openflow_actions, actions );
  return actions;
}


static const uint8_t *
mac_addr_to_cstr( VALUE mac_addr ) {
  uint8_t dl_addr[ OFP_ETH_ALEN ];
  return ( const uint8_t * ) dl_addr_to_a( mac_addr, dl_addr );
}


static uint32_t
ip_addr_to_i( VALUE ip_addr ) {
  return nw_addr_to_i( ip_addr );
}


static struct in6_addr
ipv6_addr_to_in6_addr( VALUE ipv6_addr ) {
  struct in6_addr in6_addr;

  VALUE ipv6_addr_str = rb_funcall( ipv6_addr, rb_intern( "to_s" ), 0 );
  const char *dst = rb_string_value_cstr( &ipv6_addr_str );
  inet_pton( AF_INET6, dst, &in6_addr );
  return in6_addr;
}
  

static VALUE
append_match_in_port( VALUE self, VALUE r_actions, VALUE in_port ) {
  append_action_set_field_in_port( openflow_actions_ptr( r_actions ), NUM2UINT( in_port ) );
  return self;
}


static VALUE
append_match_in_phy_port( VALUE self, VALUE r_actions, VALUE in_phy_port ) {
  append_action_set_field_in_phy_port( openflow_actions_ptr( r_actions ), NUM2UINT( in_phy_port ) );
  return self;
}


static VALUE
append_match_metadata( VALUE self, VALUE r_actions, VALUE metadata ) {
  append_action_set_field_metadata( openflow_actions_ptr( r_actions ), rb_num2ull( metadata ) );
  return self;
}


static VALUE
append_match_eth_dst( VALUE self, VALUE r_actions, VALUE eth_dst ) {
  append_action_set_field_eth_dst( openflow_actions_ptr( r_actions ), mac_addr_to_cstr( eth_dst ) );
  return self;
}


static VALUE
append_match_eth_src( VALUE self, VALUE r_actions, VALUE mac_address ) {
  append_action_set_field_eth_src( openflow_actions_ptr( r_actions ), mac_addr_to_cstr( mac_address ) );
  return self;
}


static VALUE
append_match_ether_type( VALUE self, VALUE r_actions, VALUE ether_type ) {
  append_action_set_field_eth_type( openflow_actions_ptr( r_actions ), ( const uint16_t ) NUM2UINT( ether_type ) );
  return self;
}


static VALUE
append_match_vlan_vid( VALUE self, VALUE r_actions, VALUE vlan_vid ) {
  append_action_set_field_vlan_vid( openflow_actions_ptr( r_actions ), ( const uint16_t ) NUM2UINT( vlan_vid ) );
  return self;
}


static VALUE
append_match_vlan_priority( VALUE self, VALUE r_actions, VALUE vlan_priority ) {
  append_action_set_field_vlan_pcp( openflow_actions_ptr( r_actions ), ( const uint8_t ) NUM2UINT( vlan_priority ) );
  return self;
}


static VALUE
append_match_ip_dscp( VALUE self, VALUE r_actions, VALUE ip_dscp ) {
  append_action_set_field_ip_dscp( openflow_actions_ptr( r_actions ), ( const uint8_t ) NUM2UINT( ip_dscp ) );
  return self;
}


static VALUE
append_match_ip_ecn( VALUE self, VALUE r_actions, VALUE ip_ecn ) {
  append_action_set_field_ip_ecn( openflow_actions_ptr( r_actions ), ( const uint8_t ) NUM2UINT( ip_ecn ) );
  return self;
}


static VALUE
append_match_ip_proto( VALUE self, VALUE r_actions, VALUE ip_proto ) {
  append_action_set_field_ip_proto( openflow_actions_ptr( r_actions ), ( const uint8_t ) NUM2UINT( ip_proto ) );
  return self;
}


static VALUE
append_match_ipv4_src_addr( VALUE self, VALUE r_actions, VALUE ipv4_src_addr ) {
  append_action_set_field_ipv4_src( openflow_actions_ptr( r_actions ), ip_addr_to_i( ipv4_src_addr ) );
  return self;
}


static VALUE
append_match_ipv4_dst_addr( VALUE self, VALUE r_actions, VALUE ipv4_dst_addr ) {
  append_action_set_field_ipv4_dst( openflow_actions_ptr( r_actions ), ip_addr_to_i( ipv4_dst_addr ) );
  return self;
}


static VALUE
append_match_tcp_src_port( VALUE self, VALUE r_actions, VALUE tcp_src_port ) {
  append_action_set_field_tcp_src( openflow_actions_ptr( r_actions ), ( const uint16_t ) NUM2UINT( tcp_src_port ) );
  return self;
}


static VALUE
append_match_tcp_dst_port( VALUE self, VALUE r_actions, VALUE tcp_dst_port ) {
  append_action_set_field_tcp_dst( openflow_actions_ptr( r_actions ), ( const uint16_t ) NUM2UINT( tcp_dst_port ) );
  return self;
}


static VALUE
append_match_udp_src_port( VALUE self, VALUE r_actions, VALUE udp_src_port ) {
  append_action_set_field_udp_src( openflow_actions_ptr( r_actions ), ( const uint16_t ) NUM2UINT( udp_src_port ) );
  return self;
}


static VALUE
append_match_udp_dst_port( VALUE self, VALUE r_actions, VALUE udp_dst_port ) {
  append_action_set_field_udp_dst( openflow_actions_ptr( r_actions ), ( const uint16_t ) NUM2UINT( udp_dst_port ) );
  return self;
}


static VALUE
append_match_sctp_src_port( VALUE self, VALUE r_actions, VALUE sctp_src_port ) {
  append_action_set_field_sctp_src( openflow_actions_ptr( r_actions ), ( const uint16_t ) NUM2UINT( sctp_src_port ) );
  return self;
}


static VALUE
append_match_sctp_dst_port( VALUE self, VALUE r_actions, VALUE sctp_dst_port ) {
  append_action_set_field_sctp_dst( openflow_actions_ptr( r_actions ), ( const uint16_t ) NUM2UINT( sctp_dst_port ) );
  return self;
}


static VALUE
append_match_icmpv4_type( VALUE self, VALUE r_actions, VALUE icmpv4_type ) {
  append_action_set_field_icmpv4_type( openflow_actions_ptr( r_actions ), ( const uint8_t ) NUM2UINT( icmpv4_type ) );
  return self;
}


static VALUE
append_match_icmpv4_code( VALUE self, VALUE r_actions, VALUE icmpv4_code ) {
  append_action_set_field_icmpv4_code( openflow_actions_ptr( r_actions ), ( const uint8_t ) NUM2UINT( icmpv4_code ) );
  return self;
}


static VALUE
append_match_arp_op( VALUE self, VALUE r_actions, VALUE arp_op ) {
  append_action_set_field_arp_op( openflow_actions_ptr( r_actions ), ( const uint16_t ) NUM2UINT( arp_op ) );
  return self;
}


static VALUE
append_match_arp_spa( VALUE self, VALUE r_actions, VALUE arp_spa ) {
  append_action_set_field_arp_spa( openflow_actions_ptr( r_actions ), ip_addr_to_i( arp_spa ) );
  return self;
}


static VALUE
append_match_arp_tpa( VALUE self, VALUE r_actions, VALUE arp_tpa ) {
  append_action_set_field_arp_tpa( openflow_actions_ptr( r_actions ), ip_addr_to_i( arp_tpa ) );
  return self;
}


static VALUE
append_match_arp_sha( VALUE self, VALUE r_actions, VALUE arp_sha ) {
  append_action_set_field_arp_sha( openflow_actions_ptr( r_actions ), mac_addr_to_cstr( arp_sha ) );
  return self;
}


static VALUE
append_match_arp_tha( VALUE self, VALUE r_actions, VALUE arp_tha ) {
  append_action_set_field_arp_tha( openflow_actions_ptr( r_actions ), mac_addr_to_cstr( arp_tha ) );
  return self;
}

  
static VALUE
append_match_ipv6_src_addr( VALUE self, VALUE r_actions, VALUE ipv6_src_addr ) {
  append_action_set_field_ipv6_src( openflow_actions_ptr( r_actions ), ipv6_addr_to_in6_addr( ipv6_src_addr ) ); 
  return self;
}


static VALUE
append_match_ipv6_dst_addr( VALUE self, VALUE r_actions, VALUE ipv6_dst_addr ) {
  append_action_set_field_ipv6_dst( openflow_actions_ptr( r_actions ), ipv6_addr_to_in6_addr( ipv6_dst_addr ) ); 
  return self;
}


static VALUE
append_match_ipv6_flow_label( VALUE self, VALUE r_actions, VALUE ipv6_flow_label ) {
  append_action_set_field_ipv6_flabel( openflow_actions_ptr( r_actions ), NUM2UINT( ipv6_flow_label ) );
  return self;
}


static VALUE
append_match_icmpv6_type( VALUE self, VALUE r_actions, VALUE icmpv6_type ) {
  append_action_set_field_icmpv6_type( openflow_actions_ptr( r_actions ), ( const uint8_t ) NUM2UINT( icmpv6_type ) );
  return self;
}


static VALUE
append_match_icmpv6_code( VALUE self, VALUE r_actions, VALUE icmpv6_code ) {
  append_action_set_field_icmpv6_code( openflow_actions_ptr( r_actions ), ( const uint8_t ) NUM2UINT( icmpv6_code ) );
  return self;
}


static VALUE
append_match_ipv6_nd_target( VALUE self, VALUE r_actions, VALUE ipv6_nd_target ) {
  append_action_set_field_ipv6_nd_target( openflow_actions_ptr( r_actions ), ipv6_addr_to_in6_addr( ipv6_nd_target ) );
  return self;
}


static VALUE
append_match_ipv6_nd_sll( VALUE self, VALUE r_actions, VALUE ipv6_nd_sll ) {
  append_action_set_field_ipv6_nd_sll( openflow_actions_ptr( r_actions ), mac_addr_to_cstr( ipv6_nd_sll ) );
  return self;
}


static VALUE
append_match_ipv6_nd_tll( VALUE self, VALUE r_actions, VALUE ipv6_nd_tll ) {
  append_action_set_field_ipv6_nd_tll( openflow_actions_ptr( r_actions ), mac_addr_to_cstr( ipv6_nd_tll ) );
  return self;
}


static VALUE
append_match_mpls_label( VALUE self, VALUE r_actions, VALUE mpls_label ) {
  append_action_set_field_mpls_label( openflow_actions_ptr( r_actions ), NUM2UINT( mpls_label ) );
  return self;
}


static VALUE
append_match_mpls_tc( VALUE self, VALUE r_actions, VALUE mpls_tc ) {
  append_action_set_field_mpls_tc( openflow_actions_ptr( r_actions ), ( const uint8_t ) NUM2UINT( mpls_tc ) );
  return self;
}


static VALUE
append_match_mpls_bos( VALUE self, VALUE r_actions, VALUE mpls_bos ) {
  append_action_set_field_mpls_bos( openflow_actions_ptr( r_actions ), ( const uint8_t ) NUM2UINT( mpls_bos ) );
  return self;
}


static VALUE
append_match_pbb_isid( VALUE self, VALUE r_actions, VALUE pbb_isid ) {
  append_action_set_field_pbb_isid( openflow_actions_ptr( r_actions ), NUM2UINT( pbb_isid ) );
  return self;
}


static VALUE
append_match_tunnel_id( VALUE self, VALUE r_actions, VALUE tunnel_id ) {
  append_action_set_field_tunnel_id( openflow_actions_ptr( r_actions ), rb_num2ull( tunnel_id ) );
  return self;
}


static VALUE
append_match_ipv6_exthdr( VALUE self, VALUE r_actions, VALUE ipv6_exthdr ) {
  append_action_set_field_ipv6_exthdr( openflow_actions_ptr( r_actions ), ( const uint16_t ) NUM2UINT( ipv6_exthdr ) );
  return self;
}


void
Init_match_set() {
  mMatchSet = rb_define_module_under( mTrema, "MatchSet" );
  rb_define_module_function( mMatchSet, "append_match_in_port", append_match_in_port, 2 );
  rb_define_module_function( mMatchSet, "append_match_in_phy_port", append_match_in_phy_port, 2 );
  rb_define_module_function( mMatchSet, "append_match_metadata", append_match_metadata, 2 );
  rb_define_module_function( mMatchSet, "append_match_eth_dst", append_match_eth_dst, 2 );
  rb_define_module_function( mMatchSet, "append_match_eth_src", append_match_eth_src, 2 );
  rb_define_module_function( mMatchSet, "append_match_ether_type", append_match_ether_type, 2 );
  rb_define_module_function( mMatchSet, "append_match_vlan_vid", append_match_vlan_vid, 2 );
  rb_define_module_function( mMatchSet, "append_match_vlan_priority", append_match_vlan_priority, 2 );
  rb_define_module_function( mMatchSet, "append_match_ip_dscp", append_match_ip_dscp, 2 );
  rb_define_module_function( mMatchSet, "append_match_ip_ecn", append_match_ip_ecn, 2 );
  rb_define_module_function( mMatchSet, "append_match_ip_proto", append_match_ip_proto, 2 );
  rb_define_module_function( mMatchSet, "append_match_ipv4_src_addr", append_match_ipv4_src_addr, 2 );
  rb_define_module_function( mMatchSet, "append_match_ipv4_dst_addr", append_match_ipv4_dst_addr, 2 );
  rb_define_module_function( mMatchSet, "append_match_tcp_src_port", append_match_tcp_src_port, 2 );
  rb_define_module_function( mMatchSet, "append_match_tcp_dst_port", append_match_tcp_dst_port, 2 );
  rb_define_module_function( mMatchSet, "append_match_udp_src_port", append_match_udp_src_port, 2 );
  rb_define_module_function( mMatchSet, "append_match_udp_dst_port", append_match_udp_dst_port, 2 );
  rb_define_module_function( mMatchSet, "append_match_sctp_src_port", append_match_sctp_src_port, 2 );
  rb_define_module_function( mMatchSet, "append_match_sctp_dst_port", append_match_sctp_dst_port, 2 );
  rb_define_module_function( mMatchSet, "append_match_icmpv4_type", append_match_icmpv4_type, 2 );
  rb_define_module_function( mMatchSet, "append_match_icmpv4_code", append_match_icmpv4_code, 2 );
  rb_define_module_function( mMatchSet, "append_match_arp_op", append_match_arp_op, 2 );
  rb_define_module_function( mMatchSet, "append_match_arp_spa", append_match_arp_spa, 2 );
  rb_define_module_function( mMatchSet, "append_match_arp_tpa", append_match_arp_tpa, 2 );
  rb_define_module_function( mMatchSet, "append_match_arp_sha", append_match_arp_sha, 2 );
  rb_define_module_function( mMatchSet, "append_match_arp_tha", append_match_arp_tha, 2 );
  rb_define_module_function( mMatchSet, "append_match_ipv6_src_addr", append_match_ipv6_src_addr, 2 );
  rb_define_module_function( mMatchSet, "append_match_ipv6_dst_addr", append_match_ipv6_dst_addr, 2 );
  rb_define_module_function( mMatchSet, "append_match_ipv6_flow_label", append_match_ipv6_flow_label, 2 );
  rb_define_module_function( mMatchSet, "append_match_icmpv6_type", append_match_icmpv6_type, 2 );
  rb_define_module_function( mMatchSet, "append_match_icmpv6_code", append_match_icmpv6_code, 2 );
  rb_define_module_function( mMatchSet, "append_match_ipv6_nd_target", append_match_ipv6_nd_target, 2 );
  rb_define_module_function( mMatchSet, "append_match_ipv6_nd_sll", append_match_ipv6_nd_sll, 2 );
  rb_define_module_function( mMatchSet, "append_match_ipv6_nd_tll", append_match_ipv6_nd_tll, 2 );
  rb_define_module_function( mMatchSet, "append_match_mpls_label", append_match_mpls_label, 2 );
  rb_define_module_function( mMatchSet, "append_match_mpls_tc", append_match_mpls_tc, 2 );
  rb_define_module_function( mMatchSet, "append_match_mpls_bos", append_match_mpls_bos, 2 );
  rb_define_module_function( mMatchSet, "append_match_pbb_isid", append_match_pbb_isid, 2 );
  rb_define_module_function( mMatchSet, "append_match_tunnel_id", append_match_tunnel_id, 2 );
  rb_define_module_function( mMatchSet, "append_match_ipv6_exthdr", append_match_ipv6_exthdr, 2 );
  rb_require( "trema/match-in-port" );
  rb_require( "trema/match-in-phy-port" );
  rb_require( "trema/match-metadata" );
  rb_require( "trema/match-eth-dst" );
  rb_require( "trema/match-eth-src" );
  rb_require( "trema/match-ether-type" );
  rb_require( "trema/match-vlan-vid" );
  rb_require( "trema/match-vlan-priority" );
  rb_require( "trema/match-ip-dscp" );
  rb_require( "trema/match-ip-ecn" );
  rb_require( "trema/match-ip-proto" );
  rb_require( "trema/match-ipv4-src-addr" );
  rb_require( "trema/match-ipv4-dst-addr" );
  rb_require( "trema/match-tcp-src-port" );
  rb_require( "trema/match-tcp-dst-port" );
  rb_require( "trema/match-udp-src-port" );
  rb_require( "trema/match-udp-dst-port" );
  rb_require( "trema/match-sctp-src-port" );
  rb_require( "trema/match-sctp-dst-port" );
  rb_require( "trema/match-icmpv4-type" );
  rb_require( "trema/match-icmpv4-code" );
  rb_require( "trema/match-arp-op" );
  rb_require( "trema/match-arp-spa" );
  rb_require( "trema/match-arp-tpa" );
  rb_require( "trema/match-arp-sha" );
  rb_require( "trema/match-arp-tha" );
  rb_require( "trema/match-ipv6-src-addr" );
  rb_require( "trema/match-ipv6-dst-addr" );
  rb_require( "trema/match-ipv6-flow-label" );
  rb_require( "trema/match-icmpv6-type" );
  rb_require( "trema/match-icmpv6-code" );
  rb_require( "trema/match-ipv6-nd-target" );
  rb_require( "trema/match-ipv6-nd-sll" );
  rb_require( "trema/match-ipv6-nd-tll" );
  rb_require( "trema/match-mpls-label" );
  rb_require( "trema/match-mpls-tc" );
  rb_require( "trema/match-mpls-bos" );
  rb_require( "trema/match-pbb-isid" );
  rb_require( "trema/match-tunnel-id" );
  rb_require( "trema/match-ipv6-exthdr" );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
