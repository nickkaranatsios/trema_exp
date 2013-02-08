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
    class PacketIn < Message
      unsigned_int32 :transaction_id
      unsigned_int64 :datapath_id
      unsigned_int32 :buffer_id
      unsigned_int16 :total_len
      unsigned_int8 :reason
      unsigned_int8 :table_id
      unsigned_int64 :cookie
      attr_accessor :match
      array :data
      # packet info information
      attr_accessor :macsa, :macda
      unsigned_int16 :ether_type

      attr_accessor :vtag
      alias_method :vtag?, :vtag
      attr_accessor :arp
      alias_method :arp?, :arp
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
