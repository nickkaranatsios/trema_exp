/*
 * Ruby wrapper around libtrema.
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


#include "controller.h"
#include "message-handler.h"
#include "actions.h"
#include "instructions.h"
#include "messages.h"
#include "message-handler.h"



VALUE mTrema;


void
Init_trema() {
  mTrema = rb_define_module( "Trema" );

  Init_controller();
  Init_actions();
  Init_instructions();
  Init_messages();
  Init_message_handlers();
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
