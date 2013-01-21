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


module Trema
  #
  # An action that pushes a new PBB(Provider BackBone Bridging) header onto
  # the packet. A PBB frame format consists of a supplementary MAC layer that
  # encapsulates another ethernet frame.
  #
  # @example
  #   PushPbb.new
  #
  class PushPbb < Action
    #
    # The PPB ethertype is 0x88e7.
    #
    PBB_ETHER_TAG = 0x88e7


    def initialize
      ether_type = PBB_ETHER_TAG
      validate_create :ether_type, :value => ether_type 
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
