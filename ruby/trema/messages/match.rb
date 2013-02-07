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


require "trema/message"


module Trema
  module Messages
    class Match < Message
      unsigned_int32 :in_port
      unsigned_int32 :in_phy_port
      unsigned_int64 :metadata
      unsigned_int64 :metadata_mask
      eth_addr :eth_dst
      eth_addr :eth_dst_mask
      eth_addr :eth_src
      eth_addr :eth_src_mask
      unsigned_int16 :ether_type
      unsigned_int16 :vlan_vid
      unsigned_int16 :vlan_vid_mask
      unsigned_int8 :vlan_priority
      unsigned_int8 :ip_dscp
      unsigned_int8 :ip_ecn
      unsigned_int8 :ip_proto
      ip_addr :ipv4_src
      ip_addr :ipv4_src_mask
      ip_addr :ipv4_dst
      ip_addr :ipv4_dst_mask
      unsigned_int16 :tcp_src
      unsigned_int16 :tcp_dst
      unsigned_int16 :udp_src
      unsigned_int16 :udp_dst
      unsigned_int16 :sctp_src
      unsigned_int16 :sctp_dst
      unsigned_int8 :icmpv4_type
      unsigned_int8 :icmpv4_code
      unsigned_int16 :arp_op
      ip_addr :arp_spa
      ip_addr :arp_spa_mask
      ip_addr :arp_tpa
      ip_addr :arp_tpa_mask
      eth_addr :arp_sha
      eth_addr :arp_sha_mask
      eth_addr :arp_tha
      eth_addr :arp_tha_mask
      ip_addr :ipv6_src
      ip_addr :ipv6_src_mask
      ip_addr :ipv6_dst
      ip_addr :ipv6_dst_mask
      unsigned_int32 :ipv6_flabel
      unsigned_int32 :ipv6_flabel_mask
      unsigned_int8 :icmpv6_type
      unsigned_int8 :icmpv6_code
      ip_addr :ipv6_nd_target
      ip_addr :ipv6_nd_sll
      ip_addr :ipv6_nd_tll
      unsigned_int32 :mpls_label
      unsigned_int8 :mpls_tc
      unsigned_int8 :mpls_bos
      unsigned_int32 :pbb_isid
      unsigned_int64 :tunnel_id
      unsigned_int64 :tunnel_id_mask
      unsigned_int16 :ipv6_exthdr
      unsigned_int16 :ipv6_exthdr_mask
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
