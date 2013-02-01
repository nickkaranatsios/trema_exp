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
VALUE mFlexibleActions;


static VALUE sym_port_number;


static oxm_matches *
oxm_match_ptr( VALUE self ) {
  oxm_matches *match;
  Data_Get_Struct( self, oxm_matches, match );
  return match;
}


static VALUE
append_flexible_action_in_port( VALUE self, VALUE oxm_match, VALUE in_port ) {
  append_oxm_match_in_port( oxm_match_ptr( oxm_match ), NUM2UINT( in_port ) );
  return self;
}


static VALUE
append_flexible_action_in_phy_port( VALUE self, VALUE oxm_match, VALUE in_phy_port ) {
  append_oxm_match_in_phy_port( oxm_match_ptr( oxm_match ), NUM2UINT( in_phy_port ) );
  return self;
}


static VALUE
append_flexible_action_metadata( VALUE self, VALUE oxm_match, VALUE metadata, VALUE metadata_mask ) {
  append_oxm_match_metadata( oxm_match_ptr( oxm_match ), rb_num2ull( metadata ), rb_num2ull( metadata_mask ) );
  return self;
}


static VALUE
append_flexible_action_eth_src( VALUE self, VALUE oxm_match, VALUE mac_address ) {
  uint8_t mask_zero[ OFP_ETH_ALEN ] = { 0 };
  append_oxm_match_eth_dst( oxm_match_ptr( oxm_match ), mac_addr_to_cstr( mac_address ), mask_zero );
  return self;
}


void
Init_flexible_actions() {
  mFlexibleActions = rb_define_module_under( mTrema, "FlexibleActions" );
  sym_port_number = ID2SYM( rb_intern( "port_number" ) );
  rb_define_module_function( mFlexibleActions, "append_flexible_action_in_port", append_flexible_action_in_port, 2 );
  rb_define_module_function( mFlexibleActions, "append_flexible_action_in_phy_port", append_flexible_action_in_phy_port, 2 );
  rb_define_module_function( mFlexibleActions, "append_flexible_action_metadata", append_flexible_action_metadata, 2 );
  rb_define_module_function( mFlexibleActions, "append_flexible_action_eth_src", append_flexible_action_eth_src, 2 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
