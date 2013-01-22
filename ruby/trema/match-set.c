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


#include "trema.h"
#include "ruby.h"


extern VALUE mTrema;
VALUE mMatchSet;


static openflow_actions *
openflow_actions_ptr( VALUE self ) {
  openflow_actions *actions;
  Data_Get_Struct( self, openflow_actions, actions );
  return actions;
}


static VALUE
append_match_in_port( VALUE self, VALUE r_actions, VALUE in_port ) {
  append_action_set_field_in_port( openflow_actions_ptr( r_actions ), NUM2UINT( in_port ) );
  return self;
}


static VALUE
append_match_in_phy_port( VALUE self, VALUE r_actions, VALUE in_phy_port ) {
  append_action_set_field_in_phy_port( openflow_actions_ptr( r_actions ), NUM2UINT( in_phy_port ) );
  return self;
}


void
Init_match_set() {
  mMatchSet = rb_define_module_under( mTrema, "MatchSet" );
  rb_define_module_function( mMatchSet, "append_match_in_port", append_match_in_port, 2 );
  rb_define_module_function( mMatchSet, "append_match_in_phy_port", append_match_in_phy_port, 2 );
  rb_require( "trema/match-in-port" );
  rb_require( "trema/match-in-phy-port" );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
