/*
 * main.c
 *
 *  Created on: 2021��9��8��
 *      Author: Zhaonan Ma
 *      For individual project - temperature sensor
 */


// header files
#include <stdio.h>
#include <stdlib.h>
#include "JP2/JP2.h"
#include "HPS_Watchdog/HPS_Watchdog.h"
#include "DE1SoC_SevenSeg/DE1SoC_SevenSeg.h"
#include "HPS_usleep/HPS_usleep.h"

// Peripheral/KEY base address.
volatile unsigned int *key_ptr = (unsigned int *)0xFF200050;
// Peripheral/SWITCH base address.
volatile unsigned int *sw_ptr = (unsigned int *)0xFF200040;

// Store the state of the keys last time we checked.
unsigned int key_last_state = 0;
//function to get which key pressed
unsigned int getPressedKeys() {
    // Store the current state of the keys.
    unsigned int key_current_state = *key_ptr;
    // If the key was down last cycle, and is up now, mark as pressed.
    unsigned int keys_pressed = (~key_current_state) & (key_last_state);
    // Save the key state for next time, so we can compare the next state to this.
    key_last_state = key_current_state;
    // Return result.
    return keys_pressed;
}

unsigned short config = 0x00FF;
//
// function to detect changes
//
// 0b1111xxx0: decimal points allowed
// 0b1111xxx1: no decimal points
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
	if (*sw_ptr & 200) newConfig |= 0x00F1;	// ban decimal points
	if (*sw_ptr & 100) newConfig |= 0x00F2;	// ban priority
	if (*sw_ptr & 80) newConfig |= 0x00F4;	// 32 bits
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

	unsigned int keys_pressed	= 0;			//KEY information
	unsigned int Data;
	unsigned int Pin[4] = {PIN1, PIN2, PIN3, PIN4};
	unsigned int i = 0;
	unsigned int Temp = 0, Humi = 0;
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
	while(1)
	{

		//read key value every cycle
		keys_pressed = getPressedKeys();

		fp = fopen("/Temp/data/data32.txt", "wb");
		while (*sw_ptr & 0x1)
		{
			unsigned short newPins = 0x00E0;
			if (configChanged()) fwrite(&config, 1, 1, fp);

			for (i = 0; i <= sizeof(Pin)/4; i++) {
				if (JP2_RST(Pin[i])) {
					Data = JP2_readByte(Pin[i]);
					// process
					if (config & 0xF2) fwrite(&Data, 4, 1, fp); // 32 bits
					else {										// 16 bits
						if (config & 0xF1)	// no decimal points, maintain both temp and humi
						{
							Data &= 0xFF00FF00;
							Data >>= 8;
							Data += (Data >> 8);
						} else {			// decimal points, outputs temp and humi individually
							if (isTemp) Data >>= 16;
							else Data &= 0x0000FFFF;
						}
					}
					newPins |= (1 << i);
					fwrite(&Data, 2, 1, fp);	// write 2 bytes
				} else printf("stop\n");
			}
			// check and update pin change
			if (newPins != validPins)
				{
					fwrite(&newPins, 1, 1, fp);
					validPins = newPins;
				}
			// toggle between temperature and humidity
			isTemp = ~isTemp;
			// 2 second delay for sensor(s), when there are a large amount of sensors...
			//	...this is not needed.
			usleep(2000000);
			HPS_ResetWatchdog();
		}
		fclose(fp);
//		Temp = Data & 0x00FF;
//		Humi = (Data & 0xFF00) >> 8;
//		if (Temp != 0xFF && Humi != 0xFF) {
//			DE1SoC_SevenSeg_SetDoubleDec(0, Temp);
//			DE1SoC_SevenSeg_SetDoubleDec(4, Humi);
//			printf("Temperature:%d, Humidity:%d\n", Temp, Humi);
//		}
		//finally reset the watchdog
		HPS_ResetWatchdog();
	}
}
