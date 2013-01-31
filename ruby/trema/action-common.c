/*
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


#include <stdint.h>
#include "trema.h"
#include "ruby.h"


uint32_t
nw_addr_to_i( VALUE nw_addr ) {
  return ( uint32_t ) NUM2UINT( rb_funcall( nw_addr, rb_intern( "to_i" ), 0 ) );
}


uint8_t *
dl_addr_to_a( VALUE dl_addr, uint8_t *ret_dl_addr ) {
  VALUE mac_arr = rb_funcall( dl_addr, rb_intern( "to_a" ), 0 );
  int i;

  for ( i = 0; i < RARRAY_LEN( mac_arr ); i++ ) {
    ret_dl_addr[ i ] = ( uint8_t ) ( NUM2INT( RARRAY_PTR( mac_arr )[ i ] ) );
  }
  return ret_dl_addr;
}


openflow_actions *
append_actions( VALUE action_list ) {
  openflow_actions *actions = create_actions();
  VALUE cAction;

  if ( action_list != Qnil ) {
    switch ( TYPE( action_list ) ) {
      case T_ARRAY:
        {
          VALUE *each = RARRAY_PTR( action_list );
          int i;
          
          if ( RARRAY_LEN( action_list ) ) {
            cAction = Data_Wrap_Struct( rb_obj_class( each[ 0 ] ), NULL, delete_actions, actions );
          }
          for ( i = 0; i < RARRAY_LEN( action_list ); i++ ) {
            if ( rb_respond_to( each[ i ], rb_intern( "append_action" ) ) ) {
              rb_funcall( each[ i ], rb_intern( "append_action" ), 1, cAction );
            }
          }
          Data_Get_Struct( cAction, openflow_actions, actions );
printf("no.of actions added %d\n", actions->n_actions );
        }
        break;
      case T_OBJECT:
        if ( rb_respond_to( rb_obj_class( action_list ), rb_intern( "append_action" ) ) ) {
          cAction = Data_Wrap_Struct( action_list, NULL, delete_actions, actions );
          rb_funcall( action_list, rb_intern( "append_action" ), 1, cAction );
        }
        break;
      default:
        rb_raise( rb_eTypeError, "action list argument must be either an Array or an Action object" );
        break;
    }
  }
  return actions;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
