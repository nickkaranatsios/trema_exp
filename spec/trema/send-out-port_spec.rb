#
# Author: Nick Karanatsios <nickkaranatsios@gmail.com>
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
require "trema"


shared_examples_for "any OpenFlow message with max_len option" do
  it_should_behave_like "any OpenFlow message", :option => :max_len, :name => "max_len", :size => 16
end


describe SendOutPort, ".new( VALID OPTIONS )" do
  subject { SendOutPort.new :port => port, :max_len => max_len }
  let( :port ) { 1 }
  let( :max_len ) { 256 }
  its( :port ) { should == 1 }
  its( :max_len ) { should == 256 }
  it_should_behave_like "any OpenFlow message with port option"
  it_should_behave_like "any OpenFlow message with max_len option"
end


describe SendOutPort, ".new( MANDATORY OPTION MISSING )" do
  it "should raise ArgumentError" do
    expect { subject }.to raise_error( ArgumentError )
  end
end


describe SendOutPort, ".new( INVALID OPTION ) - port" do
  subject { SendOutPort.new  :port => port }
  it_should_behave_like "any OpenFlow message with port option"
end


describe SendOutPort, ".new( VALID OPTIONS )" do
  context "when an action output is set to #flow_mod(add) " do
    it "should respond to #pack_send_out_port" do
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
        action.should_receive( :pack_send_out_port )
        apply_ins = ApplyAction.new( actions: [ action ] )
        match_fields = Match.new( in_port: 1 )
        mc.send_flow_mod_add( datapath_id,
                              cookie: 1111,
                              match: match_fields,
                              instructions: [ apply_ins ] )
      end
      mc.start_receiving
      mc.stop_receiving
    end
      

    it "should have its action set to output:1" do
    end
  end


  context "when multiple output actions assigned to #flow_mod(add)" do
    it "should have its actions set to output:1\/output:2" do
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
