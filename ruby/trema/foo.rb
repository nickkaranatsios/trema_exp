#1. register the handler.
class Controller
  abstract :handle_timer_event
  def initialize(options)
    super(options)
  end


  def run!
    ENV[ 'TREMA_HOME' ] = Trema.home
    $PROGRAM_NAME = name
    # init_trema() call here
    EM.add_periodic_timer( one_second ) do 
      EM.defer { handle_timer_event }
    end
   end
 

   def handle_timer_timer
     if self.respond_to? __method__
       call( __method__ )
     end
   end

   def start
     %w[ packet_in, flow_removed ].each do | handler |
       eval %[ set_handler_"#{ packet_in }" { | datapath_id, reply | { Em.defer{ handle_#{ handler }( datapath_id, reply ) } ]
     end
   end
   

   def handle_packet_in
      
  
    def name
      self.class.to_s.split( "::" ).last
    end
end
