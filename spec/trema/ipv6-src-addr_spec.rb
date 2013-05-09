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


describe Trema::Ipv6SrcAddr, "new( VALID OPTIONS )" do
  subject { Ipv6SrcAddr.new(  ip_addr: addr ) }
  let( :addr ) { IPAddr.new( "fe80::219:dbff:fe1d:d592" ) }
  its ( :ip_addr ) { should == "fe80::219:dbff:fe1d:d592" }
end


describe Trema::Ipv6SrcAddr, ".new( MANDADORY OPTION MISSING ) - ip_addr" do
  subject { Ipv6SrcAddr.new }
  it "should raise ArgumentError" do
    expect { subject }.to raise_error( ArgumentError, /Required option ip_addr missing/ )
  end
end


describe Trema::Ipv6SrcAddr, ".new( VALID OPTIONS )" do
  context "when setting a flow with a match ipv6_src field set" do
    it "should match its ipv6_src field when an IPv6 packet received" do
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
        match_fields = Match.new( in_port: 1, eth_type: 2048, ipv6_src: IPAddr.new( "fe80::219:dbff:fe1d:d592" ) )
        mc.send_flow_mod_add( datapath_id,
                              cookie: 1111,
                              match: match_fields,
                              instructions: [ apply_ins ] )
      end
      mc.should_receive( :packet_in ) do | datapath_id, message |
        action = Trema::Ipv6SrcAddr.new( ip_addr: IPAddr.new( "fe80::219:dbff:fe1d:d592" ) )
        expect( action.ip_addr.to_s ).to  eq( message.packet_info.ipv4_dst.to_s )
      end
      mc.start_receiving
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
