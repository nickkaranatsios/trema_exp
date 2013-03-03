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
#include "hash-util.h"


buffer *
pack_port_multipart_request( VALUE options ) {
  uint32_t xid = get_transaction_id();
  VALUE r_xid = HASH_REF( options, transaction_id );
  if ( r_xid != Qnil ) {
    xid = NUM2UINT( r_xid );
  }

  uint16_t flags = 0;
  VALUE r_flags = HASH_REF( options, flags );
  if ( r_flags != Qnil ) {
    flags = ( uint16_t ) NUM2UINT( r_flags );
  }

  uint32_t port_no = OFPP_ANY;
  VALUE r_port_no = HASH_REF( options, port_no );
  if ( r_port_no != Qnil ) {
    port_no = NUM2UINT( r_port_no );
  }
printf("port no %u\n", port_no );
  buffer *port_multipart_request = create_port_multipart_request( xid, flags, port_no );
  return port_multipart_request;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */

  