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


#ifdef LATER
static VALUE r_table_stats_pool = Qnil;
#endif


static void
unpack_flow_multipart_reply( VALUE r_attributes, void *data ) {
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
printf( "ofp_match length = %u\n", flow_stats->match.length );
  VALUE r_match = ofp_match_to_r_match( &flow_stats->match );
  if ( r_match != Qnil ) {
    HASH_SET( r_attributes, "match", r_match );
  }
}


static void
unpack_desc_multipart_reply( VALUE r_attributes, void *data ) {
  assert( data );
  const struct ofp_desc *desc_stats = data;

  if ( strlen( desc_stats->mfr_desc ) ) {
    HASH_SET( r_attributes, "mfr_desc", rb_str_new2( desc_stats->mfr_desc ) );
  }
  if ( strlen( desc_stats->hw_desc ) ) {
    HASH_SET( r_attributes, "hw_desc", rb_str_new2( desc_stats->hw_desc ) );
  }
  if ( strlen( desc_stats->sw_desc ) ) {
    HASH_SET( r_attributes, "sw_desc", rb_str_new2( desc_stats->sw_desc ) );
  }
  if ( strlen( desc_stats->serial_num ) ) {
    HASH_SET( r_attributes, "serial_num", rb_str_new2( desc_stats->serial_num ) );
  }
  if ( strlen( desc_stats->dp_desc ) ) {
    HASH_SET( r_attributes, "dp_desc", rb_str_new2( desc_stats->dp_desc ) );
  }
} 


static void
unpack_aggregate_multipart_reply( VALUE r_attributes, void *data ) {
  assert( data );
  const struct ofp_aggregate_stats_reply *aggregate_stats = data;

  HASH_SET( r_attributes, "packet_count", ULL2NUM( aggregate_stats->packet_count ) );
  HASH_SET( r_attributes, "byte_count", ULL2NUM( aggregate_stats->byte_count ) );
  HASH_SET( r_attributes, "flow_count", UINT2NUM( aggregate_stats->flow_count ) );
}


static void
unpack_table_multipart_reply( VALUE r_attributes, void *data ) {
  assert( data );
  const struct ofp_table_stats *table_stats = data;
  
  HASH_SET( r_attributes, "table_id", UINT2NUM( table_stats->table_id ) );
  HASH_SET( r_attributes, "active_count", UINT2NUM( table_stats->active_count ) );
  HASH_SET( r_attributes, "lookup_count", ULL2NUM( table_stats->lookup_count ) );
  HASH_SET( r_attributes, "matched_count", ULL2NUM( table_stats->matched_count ) );
}


static void
unpack_port_multipart_reply( VALUE r_attributes, void *data ) {
  assert( data );
  const struct ofp_port_stats *port_stats = data;

  HASH_SET( r_attributes, "port_no", UINT2NUM( port_stats->port_no ) );
  HASH_SET( r_attributes, "rx_packets", ULL2NUM( port_stats->rx_packets ) );
  HASH_SET( r_attributes, "tx_packets", ULL2NUM( port_stats->tx_packets ) );
  HASH_SET( r_attributes, "rx_bytes", ULL2NUM( port_stats->rx_bytes ) );
  HASH_SET( r_attributes, "tx_bytes", ULL2NUM( port_stats->tx_bytes ) );
  HASH_SET( r_attributes, "rx_dropped", ULL2NUM( port_stats->rx_dropped ) );
  HASH_SET( r_attributes, "tx_dropped", ULL2NUM( port_stats->tx_dropped ) );
  HASH_SET( r_attributes, "rx_errors", ULL2NUM( port_stats->rx_errors ) );
  HASH_SET( r_attributes, "tx_errors", ULL2NUM( port_stats->tx_errors ) );
  HASH_SET( r_attributes, "rx_frame_err", ULL2NUM( port_stats->rx_frame_err ) );
  HASH_SET( r_attributes, "rx_over_err", ULL2NUM( port_stats->rx_over_err ) );
  HASH_SET( r_attributes, "rx_crc_err", ULL2NUM( port_stats->rx_crc_err ) );
  HASH_SET( r_attributes, "collisions", ULL2NUM( port_stats->collisions ) );
  HASH_SET( r_attributes, "duration_sec", UINT2NUM( port_stats->duration_sec ) );
  HASH_SET( r_attributes, "duration_nsec", UINT2NUM( port_stats->duration_nsec ) );
}


