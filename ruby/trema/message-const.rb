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


require "trema/monkey-patch/kernel"


module Trema
  module MessageConst
    config_flags_hash = {
      :ofpc_frag_normal => frag_normal,
      :ofpc_frag_drop => frag_drop,
      :ofpc_frag_reasm => frag_reasm,
      :ofpc_frag_mask => frag_mask
    }
    enum_hash config_flags_hash
    CONFIG_FLAGS = config_flags_hash.values.freeze
    

    enum_hash(
      :ofpfc_add => flow_mod_add,
      :ofpfc_modify => flow_mod_modify,
      :ofpfc_modify_strict => flow_mod_modify_strict,
      :ofpfc_delete => flow_mod_delete,
      :ofpfc_delete_strict => flow_mod_delete_strict
    )

    
    flow_mod_flags = %w( 
      ofpff_send_flow_rem
      ofpff_check_overlap
      ofpff_reset_counts  
      ofpff_no_pkt_counts 
      ofpff_no_byt_counts 
    )
    enum_range flow_mod_flags

    enum_hash(
     :ofpp_max => max_port,
     :ofpp_in_port => in_port,
     :ofpp_table => table_port,
     :ofpp_normal => normal_port,
     :ofpp_flood => flood_port,
     :ofpp_all => all_ports,
     :ofpp_controller => controller_port,
     :ofpp_local => local_port,
     :ofpp_any => any_port
    )

    enum_hash(
      :ofpcml_max => controller_max_len_max,
      :ofpcml_no_buffer => controller_max_len_no_buffer
    )

    enum_hash( :ofp_no_buffer => no_buffer )

    enum_hash( 
      :ofp_default_priority => default_priority,
      :ofp_high_priority => high_priority,
      :ofp_low_priority => low_priority 
    )
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
