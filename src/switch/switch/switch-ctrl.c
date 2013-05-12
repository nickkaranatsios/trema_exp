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




enum option_type {
  OPTION_END,
  OPTION_UINT8,
  OPTION_UINT16,
  OPTION_UINT32,
  OPTION_UINT64,
  OPTION_STRING
};

#define OPT_NO_MASK( ( 0 << 0 )
#define OPT_HAS_MASK ( 1 << 0 )
#define OPT_VALUE_SET ( 1 << 1 )

struct option {
  const char *prefix_opt;
  const char *json_output;
  int flags;
  void *value;
  enum option_type type;
};
  
#define OPT_END() { OPTION_END }
#define OPT_UINT8 { ( p ), ( j ), ( f ), ( v ), OPTION_UINT8 }
#define OPT_UINT16 { ( p ), ( j ), ( f ), ( v ), OPTION_UINT16 }
#define OPT_UINT32 { ( p ), ( j ), ( f ), ( v ), OPTION_UINT32 }
#define OPT_UINT64 { ( p ), ( j ), ( f ), ( v ), OPTION_UIN64 }
#define OPT_STRING { ( p ), ( j ), ( f ), ( v ), OPTION_STRING }
#define OPT_SET( f ) ( ( f ) |= OPT_VALUE_SET )
#define OPT_MASK_SET( f ) ( ( f ) & OPT_HAS_MASK == OPT_HAS_MASK )

  

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
parse_option( const char *option, const struct option *opts ) {
  char *end;

  for ( ; opts->type != OPTION_END; opts++ ) {
    if ( !prefixcmp( option, opts->prefix_opt ) ) {
      if ( HAS_MASK( opts->flags ) ) {
        
      }
      char *ptr = strchr( option, "=" ) + 1;
      switch( opts->type ) {
        case OPTION_UINT8: {
          *( uint8_t * ) opt->value = ( uint8_t * ) strtol( ptr, &end, 10 );
          OPT_SET( opts->flags );
        }
        break;
        case OPTION_UINT16: {
          *( uint16_t * ) opt->value = ( uint16_t * ) strtol( ptr, &end, 10 );
          OPT_SET( opts->flags );
        }
        break;
        case OPTION_UINT32: {
          *( uint32_t * ) opt->value = strtol( ptr, &end, 10 );
          OPT_SET( opts->flags );
        }
        break;
        case OPTION_UINT64: {
         *( uint64_t * ) = ( uint64_t ) strtoll( ptr, &end, 0 );
         OPT_SET( opts->flags );
        }
        break;
        default:
          warn( "Unsupported option type %d ", opts->type );
        break;
    }
  }

  return 1;
}


static void
parse_match( match *match, const char *option ) {
  uint16_t udp_src_val;
  uint16_t udp_dst_val;
  uint32_t in_port_val;
  uint32_t eth_type_val;
  uint64_t metadata_val[ 2 ];

  struct option opts[] = {
    OPT_UINT32( "--in_port", "in_port:", OPT_NO_MASK, &in_port_val ),
    OPT_UINT16( "--eth_type", "eth_type:", OPT_NO_MASK, &eth_type_val );
    OPT_UINT64( "--metadata", "metadata:", OPT_HAS_MASK, &metadata_val );
    OPT_UINT8( "--ip_proto", "ip_proto:", OPT_NO_MASK, &ip_proto_val );
    OPT_UINT16( "--udp_src", "udp_src:", OPT_NO_MASK, &udp_src_val );
    OPT_UINT16( "--udp_dst", "udp_dst:", OPT_NO_MASK, &udp_dst_val );
    OPT_END()
  };
    
  int argc = parse_option( option, opts );
    
  for ( ; opts->type != OPTION_END; opts++ ) {
    if ( OPT_SET( opt->flags ) ) {
      MATCH_ATTR_SET( opts->prefix_opt + 2, opts->value );
    }
  if ( !prefixcmp( option, "--in_port" ) ) {
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
      strcpy( arg->json_buf, "table_id:" );
      strcat( arg->json_buf, ptr );
    }

    if ( !prefixcmp( option, "--cookie=" ) ) {
      const char *ptr = strchr( option, '=' ) + 1;
      parse_unsigned_int64( ptr, &cookie );
      strcpy( arg->json_buf, "cookie:" );
      strcat( arg->json_buf, ptr );
    }

    if ( !prefixcmp( option, "--cookie_mask=" ) ) {
      const char *ptr = strchr( option, '=' ) + 1;
      parse_unsigned_int64( ptr, &cookie_mask );
      strcpy( arg->json_buf, "cookie_mask:" );
      strcat( arg->json_buf, ptr );
    }

    if ( !prefixcmp( option, "--out_port=" ) ) {
      const char *ptr = strchr( option, '=' ) + 1;
      parse_unsigned_int32( ptr, &out_port );
      strcpy( arg->json_buf, "out_port:" );
      strcat( arg->json_buf, ptr );
    }

    if ( !prefixcmp( option, "--out_group=" ) ) {
      const char *ptr = strchr( option, '=' ) + 1;
      parse_unsigned_int32( ptr, &out_group );
      strcpy( arg->json_buf, "out_group:" );
      strcat( arg->json_buf, ptr );
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


static char *
dump_tables( const char *cmd, uint8_t cmd_len ) {
  uint32_t table_id = FLOW_TABLE_ID_MAX;;
  uint8_t i = 1;

  while ( cmd_len > 1 ) {
    struct arg *arg = &args[ i ];
    const char *option = ( cmd + ( i++ * TOKEN_SIZE ) );

    if ( !prefixcmp( option, "--table_id=" ) ) {
      const char *ptr = strchr( option, '=' ) + 1;
      parse_unsigned_int32( ptr, &table_id );
    }
    if ( table_id != FLOW_TABLE_ID_MAX ) {
      flow_table_features table_features;
      get_flow_table_features( ( uint8_t ) table_id, &table_features );
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
      for ( uint32_t i = 0; i < table_id; i++ ) {
        flow_table_features table_features;
        get_flow_table_features( ( uint8_t ) i, &table_features );
      }
    }
    cmd_len--;
  }
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
parse_dump_flows( const char *option, uint64_t own_datapath_id ) {
  uint64_t datapath_val;
  uint64_t metadata_val[ 2 ];
  uint64_t cookie_val[ 2 ];
  uint32_t in_port_val;
  uint32_t eth_type_val;
  uint32_t out_port_val;
  uint32_t out_group_val;
  uint16_t udp_src_val;
  uint16_t udp_dst_val;
  uint8_t ip_proto_val;
  uint8_t table_id_val;

static struct option dump_flows_opts[] = {
    OPT_UINT64( "--datapath_id", "datapath_id:", OPT_NO_MASK, &datapath_val ),
    OPT_UINT32( "--in_port", "in_port:", OPT_NO_MASK, &in_port_val ),
    OPT_UINT16( "--eth_type", "eth_type:", OPT_NO_MASK, &eth_type_val ),
    OPT_UINT64( "--metadata", "metadata:", OPT_HAS_MASK, &metadata_val ),
    OPT_UINT8( "--ip_proto", "ip_proto:", OPT_NO_MASK, &ip_proto_val ),
    OPT_UINT16( "--udp_src", "udp_src:", OPT_NO_MASK, &udp_src_val ),
    OPT_UINT16( "--udp_dst", "udp_dst:", OPT_NO_MASK, &udp_dst_val ),
    OPT_UINT8( "--table_id", "table_id:", OPT_NO_MASK, &table_id_val ),
    OPT_UINT64( "--cookie", "cookie:", OPT_HAS_MASK, &cookie_val ),
    OPT_UINT32( "--out_port", "out_port:" OPT_NO_MASK, &out_port_val ),
    OPT_UINT32( "--out_group", "out_group:" OPT_NO_MASK, &out_group_val ),
    OPT_END()
  };
  
  parse_option( option, dump_flow_opts );
  if ( OPT_SET( dump_flows_opts->opts[ 0 ].flags ) ) {
    if ( own_datapath_id != * ( uint64_t * ) dump_flows_opts[ 0 ].value ) {
      return;
    }
  }
}

static void
process_request_cmd( redisContext *context, redisReply *reply, uint64_t own_datapath_id ) {
  char *option;
  char *saveptr;
  const char *sep = " ";
  char *cmd = reply->str;

  // dump_flows --datapath_id=0xabc --in_port=1
  for ( option =  strtok_r( cmd, sep, &saveptr ); option; option = strtok_r( NULL, sep, &saveptr ) ) {
    if ( !prefixcmp( option, "dump_flows" ) ) {
      parse_dump_flows( option );
    }
    else if ( !prefixcmp( option, "dump_tables" ) ) {
      parse_dump_tables( option );
    }
    else {
      warn( "Unknown command %s", token );
    }
  }

#ifdef 0
  const char *dpid_str = ( tokens + TOKEN_SIZE );
  if ( parse_dpid( dpid_str, own_datapath_id ) ) {
    const char *input_cmd = &tokens[ 0 ];
    if ( !strncmp( input_cmd, "dump_flows", TOKEN_SIZE ) ) {
      char *reply_buf = dump_flows( tokens, i );
      redisCommand( context, "SET %s %s", "cmd.reply", reply_buf );
      xfree( reply_buf );
    }
    else if ( !strncmp( input_cmd, "dump_tables", TOKEN_SIZE ) ) {
      char *reply_buf = dump_tables( tokens, i );
      redisCommand( context, "SET %s %s", "cmd.reply", reply_buf );
      xfree( reply_buf );
    }
    else {
      warn( "Unknown command %s", tokens );
    }
  }
#endif
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
