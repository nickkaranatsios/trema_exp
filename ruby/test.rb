$LOAD_PATH.unshift File.expand_path( File.join( File.dirname( __FILE__ ), "." ) )

require "trema"
module Trema
  class Test < Controller
    def flexible_actions
      ms = [ 
        Actions::InPort.new( 2 ),
        Actions::InPhyPort.new( 2 ), 
        Actions::Metadata.new( 2**34 ),
        Actions::EthDst.new( "11:22:33:44:55:66" ),
        Actions::EthSrc.new( "11:22:33:44:55:66" ),
        Actions::EtherType.new( 0x7777 ),
        Actions::VlanVid.new( 1024 ),
        Actions::VlanPriority.new( 7 ),
        Actions::IpDscp.new( 63 ),
        Actions::IpEcn.new( 43 ),
        Actions::IpProto.new( 17 ),
        Actions::Ipv4SrcAddr.new( "10.10.10.1" ),
        Actions::Ipv4DstAddr.new( "127.0.0.1" ),
        Actions::TcpSrcPort.new( 3331 ),
        Actions::TcpDstPort.new( 3332 ),
        Actions::UdpSrcPort.new( 3333 ),
        Actions::UdpDstPort.new( 3334 ),
        Actions::SctpSrcPort.new( 3336 ),
        Actions::SctpDstPort.new( 3337 ),
        Actions::Icmpv4Type.new( 0 ), # echo reply
        Actions::Icmpv4Code.new( 2 ),
        Actions::ArpOp.new( 1 ), # REQUEST
        Actions::ArpSpa.new( "1.1.1.1" ),
        Actions::ArpTpa.new( "1.1.1.2" ),
        Actions::ArpSha.new( "11:22:33:44:55:67" ),
        Actions::ArpTha.new( "11:22:33:44:55:68" ),
        Actions::Ipv6SrcAddr.new( "ffe1::1" ),
        Actions::Ipv6DstAddr.new( "ffe2::2" ),
        Actions::Ipv6FlowLabel.new( 10 ),
        Actions::Icmpv6Type.new( 1 ), # destination unreachable
        Actions::Icmpv6Code.new( 3 ), # address unreachable if as above type.
        Actions::Ipv6NdTarget.new( "ffe3::3" ),
        Actions::Ipv6NdSll.new( "11:22:33:44:55:69" ),
        Actions::Ipv6NdTll.new( "11:22:33:44:55:70" ),
        Actions::MplsLabel.new( 0 ), # IPv4 Explicit NULL label
        Actions::MplsTc.new( 1 ), # TC refers to traffic class
        Actions::MplsBos.new( 1 ), # Bottom of stack a bit on/off.
        Actions::PbbIsid.new( 10 ), # Provider Backbone bridge i-sid ( service identifier ) 3 bytes
        Actions::TunnelId.new( rand( 2**64 ) ), # Tunnel id applies to GRE packets with RFC2890 key extension
        Actions::Ipv6Exthdr.new( rand( 2**16 ) ),
      ]
      test_flexible_action [ Actions::VlanVid.new( rand( 2**12 ) ) ]
    end
    def basic_actions
      test_basic_action build_basic_actions
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


    def build_basic_actions
      [
        Actions::SendOutPort.new( :port_number => 2, :max_len => 2**7 ),
        Actions::GroupAction.new( :group_id => 1 ),
        Actions::CopyTtlIn.new,
        Actions::CopyTtlOut.new,
        Actions::SetMplsTtl.new( 10 ),
        Actions::DecMplsTtl.new,
        Actions::PushVlan.new( 0x88a8 ),
        Actions::PopVlan.new,
        Actions::PushMpls.new( 0x8847 ),
        Actions::PopMpls.new( 0x8848 ),
        Actions::SetQueue.new( rand( 2**32 ) ),
        Actions::SetField.new( :action_set => [ Actions::VlanVid.new( rand( 4096 ) ) ] ),
        Actions::SetIpTtl.new( rand( 2** 8 ) ),
        Actions::PushPbb.new( rand( 2**16 ) ),
        Actions::PopPbb.new,
        Actions::Experimenter.new( :experimenter => rand( 2 ** 32 ), :body => "abcdef".unpack( "C*" ) ),
      ]
    end
  end
end

t = Trema::Test.new
t.basic_actions
t.flexible_actions
#t.message_list
#t.instruction_list

