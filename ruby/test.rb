$LOAD_PATH.unshift File.expand_path( File.join( File.dirname( __FILE__ ), "." ) )

require "trema"
module Trema
  class Test < Controller
    def match_set
      ms = [ 
        MatchInPort.new( 1 ), 
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
      ]
      test_match_set ms
    end
  end
end

t = Trema::Test.new
t.match_set
