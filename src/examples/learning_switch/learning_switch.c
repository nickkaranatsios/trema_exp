/*
 * Simple learning switch application.
 *
 * Author: Yasuhito Takamiya <yasuhito@gmail.com>
 *
 * Copyright (C) 2008-2012 NEC Corporation
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


#include <time.h>
#include <assert.h>
#include "trema.h"


typedef struct {
  struct key {
    uint8_t mac[ OFP_ETH_ALEN ];
    uint64_t datapath_id;
  } key;
  uint32_t port_no;
  time_t last_update;
} forwarding_entry;


time_t
now() {
  return time( NULL );
}


/********************************************************************************
 * packet_in event handler
 ********************************************************************************/

static const int MAX_AGE = 300;


static bool
aged_out( forwarding_entry *entry ) {
  if ( entry->last_update + MAX_AGE < now() ) {
    return true;
  }
  else {
    return false;
  };
}


static void
age_forwarding_db( void *key, void *forwarding_entry, void *forwarding_db ) {
  if ( aged_out( forwarding_entry ) ) {
    delete_hash_entry( forwarding_db, key );
    xfree( forwarding_entry );
  }
}


static void
update_forwarding_db( void *forwarding_db ) {
  foreach_hash( forwarding_db, age_forwarding_db, forwarding_db );
}


static void
learn( hash_table *forwarding_db, struct key new_key, uint32_t port_no ) {
  forwarding_entry *entry = lookup_hash_entry( forwarding_db, &new_key );

  if ( entry == NULL ) {
    entry = xmalloc( sizeof( forwarding_entry ) );
    memcpy( entry->key.mac, new_key.mac, OFP_ETH_ALEN );
    entry->key.datapath_id = new_key.datapath_id;
    insert_hash_entry( forwarding_db, &entry->key, entry );
  }
  entry->port_no = port_no;
  entry->last_update = now();
}


static void
do_flooding( packet_in packet_in, uint32_t in_port ) {
  openflow_actions *actions = create_actions();
  append_action_output( actions, OFPP_ALL, OFPCML_NO_BUFFER );

  buffer *packet_out;
  if ( packet_in.buffer_id == OFP_NO_BUFFER ) {
    buffer *frame = duplicate_buffer( packet_in.data );
    fill_ether_padding( frame );
    packet_out = create_packet_out(
      get_transaction_id(),
      packet_in.buffer_id,
      in_port,
      actions,
      frame
    );
    free_buffer( frame );
  }
  else {
    packet_out = create_packet_out(
      get_transaction_id(),
      packet_in.buffer_id,
      in_port,
      actions,
      NULL
    );
  }
  send_openflow_message( packet_in.datapath_id, packet_out );
  free_buffer( packet_out );
  delete_actions( actions );
}


static void
send_packet( uint32_t destination_port, packet_in packet_in, uint32_t in_port ) {
  openflow_actions *actions = create_actions();
  append_action_output( actions, destination_port, OFPCML_NO_BUFFER );

  openflow_instructions *insts = create_instructions();
  append_instructions_apply_actions( insts, actions );

  oxm_matches *match = create_oxm_matches();
  set_match_from_packet( match, in_port, NULL, packet_in.data );

  buffer *flow_mod = create_flow_mod(
    get_transaction_id(),
    get_cookie(),
    0,
    0,
    OFPFC_ADD,
    60,
    0,
    OFP_HIGH_PRIORITY,
    packet_in.buffer_id,
    0,
    0,
    OFPFF_SEND_FLOW_REM,
    match,
    insts
  );
  send_openflow_message( packet_in.datapath_id, flow_mod );
  free_buffer( flow_mod );
  delete_oxm_matches( match );
  delete_instructions( insts );

  if ( packet_in.buffer_id == OFP_NO_BUFFER ) {
    buffer *frame = duplicate_buffer( packet_in.data );
    fill_ether_padding( frame );
    buffer *packet_out = create_packet_out(
      get_transaction_id(),
      packet_in.buffer_id,
      in_port,
      actions,
      frame
    );
    send_openflow_message( packet_in.datapath_id, packet_out );
    free_buffer( packet_out );
    free_buffer( frame );
  }

  delete_actions( actions );
}


static void
handle_packet_in( uint64_t datapath_id, packet_in message ) {
  if ( message.data == NULL ) {
    error( "data must not be NULL" );
    return;
  }
  if ( !packet_type_ether( message.data ) ) {
    return;
  }

  uint32_t in_port = get_in_port_from_oxm_matches( message.match );
  if ( in_port == 0 ) {
    return;
  }

  struct key new_key;
  packet_info packet_info = get_packet_info( message.data );
  memcpy( new_key.mac, packet_info.eth_macsa, OFP_ETH_ALEN );
  new_key.datapath_id = datapath_id;
  hash_table *forwarding_db = message.user_data;
  learn( forwarding_db, new_key, in_port );

  struct key search_key;
  memcpy( search_key.mac, packet_info.eth_macda, OFP_ETH_ALEN );
  search_key.datapath_id = datapath_id;
  forwarding_entry *destination = lookup_hash_entry( forwarding_db, &search_key );

  if ( destination == NULL ) {
    do_flooding( message, in_port );
  }
  else {
    send_packet( destination->port_no, message, in_port );
  }
}


/********************************************************************************
 * Start learning_switch controller.
 ********************************************************************************/

static const int AGING_INTERVAL = 5;


unsigned int
hash_forwarding_entry( const void *key ) {
  return hash_mac( ( ( const struct key * ) key )->mac );
}


bool
compare_forwarding_entry( const void *x, const void *y ) {
  const forwarding_entry *ex = x;
  const forwarding_entry *ey = y;
  return ( memcmp( ex->key.mac, ey->key.mac, OFP_ETH_ALEN ) == 0 )
           && ( ex->key.datapath_id == ey->key.datapath_id );
}


static void
handle_switch_ready( uint64_t datapath_id, void *user_data ) {
  UNUSED( user_data );

  openflow_actions *actions = create_actions();
  append_action_output( actions, OFPP_CONTROLLER, OFPCML_NO_BUFFER );

  openflow_instructions *insts = create_instructions();
  append_instructions_apply_actions( insts, actions );

  buffer *flow_mod = create_flow_mod(
    get_transaction_id(),
    get_cookie(),
    0,
    0,
    OFPFC_ADD,
    0,
    0,
    OFP_LOW_PRIORITY,
    OFP_NO_BUFFER,
    0,
    0,
    OFPFF_SEND_FLOW_REM,
    NULL,
    insts
  );
  send_openflow_message( datapath_id, flow_mod );
  free_buffer( flow_mod );
  delete_instructions( insts );
}


int
main( int argc, char *argv[] ) {
  init_trema( &argc, &argv );

  hash_table *forwarding_db = create_hash( compare_forwarding_entry, hash_forwarding_entry );
  add_periodic_event_callback( AGING_INTERVAL, update_forwarding_db, forwarding_db );
  set_packet_in_handler( handle_packet_in, forwarding_db );

  set_switch_ready_handler( handle_switch_ready, NULL );

  start_trema();

  return 0;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
