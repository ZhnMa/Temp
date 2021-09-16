/*
 * main.c
 *
 *  Created on: 2021Äê9ÔÂ8ÈÕ
 *      Author: Zhaonan Ma
 *      For individual project - temperature sensor
 */


// header files
#include "JP2/JP2.h"
#include "HPS_Watchdog/HPS_Watchdog.h"
#include "DE1SoC_SevenSeg/DE1SoC_SevenSeg.h"

// Peripheral/KEY base address.
volatile unsigned int *key_ptr = (unsigned int *)0xFF200050;
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
	unsigned int keys_pressed	= 0;			//KEY information
	unsigned int Data = 0;
	unsigned int i = 0;
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
		unsigned int Temp = 0;
		unsigned int Humi = 0;
		//read key value every cycle
		keys_pressed = getPressedKeys();
		//check if leftmost key/KEY[3] is pressed
//		if (keys_pressed & 0x8)
//		{
//			Data = JP2_rtData();
//			Temp = Data & 0x00FF;
//			Humi = (Data & 0xFF00) >> 8;
//			i++;
//		}
		Data = JP2_rtData();
		Temp = Data & 0x00FF;
		Humi = (Data & 0xFF00) >> 8;
		if (Temp != 0xFF && Humi != 0xFF) {
		printf("Temperature:%d, Humidity:%d\n", Temp, Humi);}
		//finally reset the watchdog
		HPS_ResetWatchdog();
	}
}
