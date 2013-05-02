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


#define MAX_TOKENS 10
#define TOKEN_SIZE 32


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
parse_unsigned_int32( const char *option, const size_t option_len, uint32_t *val ) {
  char *end;
  const char *ptr = option + option_len;

  *val = ( uint32_t ) strtol( ptr, &end, 10 );
  if ( *end != '\0' || end == ptr ) {
    *val = 0;
  }
  return *val;
}


uint64_t
parse_unsigned_int64( const char *option, const size_t option_len, uint64_t *val ) {
  char *end;
  const char *ptr = option + option_len;

  *val = ( uint64_t ) strtoll( ptr, &end, 10 );
  if ( *end != '\0' || end == ptr ) {
    *val = 0;
  }
  return *val;
}


static void
parse_match( match *match, const char *option ) {
  if ( !prefixcmp( option, "--in_port=" ) ) {
    uint32_t in_port_val;
    parse_unsigned_int32( option, strlen( "--in_port=" ), &in_port_val );
    MATCH_ATTR_SET( in_port, in_port_val ); 
  }
  if ( !prefixcmp( option, "--eth_type=" ) ) {
    uint32_t eth_type_val;
    parse_unsigned_int32( option, strlen( "--eth_type=" ), &eth_type_val );
    MATCH_ATTR_SET( eth_type, ( uint16_t ) eth_type_val );
  }
  if ( !prefixcmp( option, "--metadata=" ) ) {
    char *mask_option = NULL;
    uint64_t metadata_mask_val = 0;
    if ( ( mask_option = strchr( option, '/' ) ) != NULL ) {
      parse_unsigned_int64( mask_option, strlen( "/" ), &metadata_mask_val );  
    }
    uint64_t metadata_val;
    if ( mask_option ) {
      char temp[ 64 ]; 
      memset( temp, 0, sizeof( temp ) );
      const char *ptr = option + strlen( "--metadata=" );
      memcpy( temp, ptr, ( size_t ) ( mask_option - ptr ) );
      parse_unsigned_int64( temp, 0, &metadata_val );
      MATCH_ATTR_MASK_SET( metadata, metadata_val, metadata_mask_val );
    }
    else {
      MATCH_ATTR_SET( metadata, metadata_val );
    }
  }
  if ( !prefixcmp( option, "--ip_proto=" ) ) {
    uint32_t ip_proto_val;
    parse_unsigned_int32( option, strlen( "--ip_proto=" ), &ip_proto_val );
    MATCH_ATTR_SET( ip_proto, ( uint8_t ) ip_proto_val );
  }
  if ( !prefixcmp( option, "--udp_src=" ) ) {
    uint32_t udp_src_val;
    parse_unsigned_int32( option, strlen( "--udp_src=" ), &udp_src_val ); 
    MATCH_ATTR_SET( udp_src, ( uint16_t ) udp_src_val );
  }
  if ( !prefixcmp( option, "--udp_dst=" ) ) {
    uint32_t udp_dst_val;
    parse_unsigned_int32( option, strlen( "--udp_dst=" ), &udp_dst_val );
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
    const char *option = ( cmd + ( i++ * TOKEN_SIZE ) );

    parse_match( match, option );

    if ( !prefixcmp( option, "--table_id=" ) ) {
      parse_unsigned_int32( option, strlen( "--table_id=" ), &table_id );
    }

    if ( !prefixcmp( option, "--cookie=" ) ) {
      parse_unsigned_int64( option, strlen( "--cookie=" ), &cookie );
    }

    if ( !prefixcmp( option, "--cookie_mask=" ) ) {
      parse_unsigned_int64( option, strlen( "--cookie_mask=" ), &cookie_mask );
    }

    if ( !prefixcmp( option, "--out_port=" ) ) {
      parse_unsigned_int32( option, strlen( "--out_port=" ), &out_port );
    }

    if ( !prefixcmp( option, "--out_group=" ) ) {
      parse_unsigned_int32( option, strlen( "--out_group=" ), &out_group );
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
  if ( stats != NULL && nr_stats ) {
    for ( uint32_t i = 0; i < nr_stats; i++ ) {
      left = snprintf( reply_buf + used, PATH_MAX - used, "dump_flows=table_id=%u,duration_sec=%u,duration_nsec=%u,priority=%u,idle_timeout=%u,hard_timeout=%u,cookie=%" PRIu64",packet_count=%" PRIu64",byte_count=%" PRIu64"\n",
                       stats->table_id,
                       stats->duration_sec,
                       stats->duration_nsec,
                       stats->priority,
                       stats->idle_timeout,
                       stats->hard_timeout, 
                       stats->cookie,
                       stats->packet_count,
                       stats->byte_count );
      used = ( size_t ) left - used;
      stats++;
    }
  }
  return reply_buf;
}


static void
process_request_cmd( redisContext *context, redisReply *reply ) {
  char *token;
  char *saveptr;
  const char *sep = " ";
  char tokens[ MAX_TOKENS * TOKEN_SIZE ];
  char *cmd = reply->str;

  uint8_t i = 0;
  // dump_flows 0xabc --in_port=1
  for ( token =  strtok_r( cmd, sep, &saveptr ); token; token = strtok_r( NULL, sep, &saveptr ) ) {
    if ( i < MAX_TOKENS ) {
      strncpy(  ( tokens + ( i++ * TOKEN_SIZE ) ), token, TOKEN_SIZE );
    }
  }
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


static void
_check_ctrl( void *user_data ) {
  assert( user_data );
  struct protocol *protocol = user_data;

  redisContext *context = protocol->ctrl.redis_context;
  redisReply *reply = redisCommand( context, "GET cmd.request" );
  if ( reply != NULL ) {
    if ( reply->len ) {
error( "about to process command" );
      process_request_cmd( context, reply );
      warn( "cmd %s", reply->str );
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
