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


require "forwardable"


module Trema
  module Messages
    class PacketIn < Message
      extend Forwardable

      unsigned_int32 :transaction_id
      unsigned_int64 :datapath_id
      unsigned_int32 :buffer_id
      unsigned_int16 :total_len
      unsigned_int8 :reason
      unsigned_int8 :table_id
      unsigned_int64 :cookie
      attr_accessor :packet_info
      
      # packet info information
      def_delegator :@packet_info, :eth_type
      def_delegator :@packet_info, :macsa
      def_delegator :@packet_info, :macda

      def_delegator :@packet_info, :vtag
      def_delegator :@packet_info, :vtag_tci
      def_delegator :@packet_info, :vtag_vid
      def_delegator :@packet_info, :vtag_prio
      def_delegator :@packet_info, :vtag_tpid

      def_delegator :@packet_info, :arp
      def_delegator :@packet_info, :arp_op
      def_delegator :@packet_info, :arp_sha
      def_delegator :@packet_info, :arp_spa
      def_delegator :@packet_info, :arp_tpa

      def_delegator :@packet_info, :icmpv4

      def_delegator :@packet_info, :icmpv6

      def_delegator :@packet_info, :ipv6_src
      def_delegator :@packet_info, :ipv6_dst
      def_delegator :@packet_info, :ipv6_flabel
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
