/*
 * Author: Yasuhito Takamiya <yasuhito@gmail.com>
 *
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


#include "barrier-reply.h"
#include "buffer.h"
#include "controller.h"
#include "features-reply.h"
#include "flow-removed.h"
#include "get-config-reply.h"
#include "list-switches-reply.h"
#include "logger.h"
#include "openflow-error.h"
#include "openflow.h"
#include "packet-in.h"
#include "port-status.h"
#include "queue-get-config-reply.h"
#include "ruby.h"
#include "rubysig.h"
#include "stats-reply.h"
#include "switch-disconnected.h"
#include "trema.h"
#include "vendor.h"


extern VALUE mTrema;
VALUE cController;


static void
handle_timer_event( void *self ) {
  if ( rb_respond_to( ( VALUE ) self, rb_intern( "handle_timer_event" ) ) == Qtrue ) {
    rb_funcall( ( VALUE ) self, rb_intern( "handle_timer_event" ), 0 );
  }
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
controller_send_message( VALUE self, VALUE datapath_id, VALUE message ) {
  buffer *buf;
  Data_Get_Struct( message, buffer, buf );
  send_openflow_message( NUM2ULL( datapath_id ), buf );
  return self;
}


static VALUE
controller_send_list_switches_request( VALUE self ) {
  send_list_switches_request( ( void * ) self );
  return self;
}

static void
append_action_set( openflow_actions *actions, VALUE field ) {
  if ( rb_funcall( field, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::MatchInPort" ) ) == Qtrue ) {
    const uint32_t in_port = NUM2UINT( rb_funcall( field, rb_intern( "in_port" ) , 0 ) );
    append_action_set_field_in_port( actions, in_port );
  }
  else if ( rb_funcall( actions, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::MatchInPhyPort" ) ) == Qtrue ) {
    const uint32_t in_phy_port = NUM2UINT( rb_funcall( field, rb_intern( "in_phy_port" ), 0 ) );
    append_action_set_field_in_phy_port( actions, in_phy_port );
  }
  else if ( rb_funcall( actions, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::MatchMetadata" ) ) == Qtrue ) {
    const uint64_t metadata = NUM2ULONG( rb_funcall( field, rb_intern( "metadata" ), 0 ) );
    append_action_set_field_metadata( actions, metadata );
  }
  else if ( rb_funcall( actions, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::MatchEthDst" ) ) == Qtrue ) {
    uint8_t dl_dst[ OFP_ETH_ALEN ];
    const uint8_t *ptr = ( const uint8_t * ) dl_addr_to_a( rb_funcall( action, rb_intern( "mac_address" ), 0 ), dl_dst );
    append_action_set_field_eth_dst( actions, ptr );
  }
  else if ( rb_funcall( actions, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::MatchEthSrc" ) ) == Qtrue ) {
    uint8_t dl_src[ OFP_ETH_ALEN ];
    const uint8_t *ptr = ( const uint8_t * ) dl_addr_to_a( rb_funcall( action, rb_intern( "mac_address" ), 0 ), dl_src );
    append_action_set_field_set_eth_src( actions, ptr );
  }
  else if ( rb_funcall( actions, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::MatchEthType" ) ) == Qtrue ) {
    const uint16_t ether_type = ( const uint16_t ) NUM2UINT( rb_funcall( field, rb_intern( "ether_type" ), 0 ) );
    append_action_set_field_ether_type( actions, ether_type );
  }
  else if ( rb_funcall( actions, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::MatchVlanVid" ) ) == Qtrue ) {
    const uint16_t vlan_vid = ( const uint16_t ) NUM2UINT( rb_funcall( field, rb_intern( "vlan_vid" ), 0 ) );
    append_action_set_field_vlan_vlid( actions, vlan_vid );
  }
  else if ( rb_funcall( actions, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::MatchVlanPriority" ) ) == Qtrue ) {
    const uint8_t vlan_priority = ( const uint8_t ) NUM2UINT( rb_funcall( field, rb_intern( "vlan_priority" ), 0 ) );
    append_action_set_field_vlan_pcp( actions, vlan_priority );
  }
  else if ( rb_funcall( actions, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::MatchIpDscp" ) ) == Qtrue ) {
    const uint8_t ip_dscp = ( const uint8_t ) NUM2UINT( rb_funcall( field, rb_intern( "ip_dscp" ), 0 ) );
    append_action_set_field_ip_dscp( actions, ip_dscp );
  }
  else if ( rb_funcall( actions, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::MatchIpEcn" ) ) == Qtrue ) {
    const uint8_t ip_ecn = ( const uint8_t ) NUM2UINT( rb_funcall( field, rb_intern( "ip_ecn" ), 0 ) );
    append_action_set_field_ip_ecn( actions, ip_ecn );
  }
  else if ( rb_funcall( actions, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::MatchIpProto" ) ) == Qtrue ) {
    const uint8_t ip_proto = ( const uint8_t ) NUM2UINT( rb_funcall( field, rb_intern( "ip_proto" ), 0 ) );
    append_action_set_field_ip_proto( actions, ip_proto );
  }
  else if ( rb_funcall( actions, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::MatchIpv4SrcAddr" ) ) == Qtrue ) {
    const uint32_t ip_address = ( const uint32_t ) nw_addr_to_i( rb_funcall( field, rb_intern( "ip_address" ), 0 ) );
    append_action_set_field_ipv4_src( actions, ip_address );
  }
  else if ( rb_funcall( actions, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::MatchIpv4DstAddr" ) ) == Qtrue ) {
    const uint32_t ip_address = ( const uint32_t ) nw_addr_to_i( rb_funcall( field, rb_intern( "ip_address" ), 0 ) );
    append_action_set_field_ipv4_dst( actions, ip_address );
  }
  else if ( rb_funcall( actions, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::MatchTCPSrcPort" ) ) == Qtrue ) {
    const uint16_t port_number = ( const uint16_t ) NUM2UINT( rb_funcall( field, rb_intern( "port_number" ), 0 ) );
    append_action_set_field_tcp_src( actions, port_number );
  }
  else if ( rb_funcall( actions, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::MatchTCPDstPort" ) ) == Qtrue ) {
    const uint16_t port_number = ( const uint16_t ) NUM2UINT( rb_funcall( field, rb_intern( "port_number" ), 0 ) );
    append_action_set_field_tcp_dst( actions, port_number );
  }
  else if ( rb_funcall( actions, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::MatchUdpSrcPort" ) ) == Qtrue ) {
    const uint16_t port_number = ( const uint16_t ) NUM2UINT( rb_funcall( field, rb_intern( "port_number" ), 0 ) );
    append_action_set_field_udp_src( actions, port_nummber );
  }
  else if ( rb_funcall( actions, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::MatchUdpDstPort" ) ) == Qtrue ) {
    const uint16_t port_number = ( const uint16_t ) NUM2UINT( rb_funcall( field, rb_intern( "port_number" ), 0 ) );
    append_action_set_field_udp_dst( actions, port_number );
  }
  else if ( rb_funcall( actions, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::MatchSctpSrcPort" ) ) == Qtrue ) {
    const uint16_t port_number = ( const uint16_t ) NUM2UINT( rb_funcall( field, rb_intern( "port_number" ), 0 ) );
    append_action_set_field_sctp_src( actions, port_number );
  }
  else if ( rb_funcall( actions, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::MatchSctpDstPort" ) ) == Qtrue ) {
    const uint16_t port_number = ( const uint16_t ) NUM2UINT( rb_funcall( field, rb_intern( "port_number" ), 0 ) );
    append_action_set_field_sctp_dst( actions, port_number );
  }
  else if ( rb_funcall( actions, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::MatchIcmpv4Type" ) ) == Qtrue ) {
    const uint8_t icmpv4_type = ( const uint8_t ) NUM2UINT( rb_funcall( field, rb_intern( "icmpv4_type" ), 0 ) );
    append_action_set_field_icmpv4_type( actions, icmpv4_type );
  }
  else if ( rb_funcall( actions, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::MatchIcmpv4Code" ) ) == Qtrue ) {
    const uint8_t icmpv4_code = ( const uint8_t ) NUM2UINT( rb_funcall( field, rb_intern( "icmpv4_code" ), 0 ) );
    append_action_set_field_icmpv4_code( actions, icmpv4_code );
  }
  else if ( rb_funcall( actions, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::MatchArpOp" ) ) == Qtrue ) {
    const uint16_t arp_op = ( const uint16_t ) NUM2UINT( rb_funcall( field, rb_intern( "arp_op" ), 0 ) );
    append_action_set_field_arp_op( actions, arp_op );
  }
  else if ( rb_funcall( actions, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::MatchArpSpa" ) ) == Qtrue ) {
    const uint32_t arp_spa = NUM2UINT( rb_funcall( field, rb_intern( "arp_spa" ), 0 ) );
    append_action_set_field_arp_spa( actions, arp_spa );
  }
  else if ( rb_funcall( actions, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::MatchArpTpa" ) ) == Qtrue ) {
    const uint32_t arp_tpa = NUM2UINT( rb_funcall( field, rb_intern( "arp_tpa" ), 0 ) );
    append_action_set_field_arp_tpa( actions, arp_tpa );
  }
  else if ( rb_funcall( actions, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::MatchArpSha" ) ) == Qtrue ) {
    uint8_t arp_sha[ OFP_ETH_ALEN ];
    const uint8_t *ptr = ( const uint8_t * ) dl_addr_to_a( rb_funcall( action, rb_intern( "mac_address" ), 0 ), arp_sha );
    append_action_set_field_arp_sha( actions, ptr );
  }
  else if ( rb_funcall( actions, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::MatchArpTha" ) ) == Qtrue ) {
    uint8_t arp_tha[ OFP_ETH_ALEN ];
    const uint8_t *ptr = ( const uint8_t * ) dl_addr_to_a( rb_funcall( action, rb_intern( "mac_address" ), 0 ), arp_tha );
    append_action_set_field_arp_sha( actions, ptr );
  }
  else if ( rb_funcall( actions, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::MatchIpv6Src" ) ) == Qtrue ) {
    struct in6_addr in6_addr = {};
    inet_pton( AF_INET6, rb_funcall( field, rb_intern( "to_s" ), 0 ), &in6_addr );
    append_action_set_field_ipv6_src( actions, in6_addr );
  }
  else if ( rb_funcall( actions, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::MatchIpv6Dst" ) ) == Qtrue ) {
    struct in6_addr in6_addr = {};
    inet_pton( AF_INET6, rb_funcall( field, rb_intern( "to_s" ), 0 ), &in6_addr );
    append_action_set_field_ipv6_dst( actions, in6_addr ); 
  }
  else if ( rb_funcall( actions, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::MatchIpv6FlowLabel" ) ) == Qtrue ) {
    const uint32_t ipv6_flabel = NUM2UINT( rb_funcall( field, rb_intern( "ipv6_flabel" ), 0 ) );
    append_action_set_field_ipv6_flabel( actions, ipv6_flabel );
  }
  else if ( rb_funcall( actions, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::MatchIcmpv6Type" ) ) == Qtrue ) {
    const uint8_t icmpv6_type = ( const uint8_t ) NUM2UINT( rb_funcall( field, rb_intern( "icmpv6_type" ), 0 ) );
    append_action_set_field_icmpv6_type( actions, icmpv6_type );
  }
  else if ( rb_funcall( actions, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::MatchIcmpv6Code" ) ) == Qtrue ) {
    const uint8_t icmpv6_code = ( const uint8_t ) NUM2UINT( rb_funcall( field, rb_intern( "icmpv6_code" ), 0 ) );
    append_action_set_field_icmpv6_code( actions, icmpv6_code );
  }
  else if ( rb_funcall( actions, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::MatchIpv6NdTarget" ) ) == Qtrue ) {
    struct in6_addr in6_addr = {};
    inet_pton( AF_INET6, rb_funcall( field, rb_intern( "to_s" ), 0 ), &in6_addr );
    append_action_set_field_ipv6_nd_target( actions, in6_addr );
  }
  else if ( rb_funcall( actions, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::MatchIpv6NdSll" ) ) == Qtrue ) {
    uint8_t ipv6_nd_sll[ OFP_ETH_ALEN ];
    const uint8_t *ptr = ( const uint8_t * ) dl_addr_to_a( rb_funcall( action, rb_intern( "mac_address" ), 0 ), ipv6_nd_sll );
    append_action_set_field_ipv6_nd_sll( actions, ptr );
  }
  else if ( rb_funcall( actions, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::MatchIpv6NdTll" ) ) == Qtrue ) {
    uint8_t ipv6_nd_tll[ OFP_ETH_ALEN ];
    const uint8_t *ptr = ( const uint8_t * ) dl_addr_to_a( rb_funcall( action, rb_intern( "mac_address" ), 0 ), ipv6_nd_tll );
    append_action_set_field_ipv6_nd_tll( actions, ptr );
  }
  else if ( rb_funcall( actions, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::MatchMplsLabel" ) ) == Qtrue ) {
    uint32_t mpls_label = NUM2UINT( rb_funcall( field, rb_intern( "mpls_label" ), 0 ) );
    append_action_set_field_mpls_label( actions, mpls_label );
  }
  else if ( rb_funcall( actions, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::MatchMplsTrafficClass" ) ) == Qtrue ) {
    const uint8_t mpls_tc = ( const uint8_t ) NUM2UINT( rb_funcall( field, rb_intern( "mpls_tc" ), 0 ) );
    append_action_set_field_mpls_tc( actions, mpls_tc );
  }
  else if ( rb_funcall( actions, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::MatchMplsBos" ) ) == Qtrue ) {
    const uint8_t mpls_bos = ( const uint8_t ) NUM2UINT( rb_funcall( field, rb_intern( "mpls_bos" ), 0 ) );
    append_action_set_field_mpls_bos( actions, mpls_bos );
  }
  else if ( rb_funcall( actions, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::MatchPbbIsid" ) ) == Qtrue ) {
    const uint32_t pbb_isid = NUM2UINT( rb_funcall( field, rb_intern( "pbb_isid" ), 0 ) );
    append_action_set_field_pbb_isid( actions, pbb_isid );
  }
  else if ( rb_funcall( actions, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::MatchTunnelId" ) ) == Qtrue ) {
    const uint64_t tunnel_id = NUM2ULONG( rb_funcall( field, rb_intern( "tunnel_id" ), 0 ) );
    append_action_set_field_tunnel_id( actions, tunnel_id );
  }
  else if ( rb_funcall( actions, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::MatchIpv6ExtHdr" ) ) == Qtrue ) {
    const uint16_t ipv6_exthdr = ( const uint16_t ) NUM2UINT( rb_funcall( field, rb_intern( "ipv6_exthdr" ), 0 ) );
    append_action_set_field_ipv6_extdhr( actions, ipv6_exthdr );
  }
  else {
    rb_raise( rb_eTypeError, "match field argument must be an Array of MatchField objects" );
  }
}

static void
append_action( openflow_actions *actions, VALUE action ) {
  if ( rb_funcall( action, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::SendOutPort" ) ) == Qtrue ) {
    const uint16_t port_number = ( const uint16_t ) NUM2UINT( rb_funcall( action, rb_intern( "port_number" ), 0 ) );
    const uint16_t max_len = ( const uint16_t ) NUM2UINT( rb_funcall( action, rb_intern( "max_len" ), 0 ) );
    append_action_output( actions, port_number, max_len );
  }
  else if ( rb_funcall( action, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::GroupAction" ) ) == Qtrue ) {
    const uint32_t group_id = NUM2UINT( rb_funcall( action, rb_intern( "group_id" ), 0 ) );
    append_action_group( actions, group_id );
  }
  else if ( rb_funcall( action, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::CopyTtlIn" ) ) == Qtrue ) {
    append_action_copy_ttl_out( actions );
  }
  else if ( rb_funcall( action, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::CopyTtlOut" ) ) == Qtrue ) {
    append_action_copy_ttl_out( actions );
  }
  else if ( rb_funcall( action, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::SetMplsTtl" ) ) == Qtrue ) {
    const uint8_t mpls_ttl = ( const uint8_t ) NUM2UINT( rb_funcall( action, rb_intern( "mpls_ttl" ), 0 ) );
    append_action_set_mpls_ttl( actions, mpls_ttl );
  }
  else if ( rb_funcall( action, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::DecMplsTtl" ) ) == Qtrue ) {
    append_action_dec_mpls_ttl( actions );
  }
  else if ( rb_funcall( action, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::PushVlan" ) ) == Qtrue ) {
    const uint16_t ether_type = ( const uint16_t ) NUM2UINT( rb_funcall( action, rb_intern( "ethertype" ), 0 ) );
    append_action_push_vlan( actions, ethertype );
  }
  else if ( rb_funcall( action, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::PopVlan" ) ) == Qtrue ) {
    append_action_pop_vlan( actions );
  }
  else if ( rb_funcall( action, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::PushMpls" ) ) == Qtrue ) {
    const uint16_t ether_type = ( const uint16_t ) NUM2UINT( rb_funcall( action, rb_intern( "ethertype" ), 0 ) );
    append_action_push_mpls( actions, ethertype );
  }
  else if ( rb_funcall( action, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::PopMpls" ) ) == Qtrue ) {
    const uint16_t ether_type = ( const uint16_t ) NUM2UINT( rb_funcall( action, rb_intern( "ethertype" ), 0 ) );
    append_action_pop_mpls( actions, ethertype );
  }
  else if ( rb_funcall( action, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::SetQueue" ) ) == Qtrue ) {
    const uint32_t queue_id = NUM2UINT( rb_funcall( action, rb_intern( "queue_id" ), 0 ) );
    append_action_set_queue( actions, queue_id );
  }
  else if ( rb_funcall( action, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::SetIpTtl" ) ) == Qtrue ) {
    const uint8_t ip_ttl = ( const uint8_t ) NUM2UINT( rb_funcall( action, rb_intern( "ip_ttl" ), 0 ) );
    append_action_set_nw_ttl( actions, nw_ttl );
  }
  else if ( rb_funcall( action, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::PushPbb" ) ) == Qtrue ) {
    const uint16_t ether_type = ( const uint16_t ) NUM2UINT( rb_funcall( action, rb_intern( "ethertype" ), 0 ) );
    append_action_push_pbb( actions, ethertype );
  }
  else if ( rb_funcall( action, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::PopPbb" ) ) == Qtrue ) {
    append_action_pop_pbb( actions );
  }
  else if ( rb_funcall( action, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::ExperimenterAction" ) ) == Qtrue ) {
    VALUE experimenter = rb_funcall( action, rb_intern( "experimenter" ), 0 );
    VALUE rbody = rb_funcall( action, rb_intern( "body" ), 0 );
    if ( rbody != Qnil ) {
      Check_Type( rbody, T_ARRAY );
      uint16_t length = ( uint16_t ) RARRAY_LEN( rbody );
      buffer *body = alloc_buffer_with_length( length );
      void *p = append_back_buffer( body, length );
      for ( int i = 0; i < length; i++ ) {
        ( ( uint8_t * ) p )[ i ] = ( uint8_t ) FIX2INT( RARRAY_PTR( rbody )[ i ] );
      }
      append_action_vendor( actions, ( uint32_t ) NUM2UINT( vendor_id ), body );
      free_buffer( body );
    }
    else {
      append_action_vendor( actions, ( uint32_t ) NUM2UINT( vendor_id ), NULL );
    }
  }
  else {
    rb_raise( rb_eTypeError, "actions argument must be an Array of Action objects" );
  }

  }
}


static void
form_actions( VALUE raction, openflow_actions *actions ) {
  if ( raction != Qnil ) {
    switch ( TYPE( raction ) ) {
      case T_ARRAY:
        {
          VALUE *each = RARRAY_PTR( raction );
          int i;
          for ( i = 0; i < RARRAY_LEN( raction ); i++ ) {
            append_action( actions, each[ i ] );
          }
        }
        break;
      case T_OBJECT:
        append_action( actions, raction );
        break;
      default:
        rb_raise( rb_eTypeError, "actions argument must be an Array or an Action object" );
    }
  }
}


static VALUE
get_strict( int argc, VALUE *argv ) {
  VALUE datapath_id = Qnil;
  VALUE options = Qnil;
  VALUE strict = Qnil;

  rb_scan_args( argc, argv, "11", &datapath_id, &options );
  if ( options != Qnil ) {
    strict = rb_hash_aref( options, ID2SYM( rb_intern( "strict" ) ) );
  }
  return strict;
}


static VALUE
controller_send_flow_mod( uint16_t command, int argc, VALUE *argv, VALUE self ) {
  VALUE datapath_id = Qnil;
  VALUE options = Qnil;
  rb_scan_args( argc, argv, "11", &datapath_id, &options );

  // Defaults
  struct ofp_match default_match;
  memset( &default_match, 0, sizeof( struct ofp_match ) );
  default_match.wildcards = OFPFW_ALL;
  struct ofp_match *match = &default_match;
  uint64_t cookie = get_cookie();
  uint16_t idle_timeout = 0;
  uint16_t hard_timeout = 0;
  uint16_t priority = OFP_HIGH_PRIORITY;
  uint32_t buffer_id = OFP_NO_BUFFER;
  uint16_t out_port = OFPP_NONE;
  uint16_t flags = OFPFF_SEND_FLOW_REM;
  openflow_actions *actions = create_actions();

  // Options
  if ( options != Qnil ) {
    VALUE opt_match = rb_hash_aref( options, ID2SYM( rb_intern( "match" ) ) );
    if ( opt_match != Qnil ) {
      Data_Get_Struct( opt_match, struct ofp_match, match );
    }

    VALUE opt_cookie = rb_hash_aref( options, ID2SYM( rb_intern( "cookie" ) ) );
    if ( opt_cookie != Qnil ) {
      cookie = NUM2ULL( opt_cookie );
    }

    VALUE opt_idle_timeout = rb_hash_aref( options, ID2SYM( rb_intern( "idle_timeout" ) ) );
    if ( opt_idle_timeout != Qnil ) {
      idle_timeout = ( uint16_t )NUM2UINT( opt_idle_timeout );
    }

    VALUE opt_hard_timeout = rb_hash_aref( options, ID2SYM( rb_intern( "hard_timeout" ) ) );
    if ( opt_hard_timeout != Qnil ) {
      hard_timeout = ( uint16_t )NUM2UINT( opt_hard_timeout );
    }

    VALUE opt_priority = rb_hash_aref( options, ID2SYM( rb_intern( "priority" ) ) );
    if ( opt_priority != Qnil ) {
      priority = ( uint16_t )NUM2UINT( opt_priority );
    }

    VALUE opt_buffer_id = rb_hash_aref( options, ID2SYM( rb_intern( "buffer_id" ) ) );
    if ( opt_buffer_id != Qnil ) {
      buffer_id = ( uint32_t ) NUM2ULONG( opt_buffer_id );
    }

    VALUE opt_out_port = rb_hash_aref( options, ID2SYM( rb_intern( "out_port" ) ) );
    if ( opt_out_port != Qnil ) {
      out_port = ( uint16_t )NUM2UINT( opt_out_port );
    }

    VALUE opt_send_flow_rem = rb_hash_aref( options, ID2SYM( rb_intern( "send_flow_rem" ) ) );
    if ( opt_send_flow_rem == Qfalse ) {
      flags &= ( uint16_t ) ~OFPFF_SEND_FLOW_REM;
    }

    VALUE opt_check_overlap = rb_hash_aref( options, ID2SYM( rb_intern( "check_overlap" ) ) );
    if ( opt_check_overlap != Qnil ) {
      flags |= OFPFF_CHECK_OVERLAP;
    }

    VALUE opt_emerg = rb_hash_aref( options, ID2SYM( rb_intern( "emerg" ) ) );
    if ( opt_emerg != Qnil ) {
      flags |= OFPFF_EMERG;
    }

    VALUE opt_actions = rb_hash_aref( options, ID2SYM( rb_intern( "actions" ) ) );
    if ( opt_actions != Qnil ) {
      form_actions( opt_actions, actions );
    }
  }

  buffer *flow_mod = create_flow_mod(
    get_transaction_id(),
    *match,
    cookie,
    command,
    idle_timeout,
    hard_timeout,
    priority,
    buffer_id,
    out_port,
    flags,
    actions
  );
  send_openflow_message( NUM2ULL( datapath_id ), flow_mod );

  free_buffer( flow_mod );
  delete_actions( actions );

  return self;
}


/*
 * @overload send_flow_mod_add(datapath_id, options={})
 *   Sends a flow_mod message to add a flow into the datapath.
 *
 *   @example
 *     def packet_in datapath_id, message
 *       send_flow_mod_add datapath_id, :match => Match.from(message), :actions => ActionOutput.new(OFPP_FLOOD)
 *     end
 *
 *
 *   @param [Number] datapath_id
 *     the datapath to which a message is sent.
 *
 *   @param [Hash] options
 *     the options to create a message with.
 *
 *
 *   @option options [Match, nil] :match (nil)
 *     A {Match} object describing the fields of the flow.
 *
 *   @option options [Number] :idle_timeout (0)
 *     The idle time in seconds before discarding.
 *     Zero means flow never expires.
 *
 *   @option options [Number] :cookie
 *     An opaque issued identifier.
 *
 *   @option options [Number] :hard_timeout (0)
 *     The maximum time before discarding in seconds.
 *     Zero means flow never expires.
 *
 *   @option options [Number] :priority (0xffff)
 *     The priority level of the flow entry.
 *
 *   @option options [Number] :buffer_id (0xffffffff)
 *     The buffer ID assigned by the datapath of a buffered packet to
 *     apply the flow to. If 0xffffffff, no buffered packet is to be
 *     applied to flow actions.
 *
 *   @option options [Number] :out_port (0xffff)
 *     If the option contains a value other than OFPP_NONE(0xffff), 
 *     it introduces a constraint when deleting flow entries.
 *
 *   @option options [Boolean] :send_flow_rem (true)
 *     If true, send a flow_removed message when the flow expires or
 *     is deleted.
 *
 *   @option options [Boolean] :check_overlap (false)
 *     If true, check for overlapping entries first, i.e. if there are
 *     conflicting entries with the same priority, the flow is not
 *     added and the modification fails.
 *
 *   @option options [Boolean] :emerg (false)
 *     if true, the switch must consider this flow entry as an
 *     emergency entry, and only use it for forwarding when
 *     disconnected from the controller.
 *
 *   @option options [ActionOutput, Array<ActionOutput>, nil] :actions (nil)
 *     The sequence of actions specifying the actions to perform on
 *     the flow's packets.
 */
