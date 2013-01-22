module Trema
  module MatchSet
    def append_match_in_port actions, in_port
      actions << in_port
    end
    module_function :append_match_in_port
  end
end
require "trema/test"
