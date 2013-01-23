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


extern VALUE mTrema;
VALUE mMatchSet;


static openflow_actions *
openflow_actions_ptr( VALUE self ) {
  openflow_actions *actions;
  Data_Get_Struct( self, openflow_actions, actions );
  return actions;
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
append_match_eth_dst( VALUE self, VALUE r_actions, VALUE mac_address ) {
  uint8_t eth_dst[ OFP_ETH_ALEN ];
  const uint8_t *ptr = ( const uint8_t * ) dl_addr_to_a( mac_address, eth_dst );
  append_action_set_field_eth_dst( openflow_actions_ptr( r_actions ), ptr );
  return self;
}


static VALUE
append_match_eth_src( VALUE self, VALUE r_actions, VALUE mac_address ) {
  uint8_t eth_src[ OFP_ETH_ALEN ];
  const uint8_t *ptr = ( const uint8_t * ) dl_addr_to_a( mac_address, eth_src );
  append_action_set_field_eth_src( openflow_actions_ptr( r_actions ), ptr );
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
  const uint32_t ip_address = ( const uint32_t ) nw_addr_to_i( ipv4_src_addr );
  append_action_set_field_ipv4_src( openflow_actions_ptr( r_actions ), ip_address );
  return self;
}


static VALUE
append_match_ipv4_dst_addr( VALUE self, VALUE r_actions, VALUE ipv4_dst_addr ) {
  const uint32_t ip_address = ( const uint32_t ) nw_addr_to_i( ipv4_dst_addr );
  append_action_set_field_ipv4_dst( openflow_actions_ptr( r_actions ), ip_address );
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
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
