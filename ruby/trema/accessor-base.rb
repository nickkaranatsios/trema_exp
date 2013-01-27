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
  #
  # A base class for defining user defined like accessors.
  #
  class AccessorBase
    PRIMITIVE_TYPES = {
      "char" => 8,
      "short" => 16,
      "int" => 32,
      "long" => 64
    }


    USER_DEFINED_TYPES = {
      "ip_addr" => "IPAddr",
      "eth_addr" => "Trema::Mac",
    }


    class << self
      def inherited klass
        PRIMITIVE_TYPES.keys.each do | each |
          primitive_type each
          define_method :"check_unsigned_#{ each }" do | number, name |
            unless number.send( "unsigned_#{ PRIMITIVE_TYPES[ each ] }bit?" )
              raise ArgumentError, "#{ name } must be an unsigned #{ PRIMITIVE_TYPES[ each ] }-bit integer."
            end
          end
        end
        USER_DEFINED_TYPES.each do | k, v |
          user_defined_type k, v
        end
      end


      ############################################################################
      private
      ############################################################################


      def primitive_type method
        self.class.class_eval do
          define_method :"unsigned_#{ method }" do | *args |
            opts = extract_options!( args )
            check_args args
            opts.merge! :attributes => args[ 0 ]
            define_accessor opts
          end
        end
      end


      def user_defined_type method, vcls
        self.class.class_eval do
          define_method :"#{ method }" do | *args |
            opts = extract_options!( args )
            check_args args
            opts.merge! :attributes => args[ 0 ], :user_defined => vcls
            define_accessor opts
          end
        end
      end


      def define_accessor opts
        attr_name = opts[ :attributes ]
        self.class_eval do
          define_method attr_name do
            instance_variable_get "@#{ attr_name }"
          end

          define_method "#{ attr_name }=" do | v |
            if opts.has_key? :presence
              if opts[ :presence ] == true
                if v.nil?
                  raise ArgumentError, "#{ attr_name } is a mandatory option."
                end
              end
            end
            validation_methods = opts.select { | key, value | key == :within or key == :validate_with }
            validation_methods.each do | key, method |
              __send__ method, v, attr_name
            end
            if opts.has_key? :user_defined 
              instance_variable_set "@#{ attr_name }", eval( opts[ :user_defined ] ).new( v )
            else
              instance_variable_set "@#{ attr_name }", v
            end
          end
        end
      end


      def extract_options! args
        if args.last.is_a?( Hash ) && args.last.instance_of?( Hash )
          args.pop
        else
          {}
        end
      end


      def check_args args
        raise ArgumentError, "You need at least one attribute" if args.empty?
        raise ArgumentError, "Too many attributes specified" if args.length > 1
      end
    end
   

    def initialize options=nil
      setters = self.class.instance_methods.select{ | i | i.to_s =~ /[a-z].*=$/ }
      case options
        when Hash
          setters.each do | each |
            opt_key = each.to_s.sub( '=', '' ).to_sym
            if options.has_key? opt_key
              public_send each, options[ opt_key ]
            end
          end
        when Integer, String
          public_send setters[ 0 ], options
        else
          "Invalid option specified"
      end
      set_default setters
    end


    def set_default setters
      setters.each do | each |
        opt_key = each.to_s.sub( '=', '' ).to_sym
        default_opt_key = ( 'DEFAULT_' + opt_key.to_s.upcase ).to_sym
        if self.class.constants.include? default_opt_key
          public_send each, self.class.const_get( default_opt_key )
        end
      end
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
