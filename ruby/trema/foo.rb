
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
                       priority: OFP_LOW_PRIORITY,
                       buffer_id: OFP_NO_BUFFER,
                       flags: OFPFF_SEND_FLOW_REM, 
                       cookie: 1001,
                       match: Match.new( eth_type: 2054 ),
                       instructions: [ ins ]
    )
  end


  def packet_in datapath_id, message
    @state ||= 0
    @state = @state + 1
    puts message.inspect

    action = Actions::SendOutPort.new( :port_number => OFPP_ALL, :max_len => OFPCML_NO_BUFFER ) 
    send_packet_out( datapath_id, :packet_in => message, :actions => [ action ] )
    if @state == -1
      action = Actions::SendOutPort.new( port_number: OFPP_CONTROLLER )
      match = Match.new( in_port: 2, arp_op: 1 )
      ins = Instructions::ApplyAction.new( actions: [ action ] )
      send_flow_mod_add( datapath_id,
                         match: match,
                         cookie: 1001,
                         instructions: [ ins ]
      )
    end
    if @state == 5
puts "sending flow_multipart_request"
       match = Match.new( eth_type: 2054  )
       send_flow_multipart_request datapath_id, cookie: 1001, match: match
    end
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
exit
  end
end

#me = TestController.new
#me.run!
#me.start

