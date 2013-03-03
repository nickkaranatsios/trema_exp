$LOAD_PATH.unshift File.expand_path( File.join( File.dirname( __FILE__ ), "." ) )

module ClassMethods
  def store( subclass, id )
    ClassMethods._instances[ id ] = subclass
  end

  def retrieve
    ClassMethods._instances
  end

  def self._instances
    @_instances ||= Hash.new{ |h,k| h[k] = [] }
  end
end


class Base
  extend ClassMethods
  class << self
     attr_accessor :test
    def action_type type
      self.test ||= {}
      self.test[ type ] = self
      store( self, type )
puts self.test.inspect
    end

    def search
      puts test.inspect
#      retrieve
    end
  end
end

class SendOutPort < Base
  action_type 1
end

class EthSrc < Base
  action_type 2
end

puts Base.search
exit

class Fixnum
  def unsigned_16bit?
    true
  end
  def unsigned_32bit?
    true
  end
  def unsigned_64bit?
    true
  end
end

module Trema
  #
  # A base class for defining user defined like accessors.
  #
  class Accessor
    USER_DEFINED_TYPES = %w( ip_addr mac match packet_info array string bool )


    attr_accessor :required_attributes

    class << self
      def required_attributes
        @required_attributes ||= []
      end


      def inherited klass
        primitive_sizes.each do | each |
          define_accessor_meth :"unsigned_int#{ each }"
          define_method :"check_unsigned_int#{ each }" do | number, name |
            unless number.send( "unsigned_#{ each }bit?" )
              raise ArgumentError, "#{ name } must be an unsigned #{ each }-bit integer."
            end
          end
        end
        USER_DEFINED_TYPES.each{ | meth | define_accessor_meth meth }
      end


      ############################################################################
      private
      ############################################################################


      def primitive_sizes
        ( 8..64 ).step( 8 ).select{ | i | i.to_s( 2 ).count( '1' ) == 1 }
      end


      def define_accessor_meth meth
        self.class.class_eval do
          define_method :"#{ meth }" do | *args |
            attrs = args
            opts = extract_options!( args )
            check_args args
            attrs.delete( opts ) unless opts.empty?
            opts.store :attributes, attrs
            opts.store :validate_with, "check_#{ meth }" if meth.to_s[ /unsigned_int\d\d/ ]
            attrs.each do | attr_name |
              define_accessor attr_name, opts
              self.required_attributes << attr_name if opts.has_key? :presence
            end
          end
        end
      end


      def define_accessor attr_name, opts
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
            validation_methods.each { | _, meth | __send__( meth, v, attr_name ) }
            instance_variable_set "@#{ attr_name }", v
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


module Trema
  module Messages
    class MultipartBase < Accessor
      unsigned_int64 :datapath_id
      unsigned_int32 :transaction_id
      unsigned_int16 :type, :flags
    end
  end
end


module Trema
  module Messages
    class FlowMultipartReply < MultipartBase
      unsigned_int16 :length
      unsigned_int8 :table_id
      unsigned_int32 :duration_sec, :duration_nsec
      unsigned_int16 :priority
      unsigned_int16 :idle_timeout, :hard_timeout
      unsigned_int16 :flags
      unsigned_int64 :cookie
      unsigned_int64 :packet_count, :byte_count
    end
  end
end

x = Trema::Messages::FlowMultipartReply.new( datapath_id: 0xabc, transaction_id: 1, type: 1, flags: 0, length: 84, packet_count: 1233 )
puts x.datapath_id
exit


