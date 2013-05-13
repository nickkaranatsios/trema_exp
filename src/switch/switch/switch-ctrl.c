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


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include "trema.h"
#include "ofdp.h"
#include "protocol.h"
#include "oxm-helper.h"
#include "parse-options.h"
#include "switch-ctrl.h"


int
prefixcmp( const char *str, const char *prefix ) {
  for ( ;; str++, prefix++ ) {
    if ( !*prefix ) {
      return 0;
    }
    else if ( *str != *prefix ) {
      return ( unsigned char ) *prefix - ( unsigned char ) *str;
    }
  }
}


int 
suffixcmp( const char *str, const char *suffix ) {
  size_t len = strlen( str ), suflen = strlen( suffix );
  if ( len < suflen ) {
    return -1;
  }
  else {
    return strcmp( str + len - suflen, suffix );
  }
}


uint32_t
parse_unsigned_int32( const char *option, uint32_t *val ) {
  char *end;
  const char *ptr = option;

  *val = ( uint32_t ) strtol( ptr, &end, 10 );
  if ( *end != '\0' || end == ptr ) {
    *val = 0;
  }
  return *val;
}


uint64_t
parse_unsigned_int64( const char *option, uint64_t *val ) {
  char *end;
  const char *ptr = option;

  *val = ( uint64_t ) strtoll( ptr, &end, 0 );
  if ( *end != '\0' || end == ptr ) {
    *val = 0;
  }
  return *val;
}


int
parse_option( const char *option, struct option *opts ) {
  char *end;

  int ret = 0;
  for ( ; opts->type != OPTION_END; opts++ ) {
    if ( !prefixcmp( option, opts->prefix_opt ) ) {
      char *mask_ptr = NULL;
      if ( OPT_MASK_SET( opts->flags ) ) {
        mask_ptr = strchr( option, '/' );
        *mask_ptr = '\0';
        mask_ptr = mask_ptr + 1;
      }
      char *ptr = strchr( option, '=' ) + 1;
      switch( opts->type ) {
        case OPTION_UINT8: {
          *( uint8_t * ) opts->value = ( uint8_t ) strtol( ptr, &end, 10 );
          OPT_SET( opts->flags );
        }
        break;
        case OPTION_UINT16: {
          *( uint16_t * ) opts->value = ( uint16_t ) strtol( ptr, &end, 10 );
          OPT_SET( opts->flags );
        }
        break;
        case OPTION_UINT32: {
          *( uint32_t * ) opts->value = ( uint32_t ) strtol( ptr, &end, 10 );
          OPT_SET( opts->flags );
        }
        break;
        case OPTION_UINT64: {
          if ( OPT_MASK_SET( opts->flags ) ) {
            *( uint64_t * ) opts->value = ( uint64_t ) strtoll( ptr, &end, 0 );
            *( ( uint64_t * ) opts->value + 1 ) = ( uint64_t ) strtoll( mask_ptr, &end, 0 );
           }
           else {
            *( uint64_t * ) opts->value = ( uint64_t ) strtoll( ptr, &end, 0 );
           }
           OPT_SET( opts->flags );
        }
        break;
        default:
          warn( "Unsupported option type %d ", opts->type );
          ret = -1;
        break;
      }
    }
  }

  return ret;
}


