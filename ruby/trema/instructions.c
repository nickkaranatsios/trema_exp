/*
 * Copyright (C) 2008-2013 NEC Corporation
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


#include "trema.h"
#include "ruby.h"
#include "action-common.h"


extern VALUE mTrema;
VALUE mInstructions;


static VALUE sym_table_id;
static VALUE sym_metadata;
static VALUE sym_metadata_mask;
static VALUE sym_actions;
static VALUE sym_meter;
static VALUE sym_experimenter;
static VALUE sym_user_data;


static openflow_instructions *
instructions_ptr( VALUE self ) {
  openflow_instructions *instructions;
  Data_Get_Struct( self, openflow_instructions, instructions );
  return instructions;
}


static VALUE
pack_goto_table_instruction( VALUE self, VALUE instructions_r, VALUE options ) {
  VALUE table_id = rb_hash_aref( options, sym_table_id );
  append_instructions_goto_table( instructions_ptr( instructions_r ), ( uint8_t ) NUM2UINT( table_id ) ); 
  return self;
}


static VALUE
pack_write_metadata_instruction( VALUE self, VALUE instructions_r, VALUE options ) {
  VALUE metadata = rb_hash_aref( options, sym_metadata );
  uint64_t metadata_mask = 0;
  VALUE metadata_mask_r = rb_hash_aref( options, sym_metadata_mask );

  if ( metadata_mask_r != Qnil ) {
    metadata_mask = ( uint64_t ) NUM2ULL( metadata_mask_r );
  }
  append_instructions_write_metadata( instructions_ptr( instructions_r ), ( uint64_t ) NUM2ULL( metadata ), metadata_mask );
  return self;
}


static VALUE
pack_write_action_instruction( VALUE self, VALUE instructions_r, VALUE options ) {
  VALUE basic_actions = rb_hash_aref( options, sym_actions );
  openflow_actions *actions = pack_basic_action( basic_actions );
  append_instructions_write_actions( instructions_ptr( instructions_r ), actions );
  return self;
}


static VALUE
pack_apply_action_instruction( VALUE self, VALUE instructions_r, VALUE options ) {
  VALUE action_list = rb_hash_aref( options, sym_actions );
  openflow_actions *actions = pack_basic_action( action_list );
  append_instructions_apply_actions( instructions_ptr( instructions_r ), actions );
  return self;
}


static VALUE
pack_clear_action_instruction( VALUE self, VALUE instruction_r, VALUE options ) {
  UNUSED( options );
  append_instructions_clear_actions( instructions_ptr( instruction_r ) );
  return self;
}


static VALUE
pack_meter_instruction( VALUE self, VALUE instructions_r, VALUE options ) {
  VALUE meter = rb_hash_aref( options, sym_meter );
  append_instructions_meter( instructions_ptr( instructions_r ), NUM2UINT( meter ) );
  return self;
}


static VALUE
pack_experimenter_instruction( VALUE self, VALUE instructions_r, VALUE options ) {
  VALUE experimenter = rb_hash_aref( options, sym_experimenter );
  VALUE user_data_r = Qnil;

  if ( ( user_data_r = rb_hash_aref( options, sym_user_data ) ) != Qnil ) {
    Check_Type( user_data_r, T_ARRAY );
    uint16_t length = ( uint16_t ) RARRAY_LEN( user_data_r );
    buffer *user_data = alloc_buffer_with_length( length );
    void *p = append_back_buffer( user_data, length );
    for ( int i = 0; i < length; i++ ) {
      ( ( uint8_t * ) p )[ i ] = ( uint8_t ) FIX2INT( RARRAY_PTR( user_data_r )[ i ] );
    }
    append_instructions_experimenter( instructions_ptr( instructions_r ), NUM2UINT( experimenter ), user_data );
    free_buffer( user_data );
  }
  else {
    append_instructions_experimenter( instructions_ptr( instructions_r ), NUM2UINT( experimenter ), NULL );
  }
  return self;
}


void
Init_instructions() {
  mInstructions = rb_define_module_under( mTrema, "Instructions" );

  sym_table_id = ID2SYM( rb_intern( "table_id" ) );
  sym_metadata = ID2SYM( rb_intern( "metadata" ) );
  sym_metadata_mask = ID2SYM( rb_intern( "metadata_mask" ) );
  sym_actions = ID2SYM( rb_intern( "actions" ) );
  sym_meter = ID2SYM( rb_intern( "meter" ) );
  sym_experimenter = ID2SYM( rb_intern( "experimenter" ) );
  sym_user_data = ID2SYM( rb_intern( "user_data" ) );

  rb_define_module_function( mInstructions, "pack_goto_table_instruction", pack_goto_table_instruction, 2 );
  rb_define_module_function( mInstructions, "pack_write_metadata_instruction", pack_write_metadata_instruction, 2 );
  rb_define_module_function( mInstructions, "pack_write_action_instruction", pack_write_action_instruction, 2 );
  rb_define_module_function( mInstructions, "pack_apply_action_instruction", pack_apply_action_instruction, 2 );
  rb_define_module_function( mInstructions, "pack_clear_action_instruction", pack_clear_action_instruction, 2 );
  rb_define_module_function( mInstructions, "pack_meter_instruction", pack_meter_instruction, 2 );
  rb_define_module_function( mInstructions, "pack_experimenter_instruction", pack_experimenter_instruction, 2 );
  rb_require( "trema/instructions" );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
