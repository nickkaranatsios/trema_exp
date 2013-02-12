#
# Copyright (C) 2008-2013 NEC Corporation
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


module Trema
  module Messages
    class PacketInfo < Message
      attr_accessor :eth_type
      attr_accessor :macsa
      attr_accessor :macda
      attr_accessor :ip_dscp

      attr_accessor :vtag
      alias_method :vtag?, :vtag
      attr_accessor :vlan_tci
      attr_accessor :vlan_vid
      attr_accessor :vlan_prio
      attr_accessor :vlan_tpid

      attr_accessor :arp
      alias_method :arp?, :arp
      attr_accessor :arp_op
      attr_accessor :arp_sha
      attr_accessor :arp_spa
      attr_accessor :arp_tpa

      attr_accessor :icmpv4
      alias_method :icmpv4?, :icmpv4

      attr_accessor :icmpv6
      alias_method :icmpv6?, :icmpv6

      attr_accessor :ipv6_src
      attr_accessor :ipv6_dst
      attr_accessor :ipv6_flabel
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
