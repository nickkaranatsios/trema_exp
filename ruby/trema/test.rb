require "trema/action"
#require "trema/match-set"

module Trema
#  module MatchSet
    class MatchInPort < Action
      def initialize in_port
        validate_create :in_port, :validates_with => "check_unsigned_int", :value => in_port
      end


      def append_match actions
        MatchSet::append_match_in_port actions, @in_port
      end
    end
  end
#end
