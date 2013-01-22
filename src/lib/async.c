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


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "log.h"
#include "array-util.h"
#include "async.h"
#include "async-util.h"


static int main_thread_set;
static pthread_t main_thread;
static pthread_key_t async_key;


static NORETURN void 
die_async() {
  struct async *async;

  if ( !pthread_equal( main_thread, pthread_self() ) ) {
    async = pthread_getspecific( async_key );
    if ( async->proc_in >= 0 ) {
      close(async->proc_in);
    }
    if ( async->proc_out >= 0 ) {
      close( async->proc_out );
    }
    pthread_key_delete( async_key );
    pthread_exit( ( void * ) 128 );
  }
  exit( 128 );
}


int
finish_async(struct async *async) {
  void *ret = ( void * ) ( intptr_t ) ( -1 );

  if ( pthread_join( async->tid, &ret ) ) {
    error( "pthread_join failed" );
  }
  return (int) (intptr_t) ret;
}


void 
*run_thread( void *data ) {
  struct async *async = data;
  intptr_t ret;

  pthread_setspecific( async_key, data );
  ret = async->proc( async->data );
  return ( void * ) ret;
}


int 
start_async( struct async *async ) {
  int err;

  if ( !main_thread_set ) {
    main_thread_set = 1;
    main_thread = pthread_self(); // the thread_id of the calling thread
    pthread_key_create( &async_key, NULL );
  }
  err = pthread_create( &async->tid, NULL, run_thread, async );
  if (!err) {
    return 0;
  } else {
    error("Can not create thread %s", strerror(err));
    return -1;
  }
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */