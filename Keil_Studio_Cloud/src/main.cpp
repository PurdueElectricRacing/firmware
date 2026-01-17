/*******************************************************************************
Copyright (c) 2020 - Analog Devices Inc. All Rights Reserved.
This software is proprietary & confidential to Analog Devices, Inc.
and its licensor.
------------------------------------------------------------------------------*/

/* Includes ------------------------------------------------------------------*/
#include "adbms_main.h"

/*
https://os.mbed.com/platforms/SDP_K1/
Mbed SDP-K1 board MOSI, MOSI, SCK definition.
Arduino Header Pinout
MOSI = D11
MISO = D12
SCK  = D13
CSB =  D10
*/
int main(void);
void spi_init(void);

DigitalOut chip_select(D10);                /* SPI chip select */
DigitalOut mosi(D11);                       /* MOSI */
DigitalIn miso(D12);                        /* MISO */
DigitalOut sclk(D13);                       /* SCK */
SPI spi(D11, D12, D13);                     /* SPI MOSI, MISO, SCK */
Serial pc(USBTX, USBRX);                    /* USB Tx,Rx */
Timer timer;


int main()
{
    spi_init();
    adbms_main();
    return 0;
}

void spi_init()
{   
    chip_select = 1;
    spi.format(8,0);            /* 8bit data, CPOL-CPHA= 0*/
    spi.frequency(2000000);     /* SPI clock 2Mhz */
    pc.baud(115200);              /* Usb baud rate */   
}