require "trema"
module Trema
  class Test < Controller
    def flexible_actions
      test_flexible_action create_flexible_actions
    end


    def basic_actions
      test_basic_action create_basic_actions
    end

    def test_flow_mod_add
      action = Actions::SendOutPort.new( :port_number => OFPP_CONTROLLER, :max_len => OFPCML_NO_BUFFER ) 
      ins = Instructions::ApplyAction.new( :actions => [ action ] ) 
      datapath_id = 0xabc
      send_flow_mod_add( datapath_id, 
                         :priority => OFP_LOW_PRIORITY,
                         :buffer_id => OFP_NO_BUFFER,
                         :flags => OFPFF_SEND_FLOW_REM, 
                         :out_port => 0,
                         :out_group => 0,
                         :instructions => [ ins ] )
    end


    def messages
      action = Actions::PushVlan.new ( 0x88a8 )
      bucket = Messages::Bucket.new( watch_port: 1, watch_group: 1, weight: 2, actions: [ action ] )
      ml = [
        Messages::FlowMultipartRequest.new( table_id: 1, out_port: 2, out_group: 1, cookie: 0xffaaddee ),
        Messages::GroupMod.new( :group_id => 1, :type => OFPGT_ALL, :buckets => [ bucket ] ),
        Messages::Hello.new( :transaction_id => 123, :version => [ 0x4 ] ),
        Messages::EchoRequest.new( :transaction_id => 123, :user_data => "abcdefgh".unpack( "C" ) ),
        Messages::FeaturesRequest.new( :transaction_id => 123 ),
        Messages::GetConfigRequest.new( :transaction_id => 123 ),
        Messages::SetConfig.new( :transaction_id => 123, :flags => OFPC_FRAG_NORMAL, :miss_send_len => OFPCML_NO_BUFFER ),
      ]
      send_message 0x1, ml
    end

    def instructions
      ins = [
        Instructions::GotoTable.new( :table_id => 2 ),
        Instructions::WriteMetadata.new( :metadata => rand( 2**64 ), :metadata_mask => rand( 2**64 ) ),
        Instructions::WriteAction.new( :actions => create_basic_actions ),
        Instructions::ApplyAction.new( :actions => create_basic_actions ),
        Instructions::ClearAction.new,
        Instructions::Meter.new( rand( 2**32 ) ),
        Instructions::Experimenter.new( :experimenter => rand( 2 ** 32 ), :user_data => rand( 36**10 ).to_s( 36 ).unpack( "C" ) ),
      ]
      test_instruction ins
    end


    def create_basic_actions
      controller_port = OFPP_CONTROLLER
      [
        Actions::SendOutPort.new( :port_number => controller_port, :max_len => 2**7 ),
        Actions::GroupAction.new( :group_id => 1 ),
        Actions::CopyTtlIn.new,
        Actions::CopyTtlOut.new,
        Actions::SetMplsTtl.new( 10 ),
        Actions::DecMplsTtl.new,
        Actions::PushVlan.new( 0x88a8 ),
        Actions::PopVlan.new,
        Actions::PushMpls.new( 0x8847 ),
        Actions::PopMpls.new( 0x8848 ),
        Actions::SetQueue.new( rand( 2**32 ) ),
        Actions::SetField.new( :action_set => [ Actions::VlanVid.new( rand( 4096 ) ) ] ),
        Actions::SetIpTtl.new( rand( 2** 8 ) ),
        Actions::PushPbb.new( rand( 2**16 ) ),
        Actions::PopPbb.new,
        Actions::Experimenter.new( :experimenter => rand( 2 ** 32 ), :body => "abcdef".unpack( "C*" ) ),
      ]
    end


    def create_flexible_actions
      [
        Actions::InPort.new( 2 ),
        Actions::InPhyPort.new( 2 ), 
        Actions::Metadata.new( 2**34 ),
        Actions::EthDst.new( "11:22:33:44:55:66" ),
        Actions::EthSrc.new( "11:22:33:44:55:66" ),
        Actions::EtherType.new( 0x7777 ),
        Actions::VlanVid.new( 1024 ),
        Actions::VlanPriority.new( 7 ),
        Actions::IpDscp.new( 63 ),
        Actions::IpEcn.new( 43 ),
        Actions::IpProto.new( 17 ),
        Actions::Ipv4SrcAddr.new( "10.10.10.1" ),
        Actions::Ipv4DstAddr.new( "127.0.0.1" ),
        Actions::TcpSrcPort.new( 3331 ),
        Actions::TcpDstPort.new( 3332 ),
        Actions::UdpSrcPort.new( 3333 ),
        Actions::UdpDstPort.new( 3334 ),
        Actions::SctpSrcPort.new( 3336 ),
        Actions::SctpDstPort.new( 3337 ),
        Actions::Icmpv4Type.new( 0 ), # echo reply
        Actions::Icmpv4Code.new( 2 ),
        Actions::ArpOp.new( 1 ), # REQUEST
        Actions::ArpSpa.new( "1.1.1.1" ),
        Actions::ArpTpa.new( "1.1.1.2" ),
        Actions::ArpSha.new( "11:22:33:44:55:67" ),
        Actions::ArpTha.new( "11:22:33:44:55:68" ),
        Actions::Ipv6SrcAddr.new( "ffe1::1" ),
        Actions::Ipv6DstAddr.new( "ffe2::2" ),
        Actions::Ipv6FlowLabel.new( 10 ),
        Actions::Icmpv6Type.new( 1 ), # destination unreachable
        Actions::Icmpv6Code.new( 3 ), # address unreachable if as above type.
        Actions::Ipv6NdTarget.new( "ffe3::3" ),
        Actions::Ipv6NdSll.new( "11:22:33:44:55:69" ),
        Actions::Ipv6NdTll.new( "11:22:33:44:55:70" ),
        Actions::MplsLabel.new( 0 ), # IPv4 Explicit NULL label
        Actions::MplsTc.new( 1 ), # TC refers to traffic class
        Actions::MplsBos.new( 1 ), # Bottom of stack a bit on/off.
        Actions::PbbIsid.new( 10 ), # Provider Backbone bridge i-sid ( service identifier ) 3 bytes
        Actions::TunnelId.new( rand( 2**64 ) ), # Tunnel id applies to GRE packets with RFC2890 key extension
        Actions::Ipv6Exthdr.new( rand( 2**16 ) ),
      ]
    end
  end
end

t = Trema::Test.new
#t.test_flow_mod_add
#t.basic_actions
#t.flexible_actions
t.messages
#t.instructions


