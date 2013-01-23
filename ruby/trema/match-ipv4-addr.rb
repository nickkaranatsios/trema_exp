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


require "trema/match-field"
require "ipaddr"


module Trema
  #
  # A base class for match IPv4 source and destination addresses classes.
  #
  class MatchIpv4Addr < MatchField
    def initialize ipv4_addr
      validate_create :ipv4_addr, :presence => true, :validate_with => "check_ipv4_addr", :value => ipv4_addr
      @ipv4_addr = IPAddr.new( ipv4_addr )
    end


    def check_ipv4_addr ipv4_addr, name
      unless ipv4_addr.is_a? String
        raise ArgumentError, "An IPv4 source/destination address must be a String"
      end
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
