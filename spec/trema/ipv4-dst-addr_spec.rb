#
# Copyright (C) 2008-2013 NEC Corporation
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License, version 2, as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#


require File.join( File.dirname( __FILE__ ), "..", "spec_helper" )
require "timeout"
require "trema"
require "packetfu"


def send_tcp_packet **opts
  config = {}
  config[ :iface ] = opts[ :iface ]
  pkt = PacketFu::TCPPacket.new( :config => config, :flavor => "Linux" )
  set_any_defaults pkt, opts
  pkt.recalc
  pkt.tcp_src = opts[ :tcp_src ] if opts.has_key? :tcp_src
  pkt.tcp_dst = opts[ :tcp_dst ] if opts.has_key? :tcp_dst
  inject_pkt pkt, opts[ :iface ]
end


def send_udp_packet **opts
  config = {}
  config[ :iface ] = opts[ :iface ]
  pkt = PacketFu::UDPPacket.new( :config => config, :flavor => "Linux" )
  set_any_defaults pkt, opts
  pkt.udp_src = opts[ :udp_src ] if opts.has_key? :udp_src
  pkt.udp_dst = opts[ :udp_dst ] if opts.has_key? :udp_dst
  pkt.recalc
  inject_pkt pkt, opts[ :iface ]
end


def set_any_defaults pkt, opts
  pkt.ip_saddr = opts[ :ip_saddr ] if opts.has_key? :ip_saddr
  pkt.ip_daddr = opts[ :ip_daddr ] if opts.has_key? :ip_daddr
  pkt.payload = "this is a test"
end


def inject_pkt pkt, iface
  inj = PacketFu::Inject.new( :iface => iface )
  inj.array_to_wire( :array => [ pkt.to_s ] )
end


describe Trema::Ipv4DstAddr, "new( VALID OPTIONS )" do
  subject { Ipv4DstAddr.new(  ip_addr: addr ) }
  let( :addr ) { IPAddr.new( "192.168.0.1" ) }
  its ( :ip_addr ) { should == "192.168.0.1" }
end


describe Trema::Ipv4DstAddr, ".new( MANDADORY OPTION MISSING ) - ip_addr" do
  subject { Ipv4DstAddr.new }
  it "should raise ArgumentError" do
    expect { subject }.to raise_error( ArgumentError, /Required option ip_addr missing/ )
  end
end


describe Trema::Ipv4DstAddr, ".new( VALID OPTIONS )" do
  context "when setting a flow with a match ipv4_dst field set" do
    it "should match its ipv4_dst field when an IPv4 packet received" do
      network_blk = Proc.new {
        trema_switch( "lsw" ) { datapath_id 0xabc }
        vhost( "host1" ) {
          ip "192.168.0.1"
          netmask "255.255.255.0"
          mac "00:00:00:00:00:01"
        }
        vhost( "host2" ) {
          ip "192.168.0.2"
          netmask "255.255.255.0"
          mac "00:00:00:00:00:02"
        }
        link "host1", "lsw:1"
        link "host2", "lsw:2"
      }
      mc = MockController.new( network_blk )
      mc.should_receive( :switch_ready ) do | datapath_id |
        action = SendOutPort.new( port_number: OFPP_CONTROLLER )
        apply_ins = ApplyAction.new( actions: [ action ] )
        match_fields = Match.new( in_port: 1, eth_type: 2048, ipv4_dst: IPAddr.new( "192.168.0.2" ) )
        mc.send_flow_mod_add( datapath_id,
                              cookie: 1111,
                              match: match_fields,
                              instructions: [ apply_ins ] )
      end
      mc.should_receive( :packet_in ) do | datapath_id, message |
puts message.inspect
        action = Trema::Ipv4DstAddr.new( ip_addr: IPAddr.new( "192.168.0.2" ) )
        expect( action.ip_addr.to_s ).to  eq( message.packet_info.ipv4_dst.to_s )
      end
      mc.start_receiving
      ip_saddr = vhost( "host1" ).ip
      iface = vhost( "host1" ).interface
      ip_daddr = vhost( "host2" ).ip
      send_tcp_packet iface: iface, ip_saddr: ip_saddr, ip_daddr: ip_daddr, tcp_src: 7777, tcp_dst: 8888
      #send_udp_packet iface: iface, ip_saddr: ip_saddr, ip_daddr: ip_daddr, udp_src: 7777, udp_dst: 8888
      mc.time_sleep( 2 ) {
        mc.stop_receiving
      }
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
