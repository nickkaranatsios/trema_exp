module Accessor
  def self.included klass
    klass.extend ClassMethods
    klass.instance_eval do
      include InstanceMethods
    end
    klass.setup_attributes klass
  end

  module ClassMethods
    class AttributeProperty
      attr_reader :required, :alias, :default


      def initialize
        @required ||= []
        @alias ||= {}
        @default ||= {}
      end
    end


    def attributes
      @attribute_property ||= AttributeProperty.new
    end


    def setup_attributes klass
      primitive_sizes.each do | each |
        define_accessor_meth :"unsigned_int#{ each }"
        define_method :"check_unsigned_int#{ each }" do | number, name |
          begin
            unless number.send( "unsigned_#{ each }bit?" )
              raise ArgumentError, "#{ name } must be an unsigned #{ each }-bit integer."
            end
          rescue NoMethodError
            raise TypeError, "#{ name } must be a Number."
          end
        end
      end
    end


    def primitive_sizes
      ( 8..64 ).step( 8 ).select{ | i | i.to_s( 2 ).count( '1' ) == 1 }
    end

    def define_accessor_meth meth
      class_eval do
        define_method :"#{ meth }" do | *args |
            attrs = args
            opts = extract_options!( args )
            check_args args
            attrs.delete( opts ) unless opts.empty?
            opts.store :attributes, attrs
            opts.store :validate_with, "check_#{ meth }" if meth.to_s[ /unsigned_int\d\d/ ]
            attrs.each do | attr_name |
              define_accessor attr_name, opts
              attributes.required << attr_name if opts.has_key? :presence
              attributes.default[ attr_name ] = opts[ :default ] if opts.has_key? :default
            end
          end
        end
      end


      def define_accessor attr_name, opts
        class_eval do
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
            validation_methods.each { | _, meth | __send__( meth, v, attr_name ) }
            instance_variable_set "@#{ attr_name }", v
          end
        end
      end


    def unsigned_int32 attr, **options
      puts options.inspect
      puts "calling my alias"
      class_eval do
        define_method attr do
          instance_variable_get "@#{ attr }"
        end
        define_method "#{ attr }=" do | v |
          instance_variable_set "@#{ attr }", v
        end
        if options.has_key? :alias
          alias_method options[ :alias ], attr
          attributes.alias[ attr ] = options[ :alias ] if options.has_key? :alias
        end
        attributes.required << attr if options.has_key? :presence
        attributes.default[ attr ] = options[ :default ] if options.has_key? :default
      end
    end
  end

  
  module InstanceMethods
    def initialize options=nil
      setters = self.class.instance_methods.select{ | i | i.to_s =~ /[a-z].+=$/ }.delete_if{ | i | i.to_s =~ /required=/ }
      required = self.class.attributes.required
      if required.empty?
        required = self.class.superclass.attributes.required
      end
      puts "setters=#{ setters.inspect }"
      puts "constructor is called with #{ options.inspect }"
      puts "alias attributes #{ self.class.attributes.alias }"
      set_default setters
      return if options.nil?
      setters.each do | each |
        opt_key = each.to_s.sub( '=', '' ).to_sym
        if options.has_key? opt_key
          public_send each, options[ opt_key ]
        elsif options.has_key? self.class.attributes.alias[ opt_key ]
          public_send each, options[ self.class.attributes.alias[ opt_key ] ] 
        else
          raise ArgumentError, "Required option #{ opt_key } is missing for #{ self.class.name }" if required.include? opt_key 
        end
      end
    end


    def set_default setters
      default = self.class.attributes.default
      setters.each do | each |
        opt_key = each.to_s.sub( '=', '' ).to_sym
        if default.has_key? opt_key
          if default[ opt_key ].respond_to? :call
            public_send each, default[ opt_key ].call
          else
            public_send each, default[ opt_key ]
          end
        end
      end
    end
  end
end

class BasicAction
  include Accessor
end
class Test < BasicAction
  unsigned_int32 :transaction_id, presence: true, default: 123, alias: :xid
  unsigned_int32 :other, presence:true
end
x = Test.new( xid: 456, other: rand( 2**31 ) )
puts "xid = #{ x.xid }"
puts "x is #{ x.inspect }"

