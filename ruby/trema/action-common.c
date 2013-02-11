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


uint8_t *
mac_addr_to_cstr( VALUE mac_addr ) {
  uint8_t dl_addr[ OFP_ETH_ALEN ];
  return dl_addr_to_a( mac_addr, dl_addr );
}


openflow_actions *
pack_basic_action( VALUE action ) {
  openflow_actions *actions = create_actions();
  VALUE cAction = Qnil;

  if ( action != Qnil ) {
    switch ( TYPE( action ) ) {
      case T_ARRAY: {
          VALUE *each = RARRAY_PTR( action );

          for ( int i = 0; i < RARRAY_LEN( action ); i++ ) {
            if ( rb_respond_to( each[ i ], rb_intern( "pack_action" ) ) ) {
              cAction = Data_Wrap_Struct( rb_obj_class( each[ i ] ), NULL, NULL, actions );
              rb_funcall( each[ i ], rb_intern( "pack_action" ), 1, cAction );
            }
          }
      }
      break;
      case T_OBJECT:
        if ( rb_respond_to( action, rb_intern( "pack_action" ) ) ) {
          cAction = Data_Wrap_Struct( rb_obj_class( action ), NULL, NULL, actions );
          rb_funcall( action, rb_intern( "pack_action" ), 1, cAction );
        }
      break;
      default:
        rb_raise( rb_eTypeError, "Action argument must be either an Array or an Action object" );
      break;
    }
  }
  Data_Get_Struct( cAction, openflow_actions, actions );
  return actions;
}


oxm_matches *
pack_flexible_action( VALUE action ) {
  oxm_matches *oxm_match = create_oxm_matches();
  VALUE cOxmMatch = Qnil;

  if ( action != Qnil ) {
    switch ( TYPE( action ) ) {
      case T_ARRAY: {
          VALUE *each = RARRAY_PTR( action );

          for ( int i = 0; i < RARRAY_LEN( action ); i++ ) {
            if ( rb_respond_to( each[ i ], rb_intern( "pack_match" ) ) ) {
              cOxmMatch = Data_Wrap_Struct( rb_obj_class( each[ i ] ), NULL, NULL, oxm_match );
              rb_funcall( each[ i ], rb_intern( "pack_match" ), 1, cOxmMatch );
            }
          }
      }
      break;
      case T_OBJECT:
        if ( rb_respond_to( action, rb_intern( "pack_match" ) ) ) {
          cOxmMatch = Data_Wrap_Struct( rb_obj_class( action ), NULL, NULL, oxm_match );
          rb_funcall( action, rb_intern( "pack_match" ), 1, cOxmMatch );
        }
      break;
      default:
        rb_raise( rb_eTypeError, "Action argument must be either an Array or an Action object" );
      break;
    }
  }
  Data_Get_Struct( cOxmMatch, oxm_matches, oxm_match );
  return oxm_match;
}


openflow_instructions *
pack_instruction( VALUE instruction ) {
  openflow_instructions *instructions = create_instructions();
  VALUE cInstruction;

  if ( instruction != Qnil ) {
    switch ( TYPE( instruction ) ) {
      case T_ARRAY: {
          VALUE *each = RARRAY_PTR( instruction );

          for ( int i = 0; i < RARRAY_LEN( instruction ); i++ ) {
            if ( rb_respond_to( each[ i ], rb_intern( "pack_instruction" ) ) ) {
              cInstruction = Data_Wrap_Struct( rb_obj_class( each[ i ] ), NULL, NULL, instructions );
              rb_funcall( each[ i ], rb_intern( "pack_instruction" ), 1, cInstruction );
            }
          }
        }
      break;
      case T_OBJECT:
        if ( rb_respond_to( rb_obj_class( instruction ), rb_intern( "pack_instruction" ) ) ) {
          cInstruction = Data_Wrap_Struct( instruction, NULL, NULL, instructions );
          rb_funcall( instruction, rb_intern( "pack_instruction" ), 1, cInstruction );
        }
      break;
      default:
        rb_raise( rb_eTypeError, "Instruction argument must be either an Array or an Instruction object" );
      break;
    }
  }
  Data_Get_Struct( cInstruction, openflow_instructions, instructions );
  printf("no.of instructions added %d\n", instructions->n_instructions );
  return instructions;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
