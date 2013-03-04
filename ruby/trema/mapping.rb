#
# Copyright (C) 2008-2013 NEC Corporation
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License, version 2, as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#


module Mapping
  def self.included mod
    mod.extend ClassMethods
  end


  module ClassMethods
    def map_to_ofp_type klass
      name = klass.name.demodulize.underscore
      store_at klass, name
      store_it klass, name
      store_xmt klass, name
    end


    def store id, klass 
      ClassMethods.associates[ id ] = klass
    end


    def retrieve type
      ClassMethods.associates[ type ]
    end


    private


    def store_at klass, name
      begin
        type = eval( "OFPAT_#{ name.upcase }" )
        store type, klass
      rescue NameError
      end     
   end


   def store_it klass, name
     begin
        type = eval( "OFPIT_#{ name.upcase }" )
        store type, klass
      rescue NameError
      end     
   end


   def store_xmt klass, name
     begin
       type = eval( "OFPXMT_OFB_#{ name.upcase }" )
       store type, klass
     rescue NameError
     end
   end


   def self.associates
#      @_associates ||= Hash.new{ |h,k| h[k] = [] }
      @_associates ||= Hash.new
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
