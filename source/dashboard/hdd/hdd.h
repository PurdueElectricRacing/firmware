#ifndef __HDD_H__
#define __HDD_H__


typedef struct {
   uint8_t curr_addr;
   uint16_t mux_1_val;
   uint16_t mux_2_val;
   bool mux_1_arr[12];
   bool mux_2_arr[12];
} hdd_value_t;
#endif
