$LOAD_PATH.unshift File.expand_path( File.join( File.dirname( __FILE__ ), "." ) )

require "trema"
class Test < Trema::Controller
end

ms = [ Trema::MatchInPort.new( 1 ), Trema::MatchInPhyPort.new( 2 ) ]
t = Test.new
t.test_match_set( ms )
