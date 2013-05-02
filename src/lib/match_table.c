/*
 * Author: Kazushi SUGYO
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


#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <openflow.h>
#include "checks.h"
#include "match_table.h"
#include "log.h"
#include "wrapper.h"


#define VLAN_VID_MASK 0x0fff // 12 bits
#define VLAN_PCP_MASK 0x07 // 3 bits


typedef struct {
  oxm_matches *match; // match data. host byte order
  uint16_t priority;
  void *data;
} match_entry;


typedef struct {
  list_element *wildcards_table;
  pthread_mutex_t *mutex;
} match_table;


typedef struct {
  oxm_matches *match;
  void ( * function )( oxm_matches *, uint16_t, void *, void * );
  void *user_data;
} match_walker;


match_table *_match_table_head = NULL; // non-static variable for use in unit testing


static match_entry *
allocate_match_entry( oxm_matches *match, uint16_t priority, void *data ) {
  match_entry *new_entry = xmalloc( sizeof( match_entry ) );
  new_entry->match = duplicate_oxm_matches( match );
  new_entry->priority = priority;
  new_entry->data = data;

  return new_entry;
}


static void
free_match_entry( match_entry *free_entry ) {
  assert( free_entry != NULL );
  if ( free_entry->match != NULL ) {
    delete_oxm_matches( free_entry->match );
  }

  xfree( free_entry );
}


static bool
compare_filter_match( oxm_matches *x, oxm_matches *y ) {
  return compare_oxm_match( x, y );
}


static void
init_wildcards_match_table( list_element **wildcards_table ) {
  assert( wildcards_table != NULL);

  create_list( wildcards_table );
}


static void
finalize_wildcards_match_table( list_element *wildcards_table ) {
  list_element *element;
  for ( element = wildcards_table; element != NULL; element = element->next ) {
    free_match_entry( element->data );
    element->data = NULL;
  }
  delete_list( wildcards_table );
}


static bool
insert_wildcards_match_entry( list_element **wildcards_table, oxm_matches *match, uint16_t priority, void *data ) {
  assert( match != NULL );

  list_element *element;
  for ( element = *wildcards_table; element != NULL; element = element->next ) {
    match_entry *entry = element->data;
    if ( entry->priority < priority ) {
      break;
    }
    assert( entry != NULL );
    if ( entry->priority == priority && compare_oxm_match_strict( entry->match, match ) ) {
      char match_string[ MATCH_STRING_LENGTH ];
      match_to_string( match, match_string, sizeof( match_string ) );
      warn( "wildcards match entry already exists ( match = [%s], priority = %u )",
            match_string, priority );
      return false;
    }
  }
  match_entry *new_entry = allocate_match_entry( match, priority, data );
  if ( element == NULL ) {
    // tail
    append_to_tail( wildcards_table, new_entry );
  }
  else if ( element == *wildcards_table ) {
    // head
    insert_in_front( wildcards_table, new_entry );
  }
  else {
    // insert before
    insert_before( wildcards_table, element->data, new_entry );
  }
  return true;
}


static match_entry *
lookup_wildcards_match_strict_entry( list_element *wildcards_table, oxm_matches *match, uint16_t priority ) {
  assert( match != NULL );

  list_element *element;
  for ( element = wildcards_table; element != NULL; element = element->next ) {
    match_entry *entry = element->data;
    if ( entry->priority < priority ) {
      break;
    }
    if ( entry->priority == priority && compare_oxm_match_strict( entry->match, match ) ) {
      return entry;
    }
  }
  return NULL;
}


static match_entry *
lookup_wildcards_match_entry( list_element *wildcards_table, oxm_matches *match ) {
  assert( match != NULL );

  list_element *element;
  for ( element = wildcards_table; element != NULL; element = element->next ) {
    match_entry *entry = element->data;
    if ( compare_oxm_match( entry->match, match ) ) {
      return entry;
    }
  }
  return NULL;
}


static bool
update_wildcards_match_entry( list_element *wildcards_table, oxm_matches *match, uint16_t priority, void *data ) {
  assert( match != NULL );

  match_entry *entry = lookup_wildcards_match_strict_entry( wildcards_table, match, priority );
  if ( entry == NULL ) {
    char match_string[ MATCH_STRING_LENGTH ];
    match_to_string( match, match_string, sizeof( match_string ) );
    warn( "wildcards match entry not found ( match = [%s], priority = %u )",
          match_string, priority );
    return false;
  }
  entry->data = data;
  return true;
}


static void *
delete_wildcards_match_strict_entry( list_element **wildcards_table, oxm_matches *match, uint16_t priority ) {
  assert( match != NULL );

  match_entry *entry = lookup_wildcards_match_strict_entry( *wildcards_table, match, priority );
  if ( entry == NULL ) {
    char match_string[ MATCH_STRING_LENGTH ];
    match_to_string( match, match_string, sizeof( match_string ) );
    warn( "wildcards match entry not found ( match = [%s], priority = %u )",
          match_string, priority );
    return NULL;
  }
  void *data = entry->data;
  delete_element( wildcards_table, entry );
  free_match_entry( entry );
  return data;
}


static void
map_wildcards_match_table( list_element *wildcards_table, oxm_matches *match, void function( oxm_matches *, uint16_t, void *, void * ), void * user_data ) {
  assert( function != NULL );

  list_element *element = wildcards_table;
  while ( element != NULL ) {
    match_entry *entry = element->data;
    element = element->next;
    if ( match != NULL ) {
      if ( !compare_filter_match( match, entry->match ) ) {
        continue;
      }
    }
    function( entry->match, entry->priority, entry->data, user_data );
  }
}


void
init_match_table( void ) {
  if ( _match_table_head != NULL ) {
    die( "match table is already initialized." );
  }

  match_table *table = xmalloc( sizeof( match_table ) );
  init_wildcards_match_table( &table->wildcards_table );
  pthread_mutexattr_t attr;
  pthread_mutexattr_init( &attr );
  pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_RECURSIVE_NP );
  table->mutex = xmalloc( sizeof( pthread_mutex_t ) );
  pthread_mutex_init( table->mutex, &attr );

  _match_table_head = table;
}


void
finalize_match_table( void ) {
  if ( _match_table_head == NULL ) {
    die( "match table is not initialized." );
  }

  pthread_mutex_t *mutex = _match_table_head->mutex;

  pthread_mutex_lock( mutex );
  finalize_wildcards_match_table( _match_table_head->wildcards_table );
  xfree( _match_table_head );
  _match_table_head = NULL;
  pthread_mutex_unlock( mutex );
  pthread_mutex_destroy( mutex );
  xfree( mutex );
}


bool
insert_match_entry( oxm_matches *match, uint16_t priority, void *data ) {
  if ( _match_table_head == NULL ) {
    die( "match table is not initialized." );
  }

  pthread_mutex_lock( _match_table_head->mutex );
  bool result = insert_wildcards_match_entry( &_match_table_head->wildcards_table, match, priority, data );
  pthread_mutex_unlock( _match_table_head->mutex );
  return result;
}


void *
lookup_match_strict_entry( oxm_matches *match, uint16_t priority ) {
  if ( _match_table_head == NULL ) {
    die( "match table is not initialized." );
  }

  pthread_mutex_lock( _match_table_head->mutex );
  match_entry *entry = lookup_wildcards_match_strict_entry( _match_table_head->wildcards_table, match, priority );
  void *data = ( entry != NULL ? entry->data : NULL );
  pthread_mutex_unlock( _match_table_head->mutex );
  return data;
}


void *
lookup_match_entry( oxm_matches *match ) {
  if ( _match_table_head == NULL ) {
    die( "match table is not initialized." );
  }

  pthread_mutex_lock( _match_table_head->mutex );
  match_entry *entry = lookup_wildcards_match_entry( _match_table_head->wildcards_table, match );
  void *data = ( entry != NULL ? entry->data : NULL );
  pthread_mutex_unlock( _match_table_head->mutex );
  return data;
}


bool
update_match_entry( oxm_matches *match, uint16_t priority, void *data ) {
  if ( _match_table_head == NULL ) {
    die( "match table is not initialized." );
  }

  pthread_mutex_lock( _match_table_head->mutex );
  bool result = update_wildcards_match_entry( _match_table_head->wildcards_table, match, priority, data );
  pthread_mutex_unlock( _match_table_head->mutex );
  return result;
}


void *
delete_match_strict_entry( oxm_matches *match, uint16_t priority ) {
  if ( _match_table_head == NULL ) {
    die( "match table is not initialized." );
  }

  pthread_mutex_lock( _match_table_head->mutex );
  void *data = NULL;
  data = delete_wildcards_match_strict_entry( &_match_table_head->wildcards_table, match, priority );
  pthread_mutex_unlock( _match_table_head->mutex );
  return data;
}


static void
_map_match_table( oxm_matches *match, void function( oxm_matches *match, uint16_t priority, void *data, void *user_data ), void *user_data ) {
  if ( _match_table_head == NULL ) {
    die( "match table is not initialized." );
  }
  if ( function == NULL ) {
    die( "function must not be NULL" );
  }

  pthread_mutex_lock( _match_table_head->mutex );
  map_wildcards_match_table( _match_table_head->wildcards_table, match, function, user_data );
  pthread_mutex_unlock( _match_table_head->mutex );
}


void
foreach_match_table( void function( oxm_matches *, uint16_t, void *, void *), void *user_data ) {
  _map_match_table( NULL, function, user_data );
}


void
map_match_table( oxm_matches *match, void function( oxm_matches *match, uint16_t priority, void *data, void *user_data ), void *user_data ) {
  _map_match_table( match, function, user_data );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
