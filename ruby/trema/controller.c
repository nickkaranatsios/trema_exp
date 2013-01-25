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


#include "buffer.h"
#include "controller.h"
#include "openflow.h"
#include "ruby.h"
#include "trema.h"


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


#ifdef TEST
static void
append_action_set( openflow_actions *actions, VALUE field ) {
  VALUE r_actions = Data_Wrap_Struct( field, NULL, delete_actions, actions );
  if ( rb_respond_to( field, rb_intern( "append_match" ) ) == Qtrue  ) {
    rb_funcall( field, rb_intern( "append_match" ), 1, r_actions );
  }
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
    append_action_push_vlan( actions, ether_type );
  }
  else if ( rb_funcall( action, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::PopVlan" ) ) == Qtrue ) {
    append_action_pop_vlan( actions );
  }
  else if ( rb_funcall( action, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::PushMpls" ) ) == Qtrue ) {
    const uint16_t ether_type = ( const uint16_t ) NUM2UINT( rb_funcall( action, rb_intern( "ethertype" ), 0 ) );
    append_action_push_mpls( actions, ether_type );
  }
  else if ( rb_funcall( action, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::PopMpls" ) ) == Qtrue ) {
    const uint16_t ether_type = ( const uint16_t ) NUM2UINT( rb_funcall( action, rb_intern( "ethertype" ), 0 ) );
    append_action_pop_mpls( actions, ether_type );
  }
  else if ( rb_funcall( action, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::SetQueue" ) ) == Qtrue ) {
    const uint32_t queue_id = NUM2UINT( rb_funcall( action, rb_intern( "queue_id" ), 0 ) );
    append_action_set_queue( actions, queue_id );
  }
  else if ( rb_funcall( action, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::SetIpTtl" ) ) == Qtrue ) {
    const uint8_t ip_ttl = ( const uint8_t ) NUM2UINT( rb_funcall( action, rb_intern( "ip_ttl" ), 0 ) );
    append_action_set_nw_ttl( actions, ip_ttl );
  }
  else if ( rb_funcall( action, rb_intern( "is_a?" ), 1, rb_path2class( "Trema::PushPbb" ) ) == Qtrue ) {
    const uint16_t ether_type = ( const uint16_t ) NUM2UINT( rb_funcall( action, rb_intern( "ethertype" ), 0 ) );
    append_action_push_pbb( actions, ether_type );
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
      append_action_experimenter( actions, ( uint32_t ) NUM2UINT( experimenter ), body );
      free_buffer( body );
    }
    else {
      append_action_experimenter( actions, ( uint32_t ) NUM2UINT( experimenter ), NULL );
    }
  }
  else {
    rb_raise( rb_eTypeError, "actions argument must be an Array of Action objects" );
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
#endif



/*
 * Starts this controller. Usually you do not need to invoke
 * explicitly, because this is called implicitly by "trema run"
 * command.
 */
static VALUE
controller_run( VALUE self ) {
  setenv( "TREMA_HOME", RSTRING_PTR( rb_funcall( mTrema, rb_intern( "home" ), 0 ) ), 1 );

  VALUE name = rb_funcall( self, rb_intern( "name" ), 0 );
  rb_gv_set( "$PROGRAM_NAME", name );

  int argc = 3;
  char **argv = xmalloc( sizeof ( char * ) * ( uint32_t ) ( argc + 1 ) );
  argv[ 0 ] = RSTRING_PTR( name );
  argv[ 1 ] = ( char * ) ( uintptr_t ) "--name";
  argv[ 2 ] = RSTRING_PTR( name );
  argv[ 3 ] = NULL;
  init_trema( &argc, &argv );
  xfree( argv );

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
  rb_thread_check_ints();
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


static VALUE
controller_test_match_set( VALUE self, VALUE match_set ) {
  openflow_actions *actions = create_actions();
  VALUE cActions;


  if ( match_set != Qnil ) {
    switch ( TYPE( match_set ) ) {
      case T_ARRAY:
        {
          VALUE *each = RARRAY_PTR( match_set );
          int i;
          
          if ( RARRAY_LEN( match_set ) ) {
            cActions = Data_Wrap_Struct( rb_obj_class( each[ 0 ] ), NULL, delete_actions, actions );
          }
          for ( i = 0; i < RARRAY_LEN( match_set ); i++ ) {
            if ( rb_respond_to( each[ i ], rb_intern( "append_match" ) ) ) {
              rb_funcall( each[ i ], rb_intern( "append_match" ), 1, cActions );
            }
          }
          Data_Get_Struct( cActions, openflow_actions, actions );
printf("no.of actions added %d\n", actions->n_actions );
        }
        break;
      case T_OBJECT:
        if ( rb_respond_to( rb_obj_class( match_set ), rb_intern( ":append_match" ) ) ) {
          cActions = Data_Wrap_Struct( match_set, NULL, delete_actions, actions );
          rb_funcall( match_set, rb_intern( "append_match" ), 1, cActions );
        }
        break;
      default:
        rb_raise( rb_eTypeError, "match field argument must be an Array or an MatchXXX object" );
    }
  }
  return self;
}


static VALUE
controller_test_action_list( VALUE self, VALUE action_list ) {
  openflow_actions *actions = create_actions();
  VALUE cActions;


  if ( action_list != Qnil ) {
    switch ( TYPE( action_list ) ) {
      case T_ARRAY:
        {
          VALUE *each = RARRAY_PTR( action_list );
          int i;
          
          if ( RARRAY_LEN( action_list ) ) {
            cActions = Data_Wrap_Struct( rb_obj_class( each[ 0 ] ), NULL, delete_actions, actions );
          }
          for ( i = 0; i < RARRAY_LEN( action_list ); i++ ) {
            if ( rb_respond_to( each[ i ], rb_intern( "append_action" ) ) ) {
              rb_funcall( each[ i ], rb_intern( "append_action" ), 1, cActions );
            }
          }
          Data_Get_Struct( cActions, openflow_actions, actions );
printf("no.of actions added %d\n", actions->n_actions );
        }
        break;
      case T_OBJECT:
        if ( rb_respond_to( rb_obj_class( action_list ), rb_intern( ":append_action" ) ) ) {
          cActions = Data_Wrap_Struct( action_list, NULL, delete_actions, actions );
          rb_funcall( action_list, rb_intern( "append_action" ), 1, cActions );
        }
        break;
      default:
        rb_raise( rb_eTypeError, "action list argument must be an Array or an Action object" );
    }
  }
  return self;
}


/********************************************************************************
 * Init Controller module.
 ********************************************************************************/

void
Init_controller() {
  rb_require( "trema/group-action" );
  rb_require( "trema/copy-ttl-in" );
  rb_require( "trema/copy-ttl-out" );
  rb_require( "trema/set-mpls-ttl" );
  rb_require( "trema/app" );
  VALUE cApp = rb_eval_string( "Trema::App" );
  cController = rb_define_class_under( mTrema, "Controller", cApp );

  rb_define_method( cController, "send_message", controller_send_message, 2 );

  rb_define_method( cController, "run!", controller_run, 0 );
  rb_define_method( cController, "shutdown!", controller_shutdown, 0 );
  rb_define_private_method( cController, "start_trema", controller_start_trema, 0 );
  rb_define_method( cController, "test_match_set", controller_test_match_set, 1 );
  rb_define_method( cController, "test_action_list", controller_test_action_list, 1 );

  rb_require( "trema/controller" );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
