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
  class SetMplsTtl < Action
    # @return [Fixnum] the value of attribute {#mpls_ttl}
    attr_reader :mpls_ttl
  
    #
    # An action that replaces an existing MPLS TTL. This action applies to
    # packets with an existing MPLS shim header.
    #
    # @example
    #   SetMplsTtl.new( 1 )
    #
    # @param [Integer] group_id
    #   the MPLS TTL to set to.
    #
    # @raise [ArgumentError] if mpls_ttl is not specified.
    # @raise [ArgumentError] if mpls_ttl is not an unsigned 8-bit integer.
    #
    def initialize mpls_ttl
      if mpls_ttl.nil?
        raise ArgumentError, "MPLS TTL is a mandatory option"
      end
      unless mpls_ttl.unsigned_8bit?
        raise ArgumentError, "MPLS TTL must be an unsigned 8-bit integer."
      end
      @mpls_ttl = mpls_ttl
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
