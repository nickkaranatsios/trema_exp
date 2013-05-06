
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
  def test_basic_actions
    controller_port = OFPP_CONTROLLER
    [
      SendOutPort.new( port_number: controller_port, max_len: 2**7 ),
      GroupAction.new( group_id: 1 ),
      CopyTtlIn.new,
      CopyTtlOut.new,
      SetMplsTtl.new( 10 ),
      DecMplsTtl.new,
      PushVlan.new( 0x88a8 ),
      PopVlan.new,
      PushMpls.new( 0x8847 ),
      PopMpls.new( 0x8848 ),
      SetQueue.new( rand( 2**32 ) ),
      SetField.new( :action_set => [ VlanVid.new( rand( 4096 ) ) ] ),
      SetIpTtl.new( rand( 2** 8 ) ),
      PushPbb.new( rand( 2**16 ) ),
      PopPbb.new,
      Actions::Experimenter.new( experimenter: rand( 2 ** 32 ), body: "abcdef".bytes ),
    ]
  end


  def test_flexible_actions
    [
      InPort.new( 2 ),
      InPhyPort.new( 2 ),
      Metadata.new( 2**34 ),
      EthDst.new( "11:22:33:44:55:66" ),
      EthSrc.new( "11:22:33:44:55:66" ),
      EtherType.new( 0x7777 ),
      VlanVid.new( 1024 ),
      VlanPriority.new( 7 ),
      IpDscp.new( 63 ),
      IpEcn.new( 43 ),
      IpProto.new( 17 ),
      Ipv4SrcAddr.new( "10.10.10.1" ),
      Ipv4DstAddr.new( "127.0.0.1" ),
      TcpSrcPort.new( 3331 ),
      TcpDstPort.new( 3332 ),
      UdpSrcPort.new( 3333 ),
      UdpDstPort.new( 3334 ),
      SctpSrcPort.new( 3336 ),
      SctpDstPort.new( 3337 ),
      Icmpv4Type.new( 0 ), # echo reply
      Icmpv4Code.new( 2 ),
      ArpOp.new( 1 ), # REQUEST
      ArpSpa.new( "1.1.1.1" ),
      ArpTpa.new( "1.1.1.2" ),
      ArpSha.new( "11:22:33:44:55:67" ),
      ArpTha.new( "11:22:33:44:55:68" ),
      Ipv6SrcAddr.new( "ffe1::1" ),
      Ipv6DstAddr.new( "ffe2::2" ),
      Ipv6FlowLabel.new( 10 ),
      Icmpv6Type.new( 1 ), # destination unreachable
      Icmpv6Code.new( 3 ), # address unreachable if as above type.
      Ipv6NdTarget.new( "ffe3::3" ),
      Ipv6NdSll.new( "11:22:33:44:55:69" ),
      Ipv6NdTll.new( "11:22:33:44:55:70" ),
      MplsLabel.new( 0 ), # IPv4 Explicit NULL label
      MplsTc.new( 1 ), # TC refers to traffic class
      MplsBos.new( 1 ), # Bottom of stack a bit on/off.
      PbbIsid.new( 10 ), # Provider Backbone bridge i-sid ( service identifier ) 3 bytes
      TunnelId.new( rand( 2**64 ) ), # Tunnel id applies to GRE packets with RFC2890 key extension
      Ipv6Exthdr.new( rand( 2**16 ) ),
    ]
  end

  def test_flows datapath_id
    action_set_dst = SetField.new( action_set: [ UdpDstPort.new( 1 ) ] )
    bucket_dst = Bucket.new( watch_port: 1, watch_group: 1, weight: 2, actions: [ action_set_dst ] )

    action_set_src = SetField.new( action_set: [ UdpSrcPort.new( 1 ) ] )
    bucket_src = Bucket.new( watch_port: 2, watch_group: 1, weight: 3, actions: [ action_set_src ] )

    group_mod_add = GroupMod.new( group_id: 1, type:  OFPGT_SELECT, buckets: [ bucket_dst ] )
    send_message datapath_id, group_mod_add
    sleep 1
    
    group_mod_add = GroupMod.new( group_id: 2, type:  OFPGT_SELECT, buckets: [ bucket_src ] )
    send_message datapath_id, group_mod_add
    sleep 1
 
    
    group_action = GroupAction.new( 1 )
    redirect_action = SendOutPort.new( port_number: OFPP_CONTROLLER, max_len: OFPCML_NO_BUFFER ) 
    apply_ins = ApplyAction.new( actions:  [ group_action, redirect_action ] ) 
    
    #goto_table_ins = GotoTable.new( table_id: 1 )
