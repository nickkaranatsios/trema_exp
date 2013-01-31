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


extern VALUE mTrema;
VALUE mYieldMatch;


static VALUE sym_port_number;


static oxm_matches *
oxm_match_ptr( VALUE self ) {
  oxm_matches *match;
  Data_Get_Struct( self, oxm_matches, match );
  return match;
}


static VALUE
append_yield_match_in_port( VALUE self, VALUE oxm_match, VALUE options ) {
  VALUE port = rb_hash_aref( options, sym_port_number );
  append_oxm_match_in_port( oxm_match_ptr( oxm_match ), NUM2UINT( port ) );
  return self;
}


void
Init_yield_match() {
  mYieldMatch = rb_define_module_under( mTrema, "YieldMatch" );
  sym_port_number = ID2SYM( rb_intern( "port_number" ) );
  rb_define_module_function( mYieldMatch, "append_yield_match_in_port", append_yield_match_in_port, 2 );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
