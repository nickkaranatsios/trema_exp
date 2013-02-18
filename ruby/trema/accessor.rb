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
  class Accessor
    USER_DEFINE_TYPES = { ip_addr: "IPAddr",
                           mac: "Mac",
                           match: "Match",
                           packet_info: "Messages::PacketInfo",
                           array: "Array" }


    attr_accessor :required_attributes

    class << self
#      def method_missing meth, *args, &block
#        criteria = USER_DEFINE_TYPES.keys.join( '|' )
#        if meth.to_s =~ /(#{ criteria })/
#          klass = USER_DEFINE_TYPES.include?( meth ) ? USER_DEFINE_TYPES.fetch( meth ) : meth.to_s.camelize
#          return self.__send__ :user_define_type, meth, klass
#        else
#          klass = meth.to_s.capitalize
#          return self.__send__ :user_define_type, meth, klass
#        end
#        super
#      end


      def required_attributes
        @required_attributes ||= []
      end


      def inherited klass
        primitive_sizes.each do | each |
          primitive_type each
          define_method :"check_unsigned#{ each }" do | number, name |
            unless number.send( "unsigned_#{ each }bit?" )
              raise ArgumentError, "#{ name } must be an unsigned #{ each }-bit integer."
            end
          end
        end
        USER_DEFINE_TYPES.each { | k, v | user_define_type( k, v ) }
      end


      ############################################################################
      private
      ############################################################################


      def primitive_sizes
        ( 8..64 ).step( 8 ).select{ | i | i.to_s( 2 ).count( '1' ) == 1 }
      end


      def primitive_type size_value
        self.class.class_eval do
          define_method :"unsigned_int#{ size_value }" do | *args |
            opts = extract_options!( args )
            check_args args
            opts.merge! attributes: args[ 0 ], validate_with: "check_unsigned#{ size_value }"
            define_accessor opts
            self.required_attributes << args[ 0 ] if opts.has_key? :presence
          end
        end
      end


      def user_define_type method, klass 
        self.class.class_eval do
          define_method :"#{ method }" do | *args |
            opts = extract_options!( args )
            check_args args
            opts.merge! attributes: args[ 0 ], user_define: klass 
            define_accessor opts
            self.required_attributes << args[ 0 ] if opts.has_key? :presence
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
            validation_methods = opts.select { | key, _ | key == :within or key == :validate_with }
            validation_methods.each { | _, method | __send__( method, v, attr_name ) }
            if opts.has_key? :user_define 
puts "attr_name = #{ attr_name } v = #{ v.inspect }"
              instance_variable_set "@#{ attr_name }", eval( opts[ :user_define ] ).new( v )
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
      setters = self.class.instance_methods.select{ | i | i.to_s =~ /[a-z].+=$/ }.delete_if{ | i | i.to_s =~ /required_attributes=/ }
      required_attributes = self.class.required_attributes
      if required_attributes.empty?
        required_attributes = self.class.superclass.required_attributes
      end

      case options
      when Hash
        setters.each do | each |
          opt_key = each.to_s.sub( '=', '' ).to_sym
          if options.has_key? opt_key
            public_send each, options[ opt_key ]
          else
            raise ArgumentError, "Required option #{ opt_key } is missing for #{ self.class.name }" if required_attributes.include? opt_key
          end
        end
      when Integer, String
        unless setters.empty?
          public_send setters[ 0 ], options
        else
          raise ArgumentError, "#{ self.class.name } accepts no options"
        end
      else
        raise ArgumentError, "Required option #{ required_attributes.first } missing for #{ self.class.name }" unless required_attributes.empty?
      end
      set_default setters
    end


    def set_default setters
      setters.each do | each |
        opt_key = each.to_s.sub( '=', '' ).to_sym
        default_opt_key = ( 'DEFAULT_' + opt_key.to_s.upcase ).to_sym
        if self.class.constants.include? default_opt_key and instance_variable_get( "@#{ opt_key }" ).nil?
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
