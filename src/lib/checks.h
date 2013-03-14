/*
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


#ifndef CHECKS_H
#define CHECKS_H


#define UNUSED( x ) ( void ) ( x )
#define UNREACHABLE() die( "unreachable code" )
#define die_if_NULL( data )                                     \
  if ( data == NULL ) {                                         \
    die( "Argument of %s must not be NULL.", __func__ );        \
  }


#endif // CHECKS_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
