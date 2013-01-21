#
# Copyright (C) 2008-2012 NEC Corporation
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


require "trema/action"
require "trema/monkey-patch/integer"


module Trema
  #
  # An action that sets the queue id for the packet.
  #
  class SetQueue < Action
    # @return [Fixnum] the value of attribute {#queue_id}
    attr_reader :queue_id


    #
    # An action that sets the queue id for the packet.
    #
    # @example
    #   SetQueue.new( 1 )
    #
    # @param [Integer] queue_id
    #   the queue id to set to.
    #
    # @raise [ArgumentError] if queue_id is not specified.
    # @raise [ArgumentError] if queue_id is not an unsigned 32-bit integer.
    #
    def initialize queue_id
      if queue_id.nil?
        raise ArgumentError, "Queue id is a mandatory option"
      end
      unless queue_id.unsigned_32bit?
        raise ArgumentError, "Queue id must be an unsigned 32-bit integer."
      end
      @queue_id = queue_id
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
