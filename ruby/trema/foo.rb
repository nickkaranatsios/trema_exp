
class Class
  def abstract *args
    self.class_eval do
      args.each do | method_name |
        define_method method_name do | *args |
          raise NotImplementedError.new( "Unimplemented abstract method #{ self.class.name }##{ method_name }" )
        end
      end
    end
  end
end


 
#1. register the handler.
#class Controller
#  include MessageHandler
#  abstract :handle_timer_event
#
#
#  def run!
#    ENV[ 'TREMA_HOME' ] = `pwd`
#    $PROGRAM_NAME = name
#   end
#
#
#   def start
#     %w[ packet_in flow_removed ].each do | handler |
#       eval %[ set_#{ handler }_handler() { | datapath_id, message | EM.defer{ #{ handler }( datapath_id, message ) } } ]
#     end
#     one_second = 1
#     EM.run do
#       EM.add_periodic_timer( one_second ) { handle_timer_event }
#       start_trema
#     end
#   end
# 
#
#   def handle_timer_event
#   end
#
#  def name
#    self.class.to_s.split( "::" ).last
#  end
#end


class FooController < Controller
#  def start
#     %w[ packet_in flow_removed ].each do | handler |
#       eval %[ set_#{ handler }_handler() { | datapath_id, message | EM.defer{ #{ handler }( datapath_id, message ) } } ]
#     end
#  end


  def switch_ready datapath_id
puts "#{ __method__ } datapath_id #{ datapath_id }"
    action = Actions::SendOutPort.new( port_number: OFPP_CONTROLLER, max_len: OFPCML_NO_BUFFER ) 
    ins = Instructions::ApplyAction.new( actions:  [ action ] ) 
    send_flow_mod_add( datapath_id,
                       :priority => OFP_LOW_PRIORITY,
                       :buffer_id => OFP_NO_BUFFER,
                       :flags => OFPFF_SEND_FLOW_REM, 
                       :table_id => 0,
                       :cookie => 1001,
                       :hard_timeout => 0,
                       :idle_timeout => 0,
                       :out_port => 1,
                       :out_group => 1,
                       :instructions => [ ins ]
    )
  end


  def packet_in datapath_id, message
    @state ||= 0
    @state = @state + 1
    puts message.inspect
   
    match = ExactMatch.from( message )
    action = Actions::SendOutPort.new( :port_number => OFPP_ALL, :max_len => OFPCML_NO_BUFFER ) 
    send_packet_out( datapath_id, :packet_in => message, :actions => [ action ] )
    if @state == 1
      gm_with_no_buckets = Messages::GroupMod.new( group_id: 1, type: OFPGT_ALL )
      send_message datapath_id, gm_with_no_buckets
    end
    if @state == 2
      action = Actions::PushVlan.new ( 0x88a8 )
      bucket = Messages::Bucket.new( watch_port: 1, watch_group: 1, weight: 2, actions: [ action ] )
      gm_with_buckets = Messages::GroupMod.new( group_id: 1, command: OFPGC_MODIFY, type: OFPGT_ALL, buckets: [ bucket ] )
      send_message datapath_id, gm_with_buckets
    end
    if @state == 5
puts "sending flow_multipart_request"
       match = Match.new( in_port: 1 )
#      send_message datapath_id, Messages::GroupMod.new( group_id: 1, type: OFPGT_ALL, command: OFPGC_DELETE )
       send_flow_multipart_request datapath_id, table_id: 1, cookie: 1001, match: match
    end
#    send_group_mod( datapath_id, group_id: 1, type: OFPGT_ALL )
  end


  def error datapath_id, message
    puts message.inspect
  end


  def port_status datapath_id, port
    puts port.inspect
  end


  def flow_removed datapath_id, message
    puts message.inspect
  end


  def flow_multipart_reply datapath_id, message
puts "received a flow_multipart_reply"
    puts message.inspect
  end
end

#me = TestController.new
#me.run!
#me.start

