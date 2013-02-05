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


#include "trema.h"
#include "ruby.h"
#include "messages/packet-in.h"


extern VALUE mTrema;
VALUE mMessageHandler;



VALUE
install_handlers( VALUE self ) {
  set_packet_in_handler( handle_packet_in, ( void * ) self );
  return self;
}


void
Init_message_handlers() {
  mMessageHandler = rb_define_module_under( mTrema, "MessageHandler" );

  rb_define_module_function( mMessageHandler, "install_handlers", install_handlers, 1 );
  Init_packet_in();
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
