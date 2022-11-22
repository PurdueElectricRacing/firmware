""" gen_faults.py: Generates embedded code for fault messages/structure, CAN Tx/Rx functionality"""

import  generator

#
# GENERATION STRINGS
#
gen_id_start = "BEGIN AUTO ID DEFS"
gen_id_stop = "END AUTO ID DEFS"
gen_totals_start = "BEGIN AUTO TOTAL DEFS"
gen_totals_stop = "END AUTO TOTAL DEFS"
gen_priority_start = "BEGIN AUTO PRIORITY DEFS"
gen_priority_stop = "END AUTO PRIORITY DEFS"
gen_max_start = "BEGIN AUTO MAX DEFS"
gen_max_start = "END AUTO MAX DEFS"
gen_latch_start = "BEGIN AUTO LATCH DEFS"
gen_latch_stop = "END AUOT LATCH DEFS"
gen_unlatch_start = "BEGIN AUTO UNLATCH DEFS"
gen_unlatch_stop = "END AUOT UNLATCH DEFS"
gen_screenmsg_start = "BEGIN AUTO SCREENMSG DEFS"
gen_screenmsg_stop = "END AUTO SCREENMSG DEFS"
gen_enum_start = "BEGIN AUTO ENUM DEFS"
gen_enum_stop = "END AUOT ENUM DEFS"