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


static void
unpack_ofp_port( VALUE attributes, struct ofp_port *port_desc ) {
  HASH_SET( attributes, "port_no", UINT2NUM( port_desc->port_no ) );
  VALUE hw_addr = rb_funcall( rb_eval_string( "Trema::Mac" ), rb_intern( "new" ), 1, ULL2NUM( mac_to_uint64( port_desc->hw_addr ) ) );
  HASH_SET( attributes, "hw_addr", hw_addr );
  HASH_SET( attributes, "name", rb_str_new2( port_desc->name ) );
  HASH_SET( attributes, "config", UINT2NUM( port_desc->config ) );
  HASH_SET( attributes, "state", UINT2NUM( port_desc->state ) );
  HASH_SET( attributes, "curr", UINT2NUM( port_desc->curr ) );
  HASH_SET( attributes, "advertised", UINT2NUM( port_desc->advertised ) );
  HASH_SET( attributes, "supported", UINT2NUM( port_desc->supported ) );
  HASH_SET( attributes, "peer", UINT2NUM( port_desc->peer ) );
  HASH_SET( attributes, "curr_speed", UINT2NUM( port_desc->curr_speed ) );
  HASH_SET( attributes, "max_speed", UINT2NUM( port_desc->max_speed ) );
}


void
handle_port_status( uint64_t datapath_id,
  uint32_t transaction_id,
  uint8_t reason,
  struct ofp_port port_desc,
  void *controller ) {
  if ( !rb_respond_to( ( VALUE ) controller, rb_intern( "port_status" ) ) ) {
    return;
  }
  VALUE attributes = rb_hash_new();
  HASH_SET( attributes, "datapath_id", ULL2NUM( datapath_id ) );
  HASH_SET( attributes, "transaction_id", UINT2NUM( transaction_id ) );
  HASH_SET( attributes, "reason", UINT2NUM( reason ) );
  
  unpack_ofp_port( attributes, &port_desc );
  VALUE cPortStatus = rb_funcall( rb_eval_string( "Messages::PortStatus" ), rb_intern( "new" ), 1, attributes );
  rb_funcall( ( VALUE ) controller, rb_intern( "port_status" ), 1, cPortStatus );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
