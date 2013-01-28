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
      al = [
        SendOutPort.new( :port_number => 2, :max_len => 2**7 ),
        GroupAction.new( :group_id => 1 ),
        CopyTtlIn.new,
        CopyTtlOut.new,
        SetMplsTtl.new( 10 ),
        DecMplsTtl.new,
        PushVlan.new( 0x88a8 ),
        PopVlan.new,
        PushMpls.new( 0x8847 ),
        PopMpls.new( 0x8848 ),
        SetQueue.new( rand( 2**32 ) ),
        SetIpTtl.new( rand( 2** 8 ) ),
        PushPbb.new( rand( 2**16 ) ),
        PopPbb.new,
        ExperimenterAction.new( :experimenter => rand( 2 ** 32 ), :body => "abcdef".unpack( "C*" ) ),
      ]
      test_action_list al
    end
  end
end

t = Trema::Test.new
t.action_list
t.match_set

