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




struct arg {
  char token[ TOKEN_SIZE ];
  char json_format[ TOKEN_SIZE ];
  union {
    uint8_t uint8_val;
    uint16_t uint16_val;
    uint32_t uint32_val;
    uint64_t uint64_val;
  } value;
};

static struct arg args[ MAX_TOKENS ];


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


static void
parse_match( match *match, const char *option ) {
  if ( !prefixcmp( option, "--in_port=" ) ) {
    uint32_t in_port_val;
    parse_unsigned_int32( strchr( option, '=' ) + 1, &in_port_val );
    MATCH_ATTR_SET( in_port, in_port_val ); 
  }
  if ( !prefixcmp( option, "--eth_type=" ) ) {
    uint32_t eth_type_val;
    parse_unsigned_int32( strchr( option, '=' ) + 1, &eth_type_val );
    MATCH_ATTR_SET( eth_type, ( uint16_t ) eth_type_val );
  }
  if ( !prefixcmp( option, "--metadata=" ) ) {
    char *mask_option = NULL;
    uint64_t metadata_mask_val = 0;
    if ( ( mask_option = strchr( option, '/' ) ) != NULL ) {
      parse_unsigned_int64( mask_option + 1, &metadata_mask_val );  
    }
    uint64_t metadata_val;
    if ( mask_option ) {
      char temp[ 64 ]; 
      memset( temp, 0, sizeof( temp ) );
      const char *ptr = option + strlen( "--metadata=" );
      memcpy( temp, ptr, ( size_t ) ( mask_option - ptr ) );
      parse_unsigned_int64( temp, &metadata_val );
      MATCH_ATTR_MASK_SET( metadata, metadata_val, metadata_mask_val );
    }
    else {
      MATCH_ATTR_SET( metadata, metadata_val );
    }
  }
  if ( !prefixcmp( option, "--ip_proto=" ) ) {
    uint32_t ip_proto_val;
    parse_unsigned_int32( strchr( option, '=' ) + 1, &ip_proto_val );
    MATCH_ATTR_SET( ip_proto, ( uint8_t ) ip_proto_val );
  }
  if ( !prefixcmp( option, "--udp_src=" ) ) {
    uint32_t udp_src_val;
    parse_unsigned_int32( strchr( option , '=' ) + 1, &udp_src_val ); 
    MATCH_ATTR_SET( udp_src, ( uint16_t ) udp_src_val );
  }
  if ( !prefixcmp( option, "--udp_dst=" ) ) {
    uint32_t udp_dst_val;
    parse_unsigned_int32( strchr( option, '=' ) + 1, &udp_dst_val );
    MATCH_ATTR_SET( udp_dst, ( uint16_t ) udp_dst_val );
  }
}


static char *
dump_flows( const char *cmd, uint8_t cmd_len ) {
  uint8_t i = 1;

  match *match = create_match();

  uint32_t table_id = OFPTT_ALL;
  uint64_t cookie = 0;
  uint64_t cookie_mask = 0;
  uint32_t out_port = OFPP_ANY;
  uint32_t out_group = OFPG_ANY;

  while ( cmd_len > 1 ) {
    struct arg *arg = &args[ i ];
    const char *option = ( cmd + ( i++ * TOKEN_SIZE ) );

    parse_match( match, option );

    if ( !prefixcmp( option, "--table_id=" ) ) {
      const char *ptr = strchr( option, '=' ) + 1;
      parse_unsigned_int32( ptr, &table_id );
      arg->value.uint32_val = table_id;
      // skip the --
      strcpy( arg->json_format, "table_id:" );
      strcat( arg->json_format, ptr );
    }

    if ( !prefixcmp( option, "--cookie=" ) ) {
      const char *ptr = strchr( option, '=' ) + 1;
      parse_unsigned_int64( ptr, &cookie );
      strcpy( arg->json_format, "cookie:" );
      strcat( arg->json_format, ptr );
    }

    if ( !prefixcmp( option, "--cookie_mask=" ) ) {
      const char *ptr = strchr( option, '=' ) + 1;
      parse_unsigned_int64( ptr, &cookie_mask );
      strcpy( arg->json_format, "cookie_mask:" );
      strcat( arg->json_format, ptr );
    }

    if ( !prefixcmp( option, "--out_port=" ) ) {
      const char *ptr = strchr( option, '=' ) + 1;
      parse_unsigned_int32( ptr, &out_port );
      strcpy( arg->json_format, "out_port:" );
      strcat( arg->json_format, ptr );
    }

    if ( !prefixcmp( option, "--out_group=" ) ) {
      const char *ptr = strchr( option, '=' ) + 1;
      parse_unsigned_int32( ptr, &out_group );
      strcpy( arg->json_format, "out_group:" );
      strcat( arg->json_format, ptr );
    }
    cmd_len--;
  }
  
  flow_stats *stats = NULL;
  uint32_t nr_stats;
  OFDPE ret = get_flow_stats( ( uint8_t ) table_id, match, cookie, cookie_mask, out_port, out_group, &stats, &nr_stats );
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


static int
parse_dpid( const char *dpid_str, uint64_t own_datapath_id ) {
  if ( !prefixcmp( dpid_str, "--datapath_id=" ) ) {
    uint64_t dpid;
    parse_unsigned_int64( strchr( dpid_str, '=' ) + 1, &dpid );
    return own_datapath_id == dpid;
  }

  return -1;
}


static void
process_request_cmd( redisContext *context, redisReply *reply, uint64_t own_datapath_id ) {
  char *token;
  char *saveptr;
  const char *sep = " ";
  char tokens[ MAX_TOKENS * TOKEN_SIZE ];
  char *cmd = reply->str;

  uint8_t i = 0;
  // dump_flows 0xabc --in_port=1
  for ( token =  strtok_r( cmd, sep, &saveptr ); token; token = strtok_r( NULL, sep, &saveptr ) ) {
    if ( i < MAX_TOKENS ) {
      strncpy( args[ i ].token, token, TOKEN_SIZE );
      strncpy(  ( tokens + ( i++ * TOKEN_SIZE ) ), token, TOKEN_SIZE );
    }
  }
  const char *dpid_str = ( tokens + TOKEN_SIZE );
  if ( parse_dpid( dpid_str, own_datapath_id ) ) {
    const char *input_cmd = &tokens[ 0 ];
    if ( !strncmp( input_cmd, "dump_flows", TOKEN_SIZE ) ) {
      char *reply_buf = dump_flows( tokens, i );
      redisCommand( context, "SET %s %s", "cmd.reply", reply_buf );
      xfree( reply_buf );
    }
    else {
      warn( "Unknown command %s", tokens );
    }
  }
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
