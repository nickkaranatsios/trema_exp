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


#ifndef TIMER_TEST_H
#define TIMER_TEST_H


#ifdef __cplusplus
extern "C" {
#endif


#include <time.h>
#include "doubly_linked_list.h"


#define VALID_TIMESPEC( _a )                                  \
  ( ( ( _a )->tv_sec > 0 || ( _a )->tv_nsec > 0 ) ? 1 : 0 )

#define ADD_TIMESPEC( _a, _b, _return )                       \
  do {                                                        \
    ( _return )->tv_sec = ( _a )->tv_sec + ( _b )->tv_sec;    \
    ( _return )->tv_nsec = ( _a )->tv_nsec + ( _b )->tv_nsec; \
    if ( ( _return )->tv_nsec >= 1000000000 ) {               \
      ( _return )->tv_sec++;                                  \
      ( _return )->tv_nsec -= 1000000000;                     \
    }                                                         \
  }                                                           \
  while ( 0 )

#define SUB_TIMESPEC( _a, _b, _return )                       \
  do {                                                        \
    ( _return )->tv_sec = ( _a )->tv_sec - ( _b )->tv_sec;    \
    ( _return )->tv_nsec = ( _a )->tv_nsec - ( _b )->tv_nsec; \
    if ( ( _return )->tv_nsec < 0 ) {                         \
      ( _return )->tv_sec--;                                  \
      ( _return )->tv_nsec += 1000000000;                     \
    }                                                         \
  }                                                           \
  while ( 0 )

#define TIMESPEC_LESS_THEN( _a, _b )                          \
  ( ( ( _a )->tv_sec == ( _b )->tv_sec ) ?                    \
    ( ( _a )->tv_nsec < ( _b )->tv_nsec ) :                   \
    ( ( _a )->tv_sec < ( _b )->tv_sec ) )

#define TIMESPEC_TO_MICROSECONDS( _a, _b )                    \
  do {                                                        \
    struct timespec round = { 0, 999 };                       \
    ADD_TIMESPEC( _a, &round, &round );                       \
    ( *( _b ) = ( int ) ( round.tv_sec * 1000000 + round.tv_nsec / 1000 ) ); \
  }                                                           \
  while ( 0 )


typedef void ( *timer_callback )( void *user_data );


typedef struct timer_callback_info {
  timer_callback function;
  struct timespec expires_at;
  struct timespec interval;
  void *user_data;
} timer_callback_info;


struct timer_info {
  dlist_element *timer_callbacks;
  pthread_t thread_id;
};

struct timer_info_list {
  struct timer_info **timers;
  uint32_t timers_nr;
  uint32_t timers_alloc;
};


bool ( *new_init_timer )( void );
bool ( *new_finalize_timer )( void );
void ( *new_execute_timer_events )( int *next_timeout_usec );
bool ( *new_add_timer_event_callback )( struct itimerspec *interval, timer_callback callback, void *user_data );
extern bool ( *new_add_periodic_event_callback )( const time_t seconds, timer_callback callback, void *user_data );
bool ( *new_delete_timer_event )( timer_callback callback, void *user_data );


#ifdef __cplusplus
}
#endif


#endif // TIMER_TEST_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
