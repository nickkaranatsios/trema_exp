
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

    action_set = SetField.new( :action_set => [ UdpDstPort.new( 1000 ) ] )
    bucket = Bucket.new( watch_port: 1, watch_group: 1, weight: 2, actions: [ action_set ] )
    group_mod_add = GroupMod.new( group_id: 1, type:  OFPGT_SELECT, buckets: [ bucket ] )
    send_message datapath_id, group_mod_add
    sleep 1
    
    group_action = GroupAction.new( 1 )
    redirect_action = SendOutPort.new( port_number: OFPP_CONTROLLER, max_len: OFPCML_NO_BUFFER ) 
    apply_ins = ApplyAction.new( actions:  [ group_action, redirect_action ] ) 
    
    #goto_table_ins = GotoTable.new( table_id: 1 )
    #match = Match.new( in_port: 1, eth_type: 0x88cc )
    #match = Match.new( in_port: 1, eth_type: 2054 )
    match = Match.new( in_port: 1 )
    # to match ping packets use eth_type: 2054 to match lldp packets use 0x88cc
    send_flow_mod_add( datapath_id,
                       priority: OFP_LOW_PRIORITY,
                       buffer_id: OFP_NO_BUFFER,
                       cookie: 1001,
                       match: match,
                       instructions: [ apply_ins ] )
#    send_flow_mod_add( datapath_id,
#                       cookie: 1001,
#                       table_id: 1,
#                       instructions: [ apply_ins ] )
  end


  def packet_in datapath_id, message
    @state ||= 0
    @state = @state + 1
    puts message.inspect

    action = SendOutPort.new( :port_number => 2, :max_len => OFPCML_NO_BUFFER ) 
    send_packet_out( datapath_id, :packet_in => message, :actions => [ action ] )
    if @state == -1
      action = SendOutPort.new( port_number: OFPP_CONTROLLER )
      match = Match.new( in_port: 2, arp_op: 1 )
      ins = ApplyAction.new( actions: [ action ] )
      send_flow_mod_add( datapath_id,
                         match: match,
                         cookie: 1001,
                         instructions: [ ins ]
      )
    end
    if @state == -1
       match = Match.new( in_port: 1, eth_type: 2054  )
       send_flow_multipart_request datapath_id, cookie: 1001, match: match
    end
    if @state == -1
      send_desc_multipart_request datapath_id
    end
    if @state == -1
       match = Match.new( in_port: 1, eth_type: 2054  )
       send_aggregate_multipart_request datapath_id, table_id: 0, cookie: 1001, match: match
    end
    if @state == -1
      send_table_multipart_request datapath_id
    end
    if @state == -1
      send_port_multipart_request datapath_id
    end
    if @state == -1
      send_table_features_multipart_request datapath_id
    end
    if @state == -1
      send_group_multipart_request datapath_id, 1
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
    puts message.inspect
  end


  def desc_multipart_reply datapath_id, message
    puts message.inspect
  end


  def aggregate_multipart_reply datapath_id, message
    puts message.inspect
  end


  def table_multipart_reply datapath_id, message
    puts message.inspect
  end


  def port_multipart_reply datapath_id, message
    puts message.inspect
  end


  def table_features_multipart_reply datapath_id, message
    puts message.inspect
  end
end


#me = TestController.new
#me.run!
#me.start

