$LOAD_PATH.unshift File.expand_path( File.join( File.dirname( __FILE__ ), "." ) )

require "trema"
module Trema
  class Test < Controller
    def match_set
      ms = [ 
        MatchSet::MatchInPort.new( 2 ),
        MatchSet::MatchInPhyPort.new( 2 ), 
        MatchSet::MatchMetadata.new( 2**34 ),
        MatchSet::MatchEthDst.new( "11:22:33:44:55:66" ),
        MatchSet::MatchEthSrc.new( "11:22:33:44:55:66" ),
        MatchSet::MatchEtherType.new( 0x7777 ),
        MatchSet::MatchVlanVid.new( 1024 ),
        MatchSet::MatchVlanPriority.new( 7 ),
        MatchSet::MatchIpDscp.new( 63 ),
        MatchSet::MatchIpEcn.new( 43 ),
        MatchSet::MatchIpProto.new( 17 ),
        MatchSet::MatchIpv4SrcAddr.new( "10.10.10.1" ),
        MatchSet::MatchIpv4DstAddr.new( "127.0.0.1" ),
        MatchSet::MatchTcpSrcPort.new( 3331 ),
        MatchSet::MatchTcpDstPort.new( 3332 ),
        MatchSet::MatchUdpSrcPort.new( 3333 ),
        MatchSet::MatchUdpDstPort.new( 3334 ),
        MatchSet::MatchSctpSrcPort.new( 3336 ),
        MatchSet::MatchSctpDstPort.new( 3337 ),
        MatchSet::MatchIcmpv4Type.new( 0 ), # echo reply
        MatchSet::MatchIcmpv4Code.new( 2 ),
        MatchSet::MatchArpOp.new( 1 ), # REQUEST
        MatchSet::MatchArpSpa.new( "1.1.1.1" ),
        MatchSet::MatchArpTpa.new( "1.1.1.2" ),
        MatchSet::MatchArpSha.new( "11:22:33:44:55:67" ),
        MatchSet::MatchArpTha.new( "11:22:33:44:55:68" ),
        MatchSet::MatchIpv6SrcAddr.new( "ffe1::1" ),
        MatchSet::MatchIpv6DstAddr.new( "ffe2::2" ),
        MatchSet::MatchIpv6FlowLabel.new( 10 ),
        MatchSet::MatchIcmpv6Type.new( 1 ), # destination unreachable
        MatchSet::MatchIcmpv6Code.new( 3 ), # address unreachable if as above type.
        MatchSet::MatchIpv6NdTarget.new( "ffe3::3" ),
        MatchSet::MatchIpv6NdSll.new( "11:22:33:44:55:69" ),
        MatchSet::MatchIpv6NdTll.new( "11:22:33:44:55:70" ),
        MatchSet::MatchMplsLabel.new( 0 ), # IPv4 Explicit NULL label
        MatchSet::MatchMplsTc.new( 1 ), # TC refers to traffic class
        MatchSet::MatchMplsBos.new( 1 ), # Bottom of stack a bit on/off.
        MatchSet::MatchPbbIsid.new( 10 ), # Provider Backbone bridge i-sid ( service identifier ) 3 bytes
        MatchSet::MatchTunnelId.new( rand( 2**64 ) ), # Tunnel id applies to GRE packets with RFC2890 key extension
        MatchSet::MatchIpv6Exthdr.new( rand( 2**16 ) ),
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