#    match = Match.new( in_port: 1, eth_type: 0x88cc )
    #match = Match.new( in_port: 1, eth_type: 2054 )
    match = Match.new( in_port: 1 )
    # to match ping packets use eth_type: 2054 to match lldp packets use 0x88cc
    send_flow_mod_add( datapath_id,
                       priority: OFP_LOW_PRIORITY,
                       buffer_id: OFP_NO_BUFFER,
                       cookie: 1001,
                       match: match,
                       instructions: [ apply_ins ] )
  end


  def test_actions datapath_id
    redirect_action = SendOutPort.new( port_number: OFPP_CONTROLLER, max_len: OFPCML_NO_BUFFER ) 
    apply_ins = ApplyAction.new( actions: [ redirect_action ] )
    match = Match.new( in_port: 1, eth_type: 2048, ip_proto: 17, udp_src: 1 )
    send_flow_mod_add( datapath_id,
                       priority: OFP_LOW_PRIORITY,
                       buffer_id: OFP_NO_BUFFER,
                       cookie: 1001,
                       match: match,
                       instructions: [ apply_ins ] )

    match = Match.new( in_port: 1, eth_type: 2048, ip_proto: 17, udp_dst: 1 )
    #match = Match.new( in_port: 1, eth_type: 2048, eth_src: Trema::Mac.new( "00:00:00:01:00:01" ) )
    send_flow_mod_add( datapath_id,
                       priority: OFP_LOW_PRIORITY,
                       buffer_id: OFP_NO_BUFFER,
                       cookie: 1001,
                       match: match,
                       instructions: [ apply_ins ] )
  end


  def switch_ready datapath_id
    puts "#{ __method__ } datapath_id #{ datapath_id }"
    #test_flexible_actions
    #test_basic_actions
    #test_flows datapath_id
    test_actions datapath_id
  end


  def packet_in datapath_id, message
    @state ||= 0
    @state = @state + 1
    puts message.inspect
    exact_match = ExactMatch.from( message )
    puts exact_match.inspect


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
      send_message datapath_id, Hello.new( xid: 1234 )
    end
    if @state == -1
      send_message datapath_id, FeaturesRequest.new( transaction_id: 2222 )
    end
    if @state == 5
       match = Match.new( in_port: 1, eth_type: 2048, ip_proto: 17, udp_src:1, udp_dst: 1 )
       #match = Match.new( in_port:1, eth_type: 2048, eth_src: Trema::Mac.new( "00:00:00:01:00:01" ) )
       send_flow_multipart_request datapath_id, cookie: 1001, match: match
    end
    if @state == -1
      send_desc_multipart_request datapath_id
    end
    if @state == -1
       match = Match.new( in_port: 1, eth_type: 2048, ip_proto: 17, udp_src:2 )
       send_aggregate_multipart_request datapath_id, table_id: 0, cookie: 1001, match: match
    end
    if @state == -1
      send_table_multipart_request datapath_id
    end
    if @state == -1
      send_port_multipart_request datapath_id
    end
    if @state == -1
#      send_barrier_request datapath_id
      send_table_features_multipart_request datapath_id
    end
    if @state == -1
      send_group_multipart_request datapath_id, group_id: 1
    end
    if @state == -1
      # takes no options all found groups returned
      send_group_desc_multipart_request datapath_id
    end
    if @state == -1
      send_group_features_multipart_request datapath_id
    end
    if @state == -1
      send_port_desc_multipart_request datapath_id
    end
    if @state == -1
      send_queue_multipart_request datapath_id
    end
    if @state == -1
      send_meter_features_multipart_request datapath_id
    end
    if @state == -1
      send_meter_multipart_request datapath_id
    end
    if @state == -1
      send_meter_config_multipart_request datapath_id, meter_id: 5
    end
    if @state == -1
      send_experimenter_multipart_request datapath_id, experimenter: 123, exp_type: 456, user_data: "experimenter message".bytes
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
    match = Match.new( in_port: 1 )
    redirect_action = SendOutPort.new( port_number: OFPP_CONTROLLER, max_len: OFPCML_NO_BUFFER ) 
    apply_ins = ApplyAction.new( actions:  [ redirect_action ] ) 
    send_flow_mod_add( datapath_id,
                       priority: OFP_LOW_PRIORITY,
                       buffer_id: OFP_NO_BUFFER,
                       cookie: 1001,
                       match: match,
                       instructions: [ apply_ins ] )
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


  def group_multipart_reply datapath_id, message
    puts message.inspect
  end


  def group_desc_multipart_reply datapath_id, message
    puts message.inspect
  end


  def group_features_multipart_reply datapath_id, message
    puts message.inspect
  end


  def port_desc_multipart_reply datapath_id, message
    puts message.inspect
  end


  def barrier_reply datapath_id, message
    puts message.inspect
  end


  def multipart_reply datapath_id, message
    puts message.inspect
  end


  def features_reply datapath_id, message
    puts message.inspect
  end
end


#me = TestController.new
#me.run!
#me.start

