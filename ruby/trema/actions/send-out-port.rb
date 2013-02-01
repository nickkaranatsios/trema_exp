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
  module Actions
    #
    # An action to output a packet to a port.
    #
    class SendOutPort < BasicAction
      DEFAULT_MAX_LEN = 2 ** 16 - 1
  
  
      unsigned_int16 :port_number, :presence => true
      unsigned_int16 :max_len
      alias :port :port_number
  
  
      #
      # Creates an action to output a packet to a port.
      #
      # @example
      #   SendOutPort.new( 1 )
      #   SendOutPort.new( :port_number => 1 )
      #   SendOutPort.new( :port_number => OFPP_CONTROLLER, :max_len => 256 )
      #
      # @param [Integer|Hash] options
      #   the port number or the options hash to create this action class instance with.
      #
      # @option options [Number] :port_number
      #   port number an index into switch's physical port list. There are also
      #   fake output ports. For example a port number set to +OFPP_FLOOD+ would
      #   output packets to all physical ports except input port and ports
      #   disabled by STP.
      # @option options [Number] :max_len
      #   the maximum number of bytes from a packet to send to controller when port
      #   is set to +OFPP_CONTROLLER+. A zero length means no bytes of the packet
      #   should be sent. It defaults to 64K.
      #
      # @raise [ArgumentError] if port_number is not an unsigned 16-bit integer.
      # @raise [ArgumentError] if max_len is not an unsigned 16-bit integer.
      #
  #    def initialize options={}
  #      case options
  #        when Hash
  #          port_option = options[ :port_number ] || options[ :port ]
  #          max_len_option = options[ :max_len ] || DEFAULT_MAX_LEN
  #        when Integer
  #          port_option = options
  #          max_len_option = DEFAULT_MAX_LEN
  #        else
  #          raise "Invalid option"
  #      end
  
  #      @port_number, @max_len = port_option, max_len_option
  #      @port_number = port_option
  #    end
  
  
      def to_s
        "SendOutPort: port=#{ @port_number }, max_len=#{ @max_len }"
      end
    end
  
  
    ActionOutput = SendOutPort
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
