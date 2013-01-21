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
  # An action that replaces an existing IP TTL. This action ony applies to
  # IPv4 packets.
  #
  class SetIpTtl < Action
    #
    # An action that replaces an existing IP TTL value. The packet checksum
    # must be re-calculated.
    #
    # @example
    #   SetIpTtl.new( 16 )
    #
    # @param [Integer] ip_ttl
    #   the ip_ttl value to set to.
    #
    # @raise [ArgumentError] if ip_ttl is not specified.
    # @raise [ArgumentError] if ip_ttl is not an unsigned 8-bit integer.
    #
    def initialize ip_ttl
      if ip_ttl.nil?
        raise ArgumentError, "IP TTL is a mandatory option."
      end
      unless ip_ttl.unsigned_8bit?
        raise ArgumentError, "IP TTL must be an unsigned 8-bit integer."
      end
      @ip_ttl = ip_ttl
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
