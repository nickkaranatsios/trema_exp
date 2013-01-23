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
        MatchTcpSrcPort.new( 5555 ),
      ]
      test_match_set ms
    end
  end
end

t = Trema::Test.new
t.match_set