static VALUE
controller_send_flow_mod_add( int argc, VALUE *argv, VALUE self ) {
  return controller_send_flow_mod( OFPFC_ADD, argc, argv, self );
}


/*
 * @overload send_flow_mod_modify(datapath, options={})
 *   Sends a flow_mod message to either modify or modify strict a flow from datapath.
 *   Both flow_mod modify and flow_mod modify strict commands would modify
 *   matched flow actions. The strict option adds the flow priority to the
 *   matched criteria. Accepts the same options as #send_flow_mod_add with the
 *   following additional option.
 *
 *   @option options [Symbol] :strict
 *     If set to true modify_strict command is invoked otherwise the modify
 *     command is invoked.
 */
static VALUE
controller_send_flow_mod_modify( int argc, VALUE *argv, VALUE self ) {
  uint16_t command = OFPFC_MODIFY;

  if ( get_strict( argc, argv ) == Qtrue ) {
    command = OFPFC_MODIFY_STRICT;
  }
  return controller_send_flow_mod( command, argc, argv, self );
}


/*
 * @overload send_flow_mod_delete(datapath_id, options={})
 *   Sends a flow_mod_delete message to delete all matching flows.
 *   Both flow_mod delete and flow_mod delete strict commands would delete matched flows.
 *   The strict option adds the flow priority to the matched criteria.
 *   Accepts the same options as #send_flow_mod_add with the following additional
 *   option.
 *
 *   @option options [Symbol] :strict
 *     If set to true delete_strict command is invoked otherwise the delete
 *     command is invoked.
 */
