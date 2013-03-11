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


void
handle_flow_removed( uint64_t datapath_id,
                     uint32_t transaction_id,
                     uint64_t cookie,
                     uint16_t priority,
                     uint8_t reason,
                     uint8_t table_id,
                     uint32_t duration_sec,
                     uint32_t duration_nsec,
                     uint16_t idle_timeout,
                     uint16_t hard_timeout,
                     uint64_t packet_count,
                     uint64_t byte_count,
                     const oxm_matches *match,
                     void *controller ) {
  if ( !rb_respond_to( ( VALUE ) controller, rb_intern( "flow_removed" ) ) ) {
    return;
  }
  VALUE attributes = rb_hash_new();
  HASH_SET( attributes, "datapath_id", ULL2NUM( datapath_id ) );
  HASH_SET( attributes, "transaction_id", UINT2NUM( transaction_id ) );
  HASH_SET( attributes, "cookie", ULL2NUM( cookie ) );
  HASH_SET( attributes, "priority", UINT2NUM( priority ) );
  HASH_SET( attributes, "reason", UINT2NUM( reason ) );
  HASH_SET( attributes, "table_id", UINT2NUM( table_id ) );
  HASH_SET( attributes, "duration_sec", UINT2NUM( duration_sec ) );
  HASH_SET( attributes, "duration_nsec", UINT2NUM( duration_nsec ) );
  HASH_SET( attributes, "idle_timeout", UINT2NUM( idle_timeout ) );
  HASH_SET( attributes, "hard_timeout", UINT2NUM( hard_timeout ) );
  HASH_SET( attributes, "packet_count", ULL2NUM( packet_count ) );
  HASH_SET( attributes, "byte_count", ULL2NUM( byte_count ) );

  UNUSED( match );
#ifdef LATER
  if ( match != NULL ) {
printf( "no of matches %d\n", match->n_matches );
  }
  VALLUE r_match = oxm_match_to_r_match( match );
  HASH_SET( attributes, "match", r_match );
#endif
  VALUE cFlowRemoved = rb_funcall( rb_eval_string( "Messages::FlowRemoved" ), rb_intern( "new" ), 1, attributes );
  rb_funcall( ( VALUE ) controller, rb_intern( "flow_removed" ), 2, ULL2NUM( datapath_id ), cFlowRemoved );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
