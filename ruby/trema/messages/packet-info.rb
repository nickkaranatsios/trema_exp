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
      attr_accessor :eth_src, :eth_dst
      attr_accessor :eth_type

      attr_accessor :ip_dscp, :ip_ecn, :ip_proto

      attr_accessor :vtag
      alias_method :vtag?, :vtag
      attr_accessor :vlan_vid, :vlan_tci, :vlan_prio, :vlan_tpid

      attr_accessor :arp
      alias_method :arp?, :arp

      attr_accessor :ipv4
      alias_method :ipv4?, :ipv4

      attr_accessor :ipv6
      alias_method :ipv6?, :ipv6

      attr_accessor :arp_request
      alias_method :arp_request?, :arp_request

      attr_accessor :arp_reply
      alias_method :arp_reply?, :arp_reply

      attr_accessor :arp_op, :arp_sha, :arp_spa, :arp_tha, :arp_tpa

      attr_accessor :icmpv4
      alias_method :icmpv4?, :icmpv4

      attr_accessor :icmpv4_type, :icmpv4_code

      attr_accessor :icmpv6
      alias_method :icmpv6?, :icmpv6

      attr_accessor :icmpv6_type, :icmpv6_code
      attr_accessor :ipv6_nd_target, :ipv6_nd_sll, :ipv6_nd_tll

      attr_accessor :ipv4_src, :ipv4_dst

      attr_accessor :tcp, :tcp_src, :tcp_dst
      alias_method :tcp?, :tcp

      attr_accessor :udp, :udp_src, :udp_dst
      alias_method :udp?, :udp

      attr_accessor :sctp, :sctp_src, :sctp_dst
      alias_method :sctp?, :sctp

      attr_accessor :ipv6_src, :ipv6_dst
      attr_accessor :ipv6_flabel

      attr_accessor :mpls, :mpls_label, :mpls_tc, :mpls_bos
      alias_method :mpls?, :mpls
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