static VALUE
controller_send_flow_mod_delete( int argc, VALUE *argv, VALUE self ) {
  uint16_t command = OFPFC_DELETE;

  if ( get_strict( argc, argv ) == Qtrue ) {
    command = OFPFC_DELETE_STRICT;
  }
  return controller_send_flow_mod( command, argc, argv, self );
}


/*
 * @overload send_packet_out(datapath_id, options={})
 *   Sends a packet_out message to have a packet processed by the datapath.
 *
 *   @example
 *     send_packet_out(
 *       datapath_id,
 *       :packet_in => message,
 *       :actions => Trema::ActionOutput.new(port_no)
 *     )
 *
 *
 *   @param [Number] datapath_id
 *     the datapath to which a message is sent.
 *
 *   @param [Hash] options
 *     the options to create a message with.
 *
 *
 *   @option options [PacketIn] :packet_in (nil)
 *     The {PacketIn} object received by packet_in handler. If this
 *     option is not nil, :buffer_id, :data, and :in_port option is
 *     set automatically according to the value of :packet_in.
 *
 *   @option options [Number] :in_port (OFPP_NONE)
 *     The port from which the frame is to be sent. OFPP_NONE if
 *     none. OFPP_TABLE to perform the actions defined in the flow
 *     table.
 *
 *   @option options [Number] :buffer_id (0xffffffff)
 *     The buffer ID assigned by the datapath. If 0xffffffff, the
 *     frame is not buffered, and the entire frame must be passed in
 *     :data.
 *
 *   @option options [String, nil] :data (nil)
 *     The entire Ethernet frame. Should be of length 0 if buffer_id
 *     is 0xffffffff, and should be of length >0 otherwise.
 *
 *   @option options [ActionOutput, Array<ActionOutput>, nil] :actions (nil)
 *     The sequence of actions specifying the actions to perform on
 *     the frame.
 */
