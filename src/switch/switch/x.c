# gcc -g -o x x.c
# gbd x
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>

// maximum number of options a command would have
#define MAX_TOKENS 20
// size of each option
#define TOKEN_SIZE 32

enum option_type {
  OPTION_END,
  OPTION_UINT8,
  OPTION_UINT16,
  OPTION_UINT32,
  OPTION_UINT64,
  OPTION_STRING
};

#define OPT_NO_MASK ( 0 << 0 )
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
#define OPT_UINT8( p, j, f, v ) { ( p ), ( j ), ( f ), ( v ), OPTION_UINT8 }
#define OPT_UINT16( p, j, f, v ) { ( p ), ( j ), ( f ), ( v ), OPTION_UINT16 }
#define OPT_UINT32( p, j, f, v ) { ( p ), ( j ), ( f ), ( v ), OPTION_UINT32 }
#define OPT_UINT64( p, j, f, v ) { ( p ), ( j ), ( f ), ( v ), OPTION_UINT64 }
#define OPT_STRING( p, j, f, v ) { ( p ), ( j ), ( f ), ( v ), OPTION_STRING }
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
          *( uint8_t * ) opts->value = strtol( ptr, &end, 10 );
          OPT_SET( opts->flags );
        }
        break;
        case OPTION_UINT16: {
          *( uint16_t * ) opts->value = strtol( ptr, &end, 10 );
          OPT_SET( opts->flags );
        }
        break;
        case OPTION_UINT32: {
          *( uint32_t * ) opts->value = strtol( ptr, &end, 10 );
          OPT_SET( opts->flags );
        }
        break;
        case OPTION_UINT64: {
          if ( OPT_MASK_SET( opts->flags ) ) {
            *( uint64_t * ) opts->value = strtoll( ptr, &end, 0 );
            *( ( uint64_t * ) opts->value + 1 ) = strtoll( mask_ptr, &end, 0 );
           }
           else {
            *( uint64_t * ) opts->value = strtoll( ptr, &end, 0 );
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
parse_dump_flows( char *cmd, const uint64_t datapath_id ) {
  if ( prefixcmp( cmd, "dump_flows" ) ) {
    return;
  }
  char *opt = strchr ( cmd, ' ' );
  assert( opt );

  uint32_t in_port_val;
  uint32_t eth_type_val;
  uint64_t datapath_val = 0;
  uint64_t metadata_val[ 2 ];
  uint8_t ip_proto_val;
  uint16_t udp_src_val;
  uint16_t udp_dst_val;

  struct option opts[] = {
    OPT_UINT64( "--datapath_id", "datapath_id:", OPT_NO_MASK, &datapath_val ),
    OPT_UINT32( "--in_port", "in_port:", OPT_NO_MASK, &in_port_val ),
    OPT_UINT16( "--eth_type", "eth_type:", OPT_NO_MASK, &eth_type_val ),
    OPT_UINT64( "--metadata", "metadata:", OPT_HAS_MASK, &metadata_val ),
    OPT_UINT8( "--ip_proto", "ip_proto:", OPT_NO_MASK, &ip_proto_val ),
    OPT_UINT16( "--udp_src", "udp_src:", OPT_NO_MASK, &udp_src_val ),
    OPT_UINT16( "--udp_dst", "udp_dst:", OPT_NO_MASK, &udp_dst_val ),
    OPT_END()
  };

  char *saveptr;
  const char *sep = " ";
  for ( opt = strtok_r( opt, sep, &saveptr ); opt; opt = strtok_r( NULL, sep, &saveptr ) ) {
    parse_option( opt, opts );
    if ( *( uint64_t * ) opts[ 0 ].value != datapath_id ) {
      return;
    }
  }
}

int
main( int argc, char **argv ) {
  char cmd[ 128 ];
  strncpy( cmd, "dump_flows --datapath_id=0xabcd --metadata=1234/4567 --in_port=1 --eth_type=2048 --ip_proto=17 --udp_src=1 --udp_dst=1", 128 );
  const uint64_t own_datapath_id = 0xabc;

  parse_dump_flows( cmd, own_datapath_id );
  return 0;
}

