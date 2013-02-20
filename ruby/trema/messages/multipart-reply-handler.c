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
#include "conversion-util.h"


void
handle_multipart_reply( uint64_t datapath_id,
                        uint32_t transaction_id,
                        uint16_t type,
                        uint16_t flags,
                        const buffer *data,
                        void *controller ) {
  
  if ( !rb_respond_to( ( VALUE ) controller, rb_intern( "error" ) ) ) {
    return;
  }
  
  VALUE attributes = rb_hash_new();
  HASH_SET( attributes, "datapath_id", ULL2NUM( datapath_id ) );
  HASH_SET( attributes, "transaction_id", UINT2NUM( transaction_id ) );
  HASH_SET( attributes, "type", UINT2NUM( type ) );
  HASH_SET( attributes, "flags", UINT2NUM( flags ) );
  if ( type == OFPMP_FLOW ) {
    if ( data != NULL ) {
      const struct ofp_flow_stats *flow_stats = data->data;
      printf( "length %u\n", flow_stats->length );
    }
  }
  UNUSED( data );

  VALUE cMultipartReply = rb_funcall( rb_eval_string( "Messages::MultipartReply" ), rb_intern( "new" ), 1, attributes );
  rb_funcall( ( VALUE ) controller, rb_intern( "multipart_reply" ), 2, ULL2NUM( datapath_id ), cMultipartReply );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
