/*
 * main.c
 *
 *  Created on: 2021Äê9ÔÂ8ÈÕ
 *      Author: Zhaonan Ma
 *      For individual project - temperature sensor
 *
 *  "HPS_usleep" and "HPS_Watchdog" are programs developed by University of Leeds...
 *  	and not uploaded on Github, user needs to develop functions for watchdog reset
 *  	and time delay, or seek source drivers from the University.
 */


// header files
#include <stdio.h>
#include <stdlib.h>
#include "JP2/JP2.h"
#include "HPS_Watchdog/HPS_Watchdog.h"
#include "DE1SoC_SevenSeg/DE1SoC_SevenSeg.h"
#include "HPS_usleep/HPS_usleep.h"

// Peripheral/SWITCH base address.
volatile unsigned int *sw_ptr = (unsigned int *)0xFF200040;

unsigned short config = 0x00FF;
//
// function to detect changes
//
// 0b1111xxx0: no decimal points
// 0b1111xxx1: decimal points
// 0b1111xx0x: 16 bit data
// 0b1111xx1x: 32 bit data
// 		...
//		more bits data need to be added in the code in future
//
bool configChanged()
{
	unsigned int sw = *sw_ptr;
	unsigned short newConfig = 0x00F0;

	// set configurations
	if (sw & 0x200) newConfig |= 0x00F1;	// ban decimal points
	if (sw & 0x100) newConfig |= 0x00F2;	// 32 bits
	// check changes
	if (newConfig != config)
	{
		config = newConfig;		// maintain new configuration
		return true;
	}
	else return false;
}

//debugging function
#include <stdlib.h>
void exitOnFail(signed int status, signed int successStatus){
    if (status != successStatus) {
        exit((int)status); //Add breakpoint here to catch failure
    }
}


// main
int main(void)
{
	// file system
	FILE *fp;

	unsigned int Data;
	unsigned int Pin[4] = {PIN1, PIN2, PIN3, PIN4};
	unsigned int i = 0;
	unsigned int Temp = 0;
	unsigned short validPins = 0x00E0;
	bool isTemp = true;

	//initialise JP2
	exitOnFail(
			JP2_initialise(0xFF200070),
			JP2_SUCCESS);
	// reset watchdog
	HPS_ResetWatchdog();

	//
	//main procedure
	//
	fp = fopen("/Temp/data/data32.txt", "wb");	// change "wb" to "ab" to keep previous data
	while (*sw_ptr & 0x1)
	{
		unsigned short newPins = 0x00E0;
		if (configChanged()) fwrite(&config, 1, 1, fp);
		// scan all pins and read values
		for (i = 0; i <= sizeof(Pin)/4; i++) {
			if (JP2_RST(Pin[i])) {
				Data = JP2_readByte(Pin[i]);
				// display on segments
				Temp = Data & 0x0000FFFF;
				DE1SoC_SevenSeg_SetDoubleDec(4, Temp  >> 8);
				DE1SoC_SevenSeg_SetSingle(3, Temp & 0x00FF);
				DE1SoC_SevenSeg_SetDoubleDec(0, Data >> 24);
				// process
				if (config & 0x2) fwrite(&Data, 4, 1, fp); // 32 bits
				else {										// 16 bits
					if (config & 0x1)	// no decimal points, maintain both temp and humi
					{
						Data &= 0xFF00FF00;
						Data >>= 8;
						Data += (Data >> 8);
					} else {			// decimal points, outputs temp and humi individually
						if (isTemp) Data &= 0x0000FFFF;
						else Data >>= 16;
					}
					fwrite(&Data, 2, 1, fp);	// write 2 bytes
				}
				newPins |= (1 << i);
			}
			printf("stop\n");
		}
		// check and update pin change
		if (newPins != validPins)
			{
				fwrite(&newPins, 1, 1, fp);
				validPins = newPins;
			}
		// toggle between temperature and humidity
		if (!(config & 0x1)) isTemp = !isTemp;
		// 2 second delay for sensor(s), when there are a large amount of sensors...
		//	...this is not needed.
		usleep(2000000);
		HPS_ResetWatchdog();
	}
	fclose(fp);
	//finally reset the watchdog
	HPS_ResetWatchdog();
}
