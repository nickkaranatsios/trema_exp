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


require "trema/accessor-base"


module Trema
  class BasicAction < AccessorBase
    include Actions


    #
    # appends its action into a list of actions
    #
    def append_action action
      params = {}
      instance_variables.each do | each |
        params[ each.to_s.sub( '@', '' ).to_sym ] = instance_variable_get( each )
      end
      if self.instance_of? Actions::SetField
        return append_set_field action, params
      end
      method = "append_#{ self.class.name.demodulize.underscore }"
      __send__ method, action, params
    end


    def append_set_field set_field, params
      options = {}
      params.each do | k, v |
        v.each.instance_variables.each do | attr |
          options[ attr.to_s.sub( '@', '' ).to_sym ] = v.each.instance_variable_get( attr )
        end
      end
puts options.inspect
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End: