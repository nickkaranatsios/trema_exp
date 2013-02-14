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
  class Match < Message
    unsigned_int32 :in_port
    unsigned_int32 :in_phy_port
    unsigned_int64 :metadata
    unsigned_int64 :metadata_mask
    eth_addr :eth_src
    eth_addr :eth_src_mask
    eth_addr :eth_dst
    eth_addr :eth_dst_mask
    unsigned_int16 :eth_type
    unsigned_int16 :vlan_vid
    unsigned_int16 :vlan_vid_mask
    unsigned_int8 :vlan_pcp
    unsigned_int8 :ip_dscp # IP DSCP ( 6 bits in ToS field )
    unsigned_int8 :ip_ecn # IP ECN ( 2 bits in ToS field )
    unsigned_int8 :ip_proto # ip protocol
    attr_accessor :ipv4_src
    attr_accessor :ipv4_src_mask
    attr_accessor :ipv4_dst
    attr_accessor :ipv4_dst_mask
    unsigned_int16 :tcp_src
    unsigned_int16 :tcp_dst
    unsigned_int16 :udp_src
    unsigned_int16 :udp_dst
    unsigned_int16 :sctp_src
    unsigned_int16 :sctp_dst
    unsigned_int8 :icmpv4_type
    unsigned_int8 :icmpv4_code
    unsigned_int16 :arp_op
    attr_accessor :arp_spa
    attr_accessor :arp_spa_mask
    attr_accessor :arp_tpa
    attr_accessor :arp_tpa_mask
    eth_addr :arp_sha
    eth_addr :arp_sha_mask
    eth_addr :arp_tha
    eth_addr :arp_tha_mask
    attr_accessor :ipv6_src
    attr_accessor :ipv6_src_mask
    attr_accessor :ipv6_dst
    attr_accessor :ipv6_dst_mask
    unsigned_int32 :ipv6_flabel
    unsigned_int32 :ipv6_flabel_mask
    unsigned_int8 :icmpv6_type
    unsigned_int8 :icmpv6_code
    attr_accessor :ipv6_nd_target
    attr_accessor :ipv6_nd_sll
    attr_accessor :ipv6_nd_tll
    unsigned_int32 :mpls_label
    unsigned_int8 :mpls_tc
    unsigned_int8 :mpls_bos
    unsigned_int32 :pbb_isid
    unsigned_int64 :tunnel_id
    unsigned_int64 :tunnel_id_mask
    unsigned_int16 :ipv6_exthdr
    unsigned_int16 :ipv6_exthdr_mask
    
    
    def self.from message
      options = {}
      options = message.match.class.instance_methods( false ).grep( /[a-z].+=$/ ).inject( {} ) do | options, attr |
        options.merge( attr.to_s => process( message, attr ) )
      end
      puts options.inspect


#      options[ :in_port ] = message.in_port
#      options[ :eth_type ] = message.eth_type unless message.eth_type.nil?
#      options[ :eth_src ] = message.eth_src unless message.eth_src.nil?
#      options[ :eth_src_mask ] = 0
#      options[ :eth_dst ] = message.eth_dst unless message.eth_dst.nil?
#      options[ :eth_dst ] = 0
#      options[ :vlan_vid ] = 0
#      options[ :vlan_vid_mask ] = 0
#      if message.vtag?
#        options[ :vlan_vid ] = message.vlan_vid
#        options[ :vlan_pcp ] = message.vlan_pcp
#      end
#      if message.ipv4?
#        options[ :ip_proto ] = message.ip_proto
#        options[ :ipv4_src ] = message.ipv4_src
#        options[ :ipv4_dst ] = message.ipv4_dst
#      end
#
#      if message.ipv6?
#        options[ :ipv6_src ] = message.ipv6_src
#        options[ :ipv6_dst ] = message.ipv6_dst
#        options[ :ipv6_flabel ] = message.ipv6_flabel
#        options[ :ipv6_exthdr ] = message.ipv6_exthdr
#      end
#      
#      if message.arp?
#        options[ :arp_op ] = message.arp_op
#        options[ :arp_sha ] = message.arp_sha
#        options[ :arp_spa ] = message.arp_spa
#        options[ :arp_tpa ] = message.arp_tpa
#      end
#
#      if message.icmpv6?
#        options[ :icmpv6_type ] = message.icmpv6_type
#        options[ :icmp6_code ] = message.icmpv6_code
#        options[ :ipv6_nd_target ] = message.ipv6_nd_target unless message.ipv6_nd_target.nil?
#        options[ :ipv6_nd_sll ] = message.ipv6_nd_sll unless message.ipv6_nd_sll.nil?
#        options[ :ipv6_nd_tll ] = message.ipv6_nd_tll unless message.ipv6_nd_tll.nil?
#     end
#
#     if message.icmpv4?
#       options[ :icmpv4_type ] = message.icmpv4_type unless message.icmpv4_type.nil?
#       options[ :icmpv4_code ] = message.icmpv4_code unless message.icmpv4_code.nil?
#     end
#
#     if message.tcp?
#       options[ :tcp_src ] = message.tcp_src
#       options[ :tcp_dst ] = message.tcp_dst
#     end
#
#     if message.udp?
#       options[ :udp_src ] = message.udp_src
#       options[ :udp_dst ] = message.udp_dst
#     end
#
#     if message.sctp?
#       options[ :sctp_src ] = message.sctp_src
#       options[ :sctp_dst ] = message.sctp_dst
#     end
#
#     if message.mpls?
#       options[ :mpls_label ] = message.mpls_label
#       options[ :mpls_tc ] = message.mpls_tc
#       options[ :mpls_bos ] = message.mpls_bos
#     end
#
#     if message.pbb?
#       options[ :pbb_isid ] = message.pbb_isid
#     end

      self.new options
    end


    def self.process match, attr
      get_attr = attr.to_s.sub( '=', '' )
      ret = if match.respond_to? get_attr
        match.__send__ get_attr
      end
      if ret.nil?
        return 0 
      else
        ret
      end 
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
