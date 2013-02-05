require "eventmachine"

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


module MessageHandler
  def set_packet_in_handler
puts __method__
  end

  def set_flow_removed_handler
puts __method__
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
  include MessageHandler
  def start
     %w[ packet_in flow_removed ].each do | handler |
       eval %[ set_#{ handler }_handler() { | datapath_id, message | EM.defer{ #{ handler }( datapath_id, message ) } } ]
     end
  end

  def packet_in datapath_id, message
puts __method__
  end

  def set_flow_removed datapath_id, message
puts __method__
  end
end

#me = TestController.new
#me.run!
#me.start

