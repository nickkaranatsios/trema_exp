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
VALUE cHello;


static VALUE
hello_alloc( VALUE klass ) {
  const uint32_t ofp_versions[] = { OFP_VERSION };
  buffer *hello = create_hello_elem_versionbitmap( 0, ofp_versions, sizeof( ofp_versions ) / sizeof( ofp_versions[ 0 ] ) );
  return Data_Wrap_Struct( klass, NULL, free_buffer, hello );
}


void
Init_hello() {
  cHello = rb_define_class_under( mTrema, "Hello", rb_cObject );
  rb_define_alloc_func( cHello, hello_alloc );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
