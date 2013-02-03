/*
 * Author: Yasuhito Takamiya <yasuhito@gmail.com>
 *
 * Copyright (C) 2008-2012 NEC Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */


#include "buffer.h"
#include "controller.h"
#include "openflow.h"
#include "ruby.h"
#include "trema.h"
#include "action-common.h"


extern VALUE mTrema;
VALUE cController;


static void
handle_timer_event( void *self ) {
  if ( rb_respond_to( ( VALUE ) self, rb_intern( "handle_timer_event" ) ) == Qtrue ) {
    rb_funcall( ( VALUE ) self, rb_intern( "handle_timer_event" ), 0 );
  }
}


/*
 * @overload send_message(datapath_id, message)
 *   Sends an OpenFlow message to the datapath.
 *
 *   @example
 *     send_message datapath_id, FeaturesRequest.new
 *
 *
 *   @param [Number] datapath_id
 *     the datapath to which a message is sent.
 *
 *   @param [FeaturesRequest] message
 *     the message to be sent.
 */
static VALUE
controller_send_message( VALUE self, VALUE datapath_id, VALUE messages ) {
  VALUE id_pack_msg = rb_intern( "pack_msg" );

  if ( messages != Qnil ) {
    switch ( TYPE( messages ) ) {
      case T_ARRAY:
        {
          VALUE *each = RARRAY_PTR( messages );
          int i;
          
          for ( i = 0; i < RARRAY_LEN( messages ); i++ ) {
            if ( rb_respond_to( each[ i ], id_pack_msg ) ) {
              rb_funcall( each[ i ], id_pack_msg, 1, datapath_id );
            }
          }
        }
        break;
      case T_OBJECT:
        if ( rb_respond_to( rb_obj_class( messages ), id_pack_msg ) ) {
          rb_funcall( messages, id_pack_msg, 1, datapath_id );
        }
        break;
      default:
        rb_raise( rb_eTypeError, "messages argument must be an Array or a message object" );
    }
  }
  return self;
}


#ifdef TEST
static VALUE
get_strict( int argc, VALUE *argv ) {
  VALUE datapath_id = Qnil;
  VALUE options = Qnil;
  VALUE strict = Qnil;

  rb_scan_args( argc, argv, "11", &datapath_id, &options );
  if ( options != Qnil ) {
    strict = rb_hash_aref( options, ID2SYM( rb_intern( "strict" ) ) );
  }
  return strict;
}
#endif



/*
 * Starts this controller. Usually you do not need to invoke
 * explicitly, because this is called implicitly by "trema run"
 * command.
 */
static VALUE
controller_run( VALUE self ) {
  setenv( "TREMA_HOME", RSTRING_PTR( rb_funcall( mTrema, rb_intern( "home" ), 0 ) ), 1 );

  VALUE name = rb_funcall( self, rb_intern( "name" ), 0 );
  rb_gv_set( "$PROGRAM_NAME", name );

  int argc = 3;
  char **argv = xmalloc( sizeof ( char * ) * ( uint32_t ) ( argc + 1 ) );
  argv[ 0 ] = RSTRING_PTR( name );
  argv[ 1 ] = ( char * ) ( uintptr_t ) "--name";
  argv[ 2 ] = RSTRING_PTR( name );
  argv[ 3 ] = NULL;
  init_trema( &argc, &argv );
  xfree( argv );

  struct itimerspec interval;
  interval.it_interval.tv_sec = 1;
  interval.it_interval.tv_nsec = 0;
  interval.it_value.tv_sec = 0;
  interval.it_value.tv_nsec = 0;
  add_timer_event_callback( &interval, handle_timer_event, ( void * ) self );

  if ( rb_respond_to( self, rb_intern( "start" ) ) == Qtrue ) {
    rb_funcall( self, rb_intern( "start" ), 0 );
  }

  rb_funcall( self, rb_intern( "start_trema" ), 0 );

  return self;
}


/*
 * @overload shutdown!
 *   In the context of trema framework stops this controller and its applications.
 */
static VALUE
controller_shutdown( VALUE self ) {
  stop_trema();
  return self;
}


static void
thread_pass( void *user_data ) {
  UNUSED( user_data );
  rb_thread_check_ints();
  rb_funcall( rb_cThread, rb_intern( "pass" ), 0 );
}


/*
 * In the context of trema framework invokes the scheduler to start its applications.
 */
static VALUE
controller_start_trema( VALUE self ) {
  struct itimerspec interval;
  interval.it_interval.tv_sec = 0;
  interval.it_interval.tv_nsec = 1000000;
  interval.it_value.tv_sec = 0;
  interval.it_value.tv_nsec = 0;
  add_timer_event_callback( &interval, thread_pass, NULL );

  start_trema();

  return self;
}


static VALUE
controller_test_basic_action( VALUE self, VALUE action ) {
  pack_basic_action( action );
  return self;
}


static VALUE
controller_test_flexible_action( VALUE self, VALUE oxm_match ) {
  pack_flexible_action( oxm_match );
  return self;
}


static VALUE
controller_test_instructions( VALUE self, VALUE instruction_list ) {
  openflow_instructions *instructions = create_instructions();
  VALUE cInstruction;

  if ( instruction_list != Qnil ) {
    switch ( TYPE( instruction_list ) ) {
      case T_ARRAY:
        {
          VALUE *each = RARRAY_PTR( instruction_list );
          int i;

          if ( RARRAY_LEN( instruction_list ) ) {
            cInstruction = Data_Wrap_Struct( rb_obj_class( each[ 0 ] ), NULL, delete_instructions, instructions );
          }
          for ( i = 0; i < RARRAY_LEN( instruction_list ); i++ ) {
            if ( rb_respond_to( each[ i ], rb_intern( "append_instruction" ) ) ) {
              rb_funcall( each[ i ], rb_intern( "append_instruction" ), 1, cInstruction );
            }
          }
          Data_Get_Struct( cInstruction, openflow_instructions, instructions );
printf("no.of instructions added %d\n", instructions->n_instructions );
        }
        break;
      case T_OBJECT:
        if ( rb_respond_to( rb_obj_class( instruction_list ), rb_intern( "append_instruction" ) ) ) {
          cInstruction = Data_Wrap_Struct( instruction_list, NULL, delete_instructions, instructions );
          rb_funcall( instruction_list, rb_intern( "append_instruction" ), 1, cInstruction );
        }
        break;
      default:
        rb_raise( rb_eTypeError, "instruction list argument must be either an Array or an Instruction object" );
    }
  }
  return self;
     
}


/********************************************************************************
 * Init Controller module.
 ********************************************************************************/

void
Init_controller() {
  rb_require( "trema/app" );
  VALUE cApp = rb_eval_string( "Trema::App" );
  cController = rb_define_class_under( mTrema, "Controller", cApp );

  rb_define_method( cController, "send_message", controller_send_message, 2 );

  rb_define_method( cController, "run!", controller_run, 0 );
  rb_define_method( cController, "shutdown!", controller_shutdown, 0 );
  rb_define_private_method( cController, "start_trema", controller_start_trema, 0 );
  rb_define_method( cController, "test_basic_action", controller_test_basic_action, 1 );
  rb_define_method( cController, "test_flexible_action", controller_test_flexible_action, 1 );
  rb_define_method( cController, "test_instructions", controller_test_instructions, 1 );

  rb_require( "trema/controller" );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
