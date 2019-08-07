// ATM90E36 utility - read and write registers, calibrate. STM32F103 firmware.
// Copyright (C) 2019 Mastro Gippo
//
// This program is free software: you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of  MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.

#include "mbed.h"

Serial pc(SERIAL_TX, SERIAL_RX);
SPI device(PB_5, PB_4, PB_3);
DigitalOut cs(PA_8);

DigitalOut led1(LED1);

uint16_t ReadSPI(uint16_t addr)
{
    cs = 0;
    device.write(0x8000+addr);
    int res = device.write(0x0000);
    cs = 1;
    return res;
}

uint16_t WriteSPI(uint16_t addr, uint16_t val)
{
    cs = 0;
    device.write(addr);
    int res = device.write(val);
    cs = 1;
    return res;
}

unsigned char smg[50];
int count = 0;
int num;
char tc;
bool flagser_getend;

unsigned char b1[] = {0xFE, 0xFE, 0x68, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x68, 
    0x81, 0x08, 0x65, 0xF3, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0xE3, 0x16};
unsigned char b2[] = {0xFE, 0xFE, 0x68, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x68, 
    0x81, 0x03, 0x43, 0x13, 0x49, 0xF3, 0x16};
//unsigned char b3[] = {0xFE, 0xFE, 0x68, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x68, 
//    0x81, 0x04, 0xA3, 0x13, 0x00, 0x00, 0x0B, 0x16};
unsigned char b3[] = {0xFE, 0xFE, 0x68, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x68, 
    0x81, 0x04, 0xA3, 0x13, 0x33, 0x33, 0x71, 0x16};
unsigned char rmem[] = {0xFE, 0xFE, 0x68, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x68, 
    0x81, 0x04, 0xA3, 0x13, 0x33, 0x33, 0x71, 0x16};
            
void Wr(uint8_t * buf, int len)
{
    for(int i = 0; i < len; i++)
    {
        while(!pc.writable());
        pc.putc(buf[i]);
        wait(0.001);
    }
}

void ParseAnswer()
{
    if(smg[1] == 'R')
    {

        while(!pc.writable());
        pc.putc(0x69);
        while(!pc.writable());
        pc.putc(0x69);
        while(!pc.writable());
        pc.putc(smg[2]);
        while(!pc.writable());
        pc.putc(smg[3]);
        
        uint16_t tmp;
        for(uint16_t i = smg[2]; i <= smg[3]; i++)
        {
            tmp = ReadSPI(i);
            while(!pc.writable());
            pc.putc((uint8_t)(tmp));
            while(!pc.writable());
            pc.putc((uint8_t)(tmp >> 8));
        }
    }
    else if(smg[1] == 'W')
    {
        uint16_t Addr = smg[2];
        Addr = Addr<<8;
        Addr += smg[3];
        uint16_t Val = smg[4];
        Val = Val<<8;
        Val += smg[5];
        WriteSPI(Addr, Val);
    }
}

void SerInt() {
    unsigned char temp;
    static unsigned char len;
    static unsigned char max;
    while(pc.readable())
    {
        temp = pc.getc();
        if(count == 0)
        {
            if(temp == 0x69) //start
                smg[count++] = temp;
        }
        else if(count == 1)
        {
            smg[count++] = temp;
            if(temp == 'R')
                max = 4;
            else if(temp == 'W')
                max = 6;
        }        
        else// if(count <= 3) //get address
        { 
            smg[count++] = temp;
            if(count >= max)
            {
                ParseAnswer();
                count = 0;
            }            
        }
    }
}

void SerialStart(void) {
    pc.baud(115200);
    //pc.baud(921600);
    flagser_getend = 0;
    pc.attach(&SerInt,Serial::RxIrq);
    count = 0;
}
 
int main() {
    SerialStart();

    cs = 1;
    
    // Setup the spi for 8 bit data, high steady state clock,
    // second edge capture, with a 0.5MHz clock rate
    device.format(16,0);
    device.frequency(500000);

    wait(2);
/*
    uint16_t tmp;
    for(uint16_t i = 0; i < 0x100; i++)
    {
        tmp = ReadSPI(i);
        while(!pc.writable());
        pc.putc((uint8_t)(tmp));
        while(!pc.writable());
        pc.putc((uint8_t)(tmp >> 8));
    }*/

    while(1) {
        led1 = 1;
        wait(0.2);
        led1 = 0;
        wait(0.2);
        //ser.printf("ciao\r\n");
        
    }
}