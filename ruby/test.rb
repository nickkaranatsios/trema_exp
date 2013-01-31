$LOAD_PATH.unshift File.expand_path( File.join( File.dirname( __FILE__ ), "." ) )

require "trema"
module Trema
  class Test < Controller
    def match_set
      ms = [ 
        MatchInPort.new( 2 ),
        MatchInPhyPort.new( 2 ), 
        MatchMetadata.new( 2**34 ),
        MatchEthDst.new( "11:22:33:44:55:66" ),
        MatchEthSrc.new( "11:22:33:44:55:66" ),
        MatchEtherType.new( 0x7777 ),
        MatchVlanVid.new( 1024 ),
        MatchVlanPriority.new( 7 ),
        MatchIpDscp.new( 63 ),
        MatchIpEcn.new( 43 ),
        MatchIpProto.new( 17 ),
        MatchIpv4SrcAddr.new( "10.10.10.1" ),
        MatchIpv4DstAddr.new( "127.0.0.1" ),
        MatchTcpSrcPort.new( 3331 ),
        MatchTcpDstPort.new( 3332 ),
        MatchUdpSrcPort.new( 3333 ),
        MatchUdpDstPort.new( 3334 ),
        MatchSctpSrcPort.new( 3336 ),
        MatchSctpDstPort.new( 3337 ),
        MatchIcmpv4Type.new( 0 ), # echo reply
        MatchIcmpv4Code.new( 2 ),
        MatchArpOp.new( 1 ), # REQUEST
        MatchArpSpa.new( "1.1.1.1" ),
        MatchArpTpa.new( "1.1.1.2" ),
        MatchArpSha.new( "11:22:33:44:55:67" ),
        MatchArpTha.new( "11:22:33:44:55:68" ),
        MatchIpv6SrcAddr.new( "ffe1::1" ),
        MatchIpv6DstAddr.new( "ffe2::2" ),
        MatchIpv6FlowLabel.new( 10 ),
        MatchIcmpv6Type.new( 1 ), # destination unreachable
        MatchIcmpv6Code.new( 3 ), # address unreachable if as above type.
        MatchIpv6NdTarget.new( "ffe3::3" ),
        MatchIpv6NdSll.new( "11:22:33:44:55:69" ),
        MatchIpv6NdTll.new( "11:22:33:44:55:70" ),
        MatchMplsLabel.new( 0 ), # IPv4 Explicit NULL label
        MatchMplsTc.new( 1 ), # TC refers to traffic class
        MatchMplsBos.new( 1 ), # Bottom of stack a bit on/off.
        MatchPbbIsid.new( 10 ), # Provider Backbone bridge i-sid ( service identifier ) 3 bytes
        MatchTunnelId.new( rand( 2**64 ) ), # Tunnel id applies to GRE packets with RFC2890 key extension
        MatchIpv6Exthdr.new( rand( 2**16 ) ),
      ]
      test_match_set ms
    end
    def action_list
      test_action_list build_action_list
    end
    def message_list
      ml = [
        Messages::Hello.new( :transaction_id => 123, :version => [ 0x4 ] ),
        Messages::EchoRequest.new( :transaction_id => 123, :user_data => "abcdefgh".unpack( "C" ) ),
        Messages::FeaturesRequest.new( :transaction_id => 123 ),
        Messages::GetConfigRequest.new( :transaction_id => 123 ),
        Messages::SetConfig.new( :transaction_id => 123, :flags => Messages::OFPC_FRAG_NORMAL, :miss_send_len => Messages::OFPCML_NO_BUFFER ),
      ]
      send_message 0x1, ml
    end
    def instruction_list
      il = [
        InstructionList::GotoTable.new( :table_id => 2 ),
        InstructionList::WriteMetadata.new( :metadata => rand( 2**64 ), :metadata_mask => rand( 2**64 ) ),
        InstructionList::WriteActions.new( :actions => build_action_list ),
        InstructionList::ApplyActions.new( :actions => build_action_list ),
        InstructionList::ClearActions.new,
        InstructionList::Meter.new( rand( 2**32 ) ),
        InstructionList::Experimenter.new( :experimenter => rand( 2 ** 32 ), :user_data => rand( 36**10 ).to_s( 36 ).unpack( "C" ) ),
      ]
      test_instruction_list il
    end


    def build_action_list
      [
        ActionList::SendOutPort.new( :port_number => 2, :max_len => 2**7 ),
        ActionList::GroupAction.new( :group_id => 1 ),
        ActionList::CopyTtlIn.new,
        ActionList::CopyTtlOut.new,
        ActionList::SetMplsTtl.new( 10 ),
        ActionList::DecMplsTtl.new,
        ActionList::PushVlan.new( 0x88a8 ),
        ActionList::PopVlan.new,
        ActionList::PushMpls.new( 0x8847 ),
        ActionList::PopMpls.new( 0x8848 ),
        ActionList::SetQueue.new( rand( 2**32 ) ),
        ActionList::SetIpTtl.new( rand( 2** 8 ) ),
        ActionList::PushPbb.new( rand( 2**16 ) ),
        ActionList::PopPbb.new,
        ActionList::Experimenter.new( :experimenter => rand( 2 ** 32 ), :body => "abcdef".unpack( "C*" ) ),
      ]
    end
  end
end

t = Trema::Test.new
t.action_list
t.match_set
t.message_list
t.instruction_list