static VALUE
controller_send_packet_out( int argc, VALUE *argv, VALUE self ) {
  VALUE datapath_id = Qnil;
  VALUE options = Qnil;
  rb_scan_args( argc, argv, "11", &datapath_id, &options );

  // Defaults.
  uint32_t buffer_id = OFP_NO_BUFFER;
  uint16_t in_port = OFPP_NONE;
  openflow_actions *actions = create_actions();
  const buffer *data = NULL;
  buffer *allocated_data = NULL;

  if ( options != Qnil ) {
    VALUE opt_message = rb_hash_aref( options, ID2SYM( rb_intern( "packet_in" ) ) );
    if ( opt_message != Qnil ) {
      packet_in *message;
      Data_Get_Struct( opt_message, packet_in, message );

      if ( NUM2ULL( datapath_id ) == message->datapath_id ) {
        buffer_id = message->buffer_id;
        in_port = message->in_port;
      }
      data = ( buffer_id == OFP_NO_BUFFER ? message->data : NULL );
    }

    VALUE opt_buffer_id = rb_hash_aref( options, ID2SYM( rb_intern( "buffer_id" ) ) );
    if ( opt_buffer_id != Qnil ) {
      buffer_id = ( uint32_t ) NUM2ULONG( opt_buffer_id );
    }

    VALUE opt_in_port = rb_hash_aref( options, ID2SYM( rb_intern( "in_port" ) ) );
    if ( opt_in_port != Qnil ) {
      in_port = ( uint16_t ) NUM2UINT( opt_in_port );
    }

    VALUE opt_action = rb_hash_aref( options, ID2SYM( rb_intern( "actions" ) ) );
    if ( opt_action != Qnil ) {
      form_actions( opt_action, actions );
    }

    VALUE opt_data = rb_hash_aref( options, ID2SYM( rb_intern( "data" ) ) );
    if ( opt_data != Qnil ) {
      Check_Type( opt_data, T_STRING );
      uint16_t length = ( u_int16_t ) RSTRING_LEN( opt_data );
      allocated_data = alloc_buffer_with_length( length );
      memcpy( append_back_buffer( allocated_data, length ), RSTRING_PTR( opt_data ), length );
      data = allocated_data;
    }
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
  return self;
}


/*
 * Starts this controller. Usually you do not need to invoke
 * explicitly, because this is called implicitly by "trema run"
 * command.
 */
static VALUE
controller_run( VALUE self ) {
  setenv( "TREMA_HOME", STR2CSTR( rb_funcall( mTrema, rb_intern( "home" ), 0 ) ), 1 );

  VALUE name = rb_funcall( self, rb_intern( "name" ), 0 );
  rb_gv_set( "$PROGRAM_NAME", name );

  int argc = 3;
  char **argv = xmalloc( sizeof ( char * ) * ( uint32_t ) ( argc + 1 ) );
  argv[ 0 ] = STR2CSTR( name );
  argv[ 1 ] = ( char * ) ( uintptr_t ) "--name";
  argv[ 2 ] = STR2CSTR( name );
  argv[ 3 ] = NULL;
  init_trema( &argc, &argv );
  xfree( argv );

  set_switch_ready_handler( handle_switch_ready, ( void * ) self );
  set_features_reply_handler( handle_features_reply, ( void * ) self );
  set_packet_in_handler( handle_packet_in, ( void * ) self );
  set_flow_removed_handler( handle_flow_removed, ( void * ) self );
  set_switch_disconnected_handler( handle_switch_disconnected, ( void * ) self );
  set_port_status_handler( handle_port_status, ( void * ) self );
  set_stats_reply_handler( handle_stats_reply, ( void * ) self );
  set_error_handler( handle_openflow_error, ( void * ) self );
  set_get_config_reply_handler( handle_get_config_reply, ( void * ) self );
  set_barrier_reply_handler( handle_barrier_reply, ( void * ) self );
  set_vendor_handler( handle_vendor, ( void * ) self );
  set_queue_get_config_reply_handler( handle_queue_get_config_reply, ( void * ) self );
  set_list_switches_reply_handler( handle_list_switches_reply );

  struct itimerspec interval;
  interval.it_interval.tv_sec = 1;
  interval.it_interval.tv_nsec = 0;
  interval.it_value.tv_sec = 0;
  interval.it_value.tv_nsec = 0;
  add_timer_event_callback( &interval, handle_timer_event, ( void * ) self );

  if ( rb_respond_to( self, rb_intern( "start" ) ) == Qtrue ) {
    rb_funcall( self, rb_intern( "start" ), 0 );
  }

  rb_funcall( self, rb_intern( "start_trema" ), 0 );

  return self;
}


/*
 * @overload shutdown!
 *   In the context of trema framework stops this controller and its applications.
 */
static VALUE
controller_shutdown( VALUE self ) {
  stop_trema();
  return self;
}


static void
thread_pass( void *user_data ) {
  UNUSED( user_data );
  CHECK_INTS;
  rb_funcall( rb_cThread, rb_intern( "pass" ), 0 );
}


/*
 * In the context of trema framework invokes the scheduler to start its applications.
 */
static VALUE
controller_start_trema( VALUE self ) {
  struct itimerspec interval;
  interval.it_interval.tv_sec = 0;
  interval.it_interval.tv_nsec = 1000000;
  interval.it_value.tv_sec = 0;
  interval.it_value.tv_nsec = 0;
  add_timer_event_callback( &interval, thread_pass, NULL );

  start_trema();

  return self;
}


/********************************************************************************
 * Init Controller module.
 ********************************************************************************/

void
Init_controller() {
  rb_require( "trema/send-out-port" );
  rb_require( "trema/group-action" );
  rb_require( "trema/copy-ttl-in" );
  rb_require( "trema/copy-ttl-out" );
  rb_require( "trema/set-mpls-ttl" );
  rb_require( "trema/app" );
  VALUE cApp = rb_eval_string( "Trema::App" );
  cController = rb_define_class_under( mTrema, "Controller", cApp );

  rb_define_const( cController, "OFPP_MAX", INT2NUM( OFPP_MAX ) );
  rb_define_const( cController, "OFPP_IN_PORT", INT2NUM( OFPP_IN_PORT ) );
  rb_define_const( cController, "OFPP_TABLE", INT2NUM( OFPP_TABLE ) );
  rb_define_const( cController, "OFPP_NORMAL", INT2NUM( OFPP_NORMAL ) );
  rb_define_const( cController, "OFPP_FLOOD", INT2NUM( OFPP_FLOOD ) );
  rb_define_const( cController, "OFPP_ALL", INT2NUM( OFPP_ALL ) );
  rb_define_const( cController, "OFPP_CONTROLLER", INT2NUM( OFPP_CONTROLLER ) );
  rb_define_const( cController, "OFPP_LOCAL", INT2NUM( OFPP_LOCAL ) );
  rb_define_const( cController, "OFPP_NONE", INT2NUM( OFPP_NONE ) );

  rb_define_method( cController, "send_message", controller_send_message, 2 );
  rb_define_method( cController, "send_list_switches_request", controller_send_list_switches_request, 0 );
  rb_define_method( cController, "send_flow_mod_add", controller_send_flow_mod_add, -1 );
  rb_define_method( cController, "send_flow_mod_modify", controller_send_flow_mod_modify, -1 );
  rb_define_method( cController, "send_flow_mod_delete", controller_send_flow_mod_delete, -1 );
  rb_define_method( cController, "send_packet_out", controller_send_packet_out, -1 );

  rb_define_method( cController, "run!", controller_run, 0 );
  rb_define_method( cController, "shutdown!", controller_shutdown, 0 );
  rb_define_private_method( cController, "start_trema", controller_start_trema, 0 );

  rb_require( "trema/controller" );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
