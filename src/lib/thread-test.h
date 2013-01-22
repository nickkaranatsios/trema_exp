/*
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


#ifndef THREAD_TEST
#define THREAD_TEST


#ifdef __cplusplus
extern "C" {
#endif


#include <sys/types.h>
#include "bool.h"


#define   EVENT_HANDLER_INITIALIZED  1 << 0
#define   EVENT_HANDLER_RUNNING      1 << 1
#define   EVENT_HANDLER_STOP         1 << 2
#define   EVENT_HANDLER_FINALIZED    1 << 3


void init_thread_test();
typedef void ( *event_fd_callback )( int, void *data );
typedef void ( *external_callback_t )( void );


struct event_fd {
  int fd;
  event_fd_callback read_callback;
  event_fd_callback write_callback;
  void *read_data;
  void *write_data;
};


struct event_info {
  struct event_fd event_list[ FD_SETSIZE ];
  struct event_fd *event_last;
  struct event_fd *event_fd_set[ FD_SETSIZE ];
  external_callback_t external_callback;
  fd_set event_read_set; 
  fd_set event_write_set;
  fd_set current_read_set;
  fd_set current_write_set;
  pthread_t thread_id;
  pthread_mutex_t mutex;
  int event_handler_state; 
  int fd_set_size;
};


struct event_info_list {
  struct event_info **events;
  uint32_t events_nr;
  uint32_t events_alloc;
};


void ( *new_init_event_handler )();
void ( *new_finalize_event_handler )();
bool ( *new_start_event_handler )();
void ( *new_stop_event_handler )();
void ( *new_set_readable )( int fd, bool state );
void ( *new_set_writable )( int fd, bool state );
void ( *new_set_fd_handler )( int fd, event_fd_callback read_callback, void *read_data, event_fd_callback write_callback, void *write_data );
bool ( *new_set_external_callback )( external_callback_t callback );
void ( *new_delete_fd_handler )( int fd );
bool ( *new_readable )( int fd );
bool ( *new_writable )( int fd );


#ifdef __cplusplus
}
#endif


#endif // THREAD_TEST_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