static void
parse_match( match *match, const struct option *opts ) {

#define MATCH_FIELDS( opts )                                       \
  MATCH_FIELD( in_port, opts, * ( uint32_t * ) opts->value, 0 )    \
  MATCH_FIELD( eth_type, opts, * ( uint16_t * ) opts->value, 0 )   \
  MATCH_FIELD( ip_proto, opts, * ( uint8_t * ) opts->value, 0 )    \
  MATCH_FIELD( udp_src, opts, * ( uint16_t * ) opts->value, 0 )    \
  MATCH_FIELD( udp_dst, opts, * ( uint16_t * ) opts->value, 0 )    \
  MATCH_FIELD( metadata, opts, * ( uint64_t * ) opts->value, * ( ( uint64_t * ) opts->value + 1 ) )


#define MATCH_FIELD( name, opts, value, mask )                     \
  do {                                                             \
    if ( !strcmp( #name, opts->prefix_opt + 2 ) ) {                \
      if ( OPT_MASK_SET( opts->flags ) ) {                         \
        MATCH_ATTR_MASK_SET( name, value, mask );                  \
      }                                                            \
      else {                                                       \
        MATCH_ATTR_SET( name, value );                             \
      }                                                            \
    }                                                              \
  }                                                                \
  while ( 0 );


  for ( ; opts->type != OPTION_END; opts++ ) {
    MATCH_FIELDS( opts );
  }
}


static char *
parse_dump_tables( char *cmd, const uint64_t own_datapath_id ) {
  if ( prefixcmp( cmd, "dump_tables" ) ) {
    return NULL;
  }
  char *opt = strchr ( cmd, ' ' );
  assert( opt );

  uint64_t datapath_val;
  uint8_t table_id_val = FLOW_TABLE_ID_MAX;

  struct option opts[] = {
    OPT_UINT64( "--datapath_id", "datapath_id:", OPT_NO_MASK, &datapath_val ),
    OPT_UINT8( "--table_id", "table_id:", OPT_NO_MASK, &table_id_val ),
    OPT_END()
  };


  char *saveptr;
  const char *sep = " ";

  for ( opt = strtok_r( opt, sep, &saveptr ); opt; opt = strtok_r( NULL, sep, &saveptr ) ) {
    parse_option( opt, opts );
    if ( *( uint64_t * ) opts[ 0 ].value != own_datapath_id ) {
      return NULL;
    }
  }

  if ( table_id_val != FLOW_TABLE_ID_MAX ) {
    flow_table_features table_features;
    get_flow_table_features( ( uint8_t ) table_id_val, &table_features );
    char *reply_buf = xmalloc( PATH_MAX + 1 );
    reply_buf[ 0 ] = '\0';
    const char *orig = "{ \"table_features\" [ ";
    const char *term = "] }";
      
    snprintf( reply_buf, PATH_MAX, "%s {" DUMP_TABLE_FIELDS "} %s",
              orig,
              table_features.table_id,
              table_features.name,
              table_features.config,
              table_features.max_entries,
              table_features.metadata_match,
              table_features.metadata_write,
              term );
    return reply_buf;
  }
  else {
    for ( uint32_t i = 0; i < table_id_val; i++ ) {
      flow_table_features table_features;
      get_flow_table_features( ( uint8_t ) i, &table_features );
    }
  }

  return NULL;
}


static char *
parse_dump_flows( char *cmd, const uint64_t own_datapath_id ) {
  if ( prefixcmp( cmd, "dump_flows" ) ) {
    return NULL;
  }
  char *opt = strchr ( cmd, ' ' );
  assert( opt );

  uint64_t datapath_val;
  uint32_t in_port_val;
  uint16_t eth_type_val;
  uint8_t ip_proto_val;
  uint64_t metadata_val[ 2 ];
  uint16_t udp_src_val;
  uint16_t udp_dst_val;
  uint8_t table_id_val;
  uint32_t out_port_val;
  uint32_t out_group_val;
  uint64_t cookie_val[ 2 ];


  struct option opts[] = {
    OPT_UINT64( "--datapath_id", "datapath_id:", OPT_NO_MASK, &datapath_val ),
    OPT_UINT32( "--in_port", "in_port:", OPT_NO_MASK, &in_port_val ),
    OPT_UINT16( "--eth_type", "eth_type:", OPT_NO_MASK, &eth_type_val ),
    OPT_UINT8( "--ip_proto", "ip_proto:", OPT_NO_MASK, &ip_proto_val ),
    OPT_UINT64( "--metadata", "metadata:", OPT_HAS_MASK, &metadata_val ),
    OPT_UINT16( "--udp_src", "udp_src:", OPT_NO_MASK, &udp_src_val ),
    OPT_UINT16( "--udp_dst", "udp_dst:", OPT_NO_MASK, &udp_dst_val ),
    OPT_UINT8( "--table_id", "table_id:", OPT_NO_MASK, &table_id_val ),
    OPT_UINT64( "--cookie", "cookie:", OPT_HAS_MASK, &cookie_val ),
    OPT_UINT32( "--out_port", "out_port:", OPT_NO_MASK, &out_port_val ),
    OPT_UINT32( "--out_group", "out_group:", OPT_NO_MASK, &out_group_val ),
    OPT_END()
  };

  
  char *saveptr;
  const char *sep = " ";

  for ( opt = strtok_r( opt, sep, &saveptr ); opt; opt = strtok_r( NULL, sep, &saveptr ) ) {
    parse_option( opt, opts );
    if ( *( uint64_t * ) opts[ 0 ].value != own_datapath_id ) {
      return NULL;
    }
  }
  match *match = create_match();
  parse_match( match, opts );

  flow_stats *stats = NULL;
  uint32_t nr_stats;
  OFDPE ret = get_flow_stats( ( uint8_t ) table_id_val, match, cookie_val[ 0 ], cookie_val[ 1 ], out_port_val, out_group_val, &stats, &nr_stats );
  if ( ret != OFDPE_SUCCESS ) {
    error( "Failed to retrieve flow stats from datapath ( ret = %d ).", ret );
  }
  delete_match( match );
  
  char *reply_buf = xmalloc( PATH_MAX + 1 );
  reply_buf[ 0 ] = '\0';
  size_t used = 0;
  int left;
  const char *term = ",";
  const char *orig = "{ \"dump_flows\": [";
  if ( stats != NULL && nr_stats ) {
    for ( uint32_t i = 0; i < nr_stats; i++ ) {
      if ( i != 0 ) {
        orig = "";
      }
      if ( i == nr_stats - 1 ) {
        term = "]}";
      }
      left = snprintf( reply_buf + used, PATH_MAX - used, "%s {" DUMP_FLOW_FIELDS "} %s",
                       orig,
                       stats->table_id,
                       stats->duration_sec,
                       stats->priority,
                       stats->idle_timeout,
                       stats->hard_timeout, 
                       stats->flags,
                       stats->cookie,
                       stats->packet_count,
                       stats->byte_count,
                       term );
      used = ( size_t ) left - used;
      stats++;
    }
  }

  return reply_buf;
}


static void
redis_reply( redisContext *context, char *reply_buf ) {
  if ( reply_buf != NULL ) {
    redisCommand( context, "SET %s %s", "cmd.reply", reply_buf );
    xfree( reply_buf );
  }
}


static void
process_request_cmd( redisContext *context, redisReply *reply, uint64_t own_datapath_id ) {
  char *cmd = reply->str;
  char *reply_buf;

  reply_buf = parse_dump_flows( cmd, own_datapath_id );
  redis_reply( context, reply_buf );
  reply_buf = parse_dump_tables( cmd, own_datapath_id );
  redis_reply( context, reply_buf );
}


static void
_check_ctrl( void *user_data ) {
  assert( user_data );
  struct protocol *protocol = user_data;

  redisContext *context = protocol->ctrl.redis_context;
  redisReply *reply = redisCommand( context, "GET cmd.request" );
  if ( reply != NULL ) {
    if ( reply->len ) {
      process_request_cmd( context, reply, protocol->args->datapath_id );
    }
    freeReplyObject( reply );
  }
}
void ( *check_ctrl )( void *user_data ) = _check_ctrl;


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
