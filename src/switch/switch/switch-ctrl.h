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


#ifndef SWITCH_CTRL_H
#define SWITCH_CTRL_H


#ifdef __cplusplus
extern "C" {
#endif


// how often to poll for control command
#define CTRL_INTERVAL 5
#define DUMP_FLOW_FIELDS \
  "\"table_id\":%u, \
  \"duration_sec\":%u, \
  \"priority\":%u, \
  \"idle_timeout\":%u, \
  \"hard_timeout\":%u, \
  \"flags\":%u, \
  \"cookie\":%"PRIu64", \
  \"packet_count\":%"PRIu64", \
  \"byte_count\":%"PRIu64""

#define DUMP_TABLE_FIELDS \
  "\"table_id\":%u, \
  \"name\":%s, \
  \"config\":%u, \
  \"max_entries\":%u, \
  \"metadata_match\":%"PRIu64", \
  \"metadata_write\":%"PRIu64""

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


#define OPT_END() { ( NULL ), ( NULL ), 0, ( NULL ), OPTION_END  }
#define OPT_UINT8( p, j, f, v ) { ( p ), ( j ), ( f ), ( v ), OPTION_UINT8 }
#define OPT_UINT16( p, j, f, v ) { ( p ), ( j ), ( f ), ( v ), OPTION_UINT16 }
#define OPT_UINT32( p, j, f, v ) { ( p ), ( j ), ( f ), ( v ), OPTION_UINT32 }
#define OPT_UINT64( p, j, f, v ) { ( p ), ( j ), ( f ), ( v ), OPTION_UINT64 }
#define OPT_STRING( p, j, f, v ) { ( p ), ( j ), ( f ), ( v ), OPTION_STRING }
#define OPT_SET( f ) ( ( f ) |= OPT_VALUE_SET )
#define OPT_MASK_SET( f ) ( ( ( f ) & OPT_HAS_MASK ) == OPT_HAS_MASK ) 


void ( *check_ctrl )( void *user_data );


#ifdef __cplusplus
}
#endif


#endif // SWITCH_CTRL_H 


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
