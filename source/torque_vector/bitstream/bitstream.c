#include "bitstream.h"

void bitstream_data_callback(CanParsedData_t* msg_data_a)
{
    msg_data_a->bitstream_data.word_0;
    msg_data_a->bitstream_data.word_1;
}