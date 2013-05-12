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


require "packetfu"


class PacketGen
  def initialize **opts
    @tcp_pkt = PacketFu::TCPPacket.new( :config => opts[ :iface ], :flavor => "Linux" )
    @udp_pkt = PacketFu::UDPPacket.new( :config => opts[ :iface ], :flavor => "Linux" )
    @arp_pkt = PacketFu::ARPPacket.new( :config => opts[ :iface ], :flavor => "Linux" )
  end


  def send_tcp_packet **opts
    set_any_defaults @tcp_pkt, opts
    @tcp_pkt.recalc
    @tcp_pkt.tcp_src = opts[ :tcp_src ] if opts.has_key? :tcp_src
    @tcp_pkt.tcp_dst = opts[ :tcp_dst ] if opts.has_key? :tcp_dst
    inject_pkt @tcp_pkt, opts[ :iface ]
  end


  def send_udp_packet **opts
    set_any_defaults @udp_pkt, opts
    @udp_pkt.udp_src = opts[ :udp_src ] if opts.has_key? :udp_src
    @udp_pkt.udp_dst = opts[ :udp_dst ] if opts.has_key? :udp_dst
    @udp_pkt.recalc
    inject_pkt @udp_pkt, opts[ :iface ]
  end


  private


  def set_any_defaults pkt, opts
    pkt.ip_saddr = opts[ :ip_saddr ] if opts.has_key? :ip_saddr
    pkt.ip_daddr = opts[ :ip_daddr ] if opts.has_key? :ip_daddr
    pkt.payload = "this is a test"
  end


  def inject_pkt pkt, iface
    inj = PacketFu::Inject.new( :iface => iface )
    inj.array_to_wire( :array => [ pkt.to_s ] )
  end
end


def send_tcp_packet **opts
  pkt = PacketGen opts[ :iface ] 
  pkt.send_tcp_packet opts
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
