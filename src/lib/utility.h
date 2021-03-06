/*
 * Utility functions.
 *
 * Author: Yasuhito Takamiya <yasuhito@gmail.com>
 *
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


#ifndef UTILITY_H
#define UTILITY_H


#include <stdint.h>
#include <stdio.h>
#include "bool.h"
#include "openflow.h"
#include "oxm_match.h"

#define MATCH_STRING_LENGTH 2048

extern void ( *die )( const char *format, ... );

unsigned int hash_core( const void *key, int size );

bool compare_string( const void *x, const void *y );
unsigned int hash_string( const void *key );

bool compare_mac( const void *x, const void *y );
unsigned int hash_mac( const void *mac );
uint64_t mac_to_uint64( const uint8_t *mac );

bool compare_uint32( const void *x, const void *y );
unsigned int hash_uint32( const void *key );

bool compare_datapath_id( const void *x, const void *y );
unsigned int hash_datapath_id( const void *key );

bool string_to_datapath_id( const char *str, uint64_t *datapath_id );

bool match_to_string( const oxm_matches *match, char *str, size_t size );

bool port_to_string( const struct ofp_port *phy_port, char *str, size_t size );
bool actions_to_string( const struct ofp_action_header *actions, uint16_t actions_length, char *str, size_t str_length );
bool instructions_to_string( const struct ofp_instruction *instructions, uint16_t instructions_length, char *str, size_t str_length );

uint16_t get_checksum( uint16_t *pos, uint32_t size );

uint32_t get_in_port_from_oxm_matches( const oxm_matches *match );


#endif // UTILITY_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