static VALUE
unpack_multipart_reply( void *controller, VALUE r_attributes, const uint16_t stats_type, const buffer *frame ) {
  VALUE sym_datapath_id = ID2SYM( rb_intern( "datapath_id" ) );
  VALUE r_dpid = rb_hash_aref( r_attributes, sym_datapath_id );
  VALUE r_reply_obj = Qnil;

  if ( frame != NULL ) {
    if ( frame->length ) {
      switch ( stats_type ) {
        case OFPMP_DESC: {
          unpack_desc_multipart_reply( r_attributes, frame->data );
          r_reply_obj = rb_funcall( rb_eval_string( "Messages::DescMultipartReply" ), rb_intern( "new" ), 1, r_attributes );
          if ( rb_respond_to( ( VALUE ) controller, rb_intern( "desc_multipart_reply"  ) ) ) {
            rb_funcall( ( VALUE ) controller, rb_intern( "desc_multipart_reply" ), 2, r_dpid, r_reply_obj );
          }
        }
        break;
        case OFPMP_FLOW: {
          unpack_flow_multipart_reply( r_attributes, frame->data );
          r_reply_obj = rb_funcall( rb_eval_string( "Messages::FlowMultipartReply" ), rb_intern( "new" ), 1, r_attributes );
          if ( rb_respond_to( ( VALUE ) controller, rb_intern( "flow_multipart_reply" ) ) ) {
            rb_funcall( ( VALUE ) controller, rb_intern( "flow_multipart_reply" ), 2, r_dpid, r_reply_obj );
          }
        }
        break;
        case OFPMP_AGGREGATE: {
          unpack_aggregate_multipart_reply( r_attributes, frame->data );
          r_reply_obj = rb_funcall( rb_eval_string( "Messages::AggregateMultipartReply" ), rb_intern( "new" ), 1, r_attributes );
          if ( rb_respond_to( ( VALUE ) controller, rb_intern( "aggregate_multipart_reply" ) ) ) {
            rb_funcall( ( VALUE ) controller, rb_intern( "aggregate_multipart_reply" ), 2, r_dpid, r_reply_obj ); 
          }
        }
        break;
        case OFPMP_TABLE: {
          uint16_t flags = 0;
          VALUE r_flags = HASH_REF( r_attributes, flags );
          if ( r_flags != Qnil ) {
            flags =  ( uint16_t ) NUM2UINT( r_flags );
          }
          UNUSED( flags );
          unpack_table_multipart_reply( r_attributes, frame->data );
          r_reply_obj = rb_funcall( rb_eval_string( "Messages::TableMultipartReply" ), rb_intern( "new" ), 1, r_attributes );
          if ( rb_respond_to( ( VALUE ) controller, rb_intern( "table_multipart_reply" ) ) ) {
            rb_funcall( ( VALUE ) controller, rb_intern( "table_multipart_reply" ), 2, r_dpid, r_reply_obj ); 
          }
#ifdef LATER
          else {
            if ( r_table_stats_pool == Qnil ) {
              r_table_stats_pool = rb_funcall( rb_eval_string( "TableStatsPool" ), rb_intern( "new" ), 0 );
              rb_funcall( r_table_stats_pool, rb_intern( "add" ), 1, r_reply_obj );
            }
          }
#endif
        }
        break;
        case OFPMP_PORT_STATS: {
          unpack_port_multipart_reply( r_attributes, frame->data );
          r_reply_obj = rb_funcall( rb_eval_string( "Messages::PortMultipartReply" ), rb_intern( "new" ), 1, r_attributes );
          if ( rb_respond_to( ( VALUE ) controller, rb_intern( "port_multipart_reply" ) ) ) {
            rb_funcall( ( VALUE ) controller, rb_intern( "port_multipart_reply" ), 2, r_dpid, r_reply_obj );
          }
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
  unpack_multipart_reply( controller, r_attributes, type, data );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
