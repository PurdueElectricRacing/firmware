#ifndef __HDD_H__
#define __HDD_H__


typedef struct {
   uint8_t deadband_pos;
   uint8_t intensity_pos;
   uint8_t deadband_prev;
   uint8_t intensity_prev;
} hdd_value_t;

#endif
