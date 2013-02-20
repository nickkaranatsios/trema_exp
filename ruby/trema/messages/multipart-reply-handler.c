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


#include <assert.h>
#include "trema.h"
#include "ruby.h"
#include "hash-util.h"
#include "conversion-util.h"


static void
unpack_flow_stats_reply( VALUE r_attributes, void *data ) {
  assert( data );
  const struct ofp_flow_stats *flow_stats = data;

  HASH_SET( r_attributes, "length", UINT2NUM( flow_stats->length ) );
  HASH_SET( r_attributes, "table_id", UINT2NUM( flow_stats->table_id ) );
  HASH_SET( r_attributes, "duration_sec", UINT2NUM( flow_stats->duration_sec ) );
  HASH_SET( r_attributes, "duration_nsec", UINT2NUM( flow_stats->duration_nsec ) );
  HASH_SET( r_attributes, "priority", UINT2NUM( flow_stats->priority ) );
  HASH_SET( r_attributes, "idle_timeout", UINT2NUM( flow_stats->idle_timeout ) );
  HASH_SET( r_attributes, "hard_timeout", UINT2NUM( flow_stats->hard_timeout ) );
  HASH_SET( r_attributes, "flags", UINT2NUM( flow_stats->flags ) );
  HASH_SET( r_attributes, "cookie", ULL2NUM( flow_stats->cookie ) );
  HASH_SET( r_attributes, "packet_count", ULL2NUM( flow_stats->packet_count ) );
  HASH_SET( r_attributes, "byte_count", ULL2NUM( flow_stats->byte_count ) );
}


static VALUE
unpack_multipart_reply( VALUE r_attributes, const uint16_t stats_type, const buffer *frame ) {
  VALUE r_reply_obj = Qnil;

  if ( frame != NULL ) {
    if ( frame->length ) {
      switch ( stats_type ) {
        case OFPMP_FLOW: {
          unpack_flow_stats_reply( r_attributes, frame->data );
printf("we are here\n");
          r_reply_obj = rb_funcall( rb_eval_string( "Messages::FlowStatsReply" ), rb_intern( "new" ), 1, r_attributes );
        }
        break;
        default:
          warn( "Received invalid mutlipart reply type %u", stats_type );
        break;
      }
    }
  }
  return r_reply_obj;
}


static void
dispatch_reply( VALUE r_reply_obj, VALUE controller, VALUE dpid ) {
  if ( r_reply_obj != Qnil ) {
    if ( rb_obj_is_instance_of( r_reply_obj, rb_eval_string( "Messsages::FlowStatsReply" ) ) ) {
      rb_funcall( controller, rb_intern( "flow_stats_reply" ), 2, ULL2NUM( dpid ), r_reply_obj );
    }
  }
}
    

void
handle_multipart_reply( uint64_t datapath_id,
                        uint32_t transaction_id,
                        uint16_t type,
                        uint16_t flags,
                        const buffer *data,
                        void *controller ) {
  VALUE r_attributes = rb_hash_new();
  VALUE r_dpid = ULL2NUM( datapath_id );
  HASH_SET( r_attributes, "datapath_id", r_dpid );
  HASH_SET( r_attributes, "transaction_id", UINT2NUM( transaction_id ) );
  HASH_SET( r_attributes, "type", UINT2NUM( type ) );
  HASH_SET( r_attributes, "flags", UINT2NUM( flags ) );
  VALUE r_reply_obj = unpack_multipart_reply( r_attributes, type, data );
  dispatch_reply( r_reply_obj, ( VALUE ) controller, r_dpid );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
