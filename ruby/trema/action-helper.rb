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


module Trema
  module ActionHelper
    def validate_create *attributes
      #
      # extracts the hash opptions after extraction the attributes holds
      # the attribute name.
      #
      options = extract_options!( attributes )
      raise ArgumentError, "You need at least one attribute" if attributes.empty?
      options.merge!( :attributes => attributes )
      validates_attribute options
      create_attribute options
    end

    protected

    #
    # @raise [ArgumentError] if number is not an unsigned 16-bit integer.
    #
    def check_unsigned_short number, attr_name
      unless number.unsigned_16bit?
        raise ArgumentError, "#{ attr_name } must be an unsigned 16-bit integer"
      end
    end


    #
    # @raise [ArgumentError] if number is not unsigned 32-bit integer.
    #
    def check_unsigned_int number, attr_name
      unless number.unsigned_32bit?
        raise ArgumentError, "#{ attr_name } must be an unsigned 32-bit integer"
      end
    end

    ############################################################################
    private
    ############################################################################


    #
    # Checks that mandatory attribute is present and calls validation methods
    # specified by validation keys.
    #
    # @raise [ArgumentError] if a mandatory attribute is not present.
    # @raise [ArgumentError] if any of the called validation methods fails.
    #
    def validates_attribute options
      if options.has_key?( :presence )
        if options[ :presence ] == true
          if options[ :value ].nil?
            raise ArgumentError, "#{ options[ :attributes ][ 0 ].to_s } is a mandatory option."
          end
        end
      end
      validation_methods = options.select { | key, value | key == :within or key == :validate_with }
      validation_methods.each do | key, method |
        __send__ method, options[:value ], options[ :attributes ][ 0 ]
      end
    end


    #
    # Extracts and returns the hash of options specified for the attribute
    #
    # @return [Hash] a hash of options specified or an empty hash.
    #
    def extract_options! attributes
      if attributes.last.is_a?( Hash ) && attributes.last.instance_of?( Hash )
        attributes.pop
      else
        {}
      end
    end


    #
    # Creates attribute accessor methods for caller class and sets specified
    # value.
    #
    def create_attribute options
      attributes = options[ :attributes ]
      attr_value = options[ :value ]
      attributes.each do | attr_name |
        self.class.class_eval do
          define_method attr_name do
            instance_variable_get "@#{ attr_name }"
          end
          define_method "#{ attr_name }=" do | value |
            instance_variable_set( "@#{ attr_name }", value )
          end
        end
        self.public_send( "#{ attr_name }=", attr_value )
      end
    end
  end



end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
