/*
 * DE1SoC_SevenSeg.c
 *
 *  Created on: 12 Feb 2021
 *      Author: Harry Clegg
 *      		You!
 *-----------------------------------------
 *	From earlier tasks.
 *	Copied and pasted for Mini-Project.
 *		By: Zhaonan Ma [el20z2m], SID: 201397999
 *
 *	2021-5-7
 */

#include "DE1SoC_SevenSeg.h"

// ToDo: Add the base addresses of the seven segment display peripherals.
volatile unsigned char *sevenseg_base_lo_ptr = (volatile unsigned char *)0xFF200020;
volatile unsigned char *sevenseg_base_hi_ptr = (volatile unsigned char *)0xFF200030;

// There are four HEX displays attached to the low (first) address.
#define SEVENSEG_N_DISPLAYS_LO 4

// There are two HEX displays attached to the high (second) address.
#define SEVENSEG_N_DISPLAYS_HI 2

void DE1SoC_SevenSeg_Write(unsigned int display, unsigned char value) {
    // Select between the two addresses so that the higher level functions
    // have a seamless interface.
    if (display < SEVENSEG_N_DISPLAYS_LO) {
        // If we are targeting a low address, use byte addressing to access
        // directly.
        sevenseg_base_lo_ptr[display] = value;
    } else {
        // If we are targeting a high address, shift down so byte addressing
        // works.
        display = display - SEVENSEG_N_DISPLAYS_LO;
        sevenseg_base_hi_ptr[display] = value;
    }
}

void DE1SoC_SevenSeg_SetSingle(unsigned int display, unsigned int value) {
    // ToDo: Write the code for driving a single seven segment display here.
    // Your function should turn a real value 0-F into the correctly encoded
    // bits to enable the correct segments on the seven segment display to
    // illuminate. Use the DE1SoC_SevenSeg_Write function you created earlier
    // to set the bits of the display.
	switch (value) {
		case 0x0: value = 0x3F; break;  //0111111;
		case 0x1: value = 0x06; break;  //0000110;
		case 0x2: value = 0x5B; break;  //1011011;
		case 0x3: value = 0x4F; break;  //1001111;
		case 0x4: value = 0x66; break;  //1100110;
		case 0x5: value = 0x6D; break;  //1101101;
		case 0x6: value = 0x7D; break;  //1111101;
		case 0x7: value = 0x07; break;  //0000111;
		case 0x8: value = 0x7F; break;  //1111111;
		case 0x9: value = 0x6F; break;  //1101111;
		case 0xA: value = 0x77; break;  //1110111;
		case 0xB: value = 0x7C; break;  //1111100;
		case 0xC: value = 0x39; break;  //0111001;
		case 0xD: value = 0x5E; break;  //1011110;
		case 0xE: value = 0x79; break;  //1111001;
		case 0xF: value = 0x71; break;  //1110001;
			default: value = 0x40;  //1000000;
		//Each case should involve a break instruction(except for default, of course) for the loop to work.
	}

	//for assignment 2, display the decimal points for minute and hour
	if (display == 2 || display == 4) value += (1 << 7);

	DE1SoC_SevenSeg_Write(display, value);
}

void DE1SoC_SevenSeg_SetDoubleHex(unsigned int display, unsigned int value) {
    // ToDo: Write the code for setting a pair of seven segment displays
    // here. Good coding practice suggests your solution should call
    // DE1SoC_SevenSeg_SetSingle() twice.
    // This function should show the first digit of a HEXADECIMAL number on
    // the specified 'display', and the second digit on the display to
    // the left of the specified display.

    /** Some examples:
     *
     *    input | output | HEX(N+1) | HEX(N)
     *    ----- | ------ | -------- | ------
     *        5 |     05 |        0 |      5
     *       30 |     1E |        1 |      E
     *     0x60 |     60 |        6 |      0
     */
	unsigned int value1 = 0;
	unsigned int value2 = 0; //Set two digits for two displays

	if (value > 0xFF) {             //For number greater than 0xFF, display 2 dashes rather than restart from 0x00.
		value1 = 0x40;
		value2 = 0x40;  // Dash.
	}
	else {
		value1 = value&0xF;        //First digit.
		value2 = (value>>4)&0xF;   //Second digit.
	}

	DE1SoC_SevenSeg_SetSingle(display, value1);
	DE1SoC_SevenSeg_SetSingle(display+1, value2);  //Call the function twice for 2 digits.
}

void DE1SoC_SevenSeg_SetDoubleDec(unsigned int display, unsigned int value) {
    // ToDo: Write the code for setting a pair of seven segment displays
    // here. Good coding practice suggests your solution should call
    // DE1SoC_SevenSeg_SetSingle() twice.
    // This function should show the first digit of a DECIMAL number on
    // the specified 'display', and the second digit on the display to
    // the left of the specified display.

    /** Some examples:
     *
     *	  input | output | HEX(N+1) | HEX(N)
     *    ----- | ------ | -------- | ------
     *        5 |     05 |        0 |      5
     *       30 |     30 |        3 |      0
     *     0x90 |     5A |        5 |      A
     */
	unsigned int value1 = 0;
	unsigned int value2 = 0;

	if (value > 99) {            //For number more than 99, display 2 dashes.
		value1 = 0x40;
		value2 = 0x40;  //Dash
	}
	else {
		value1 = value%10;    //Number in the unit.
		value2 = value/10;	  //Number in the ten's place.
		//Turn hex to dec, and divide the digits.
	}

	DE1SoC_SevenSeg_SetSingle(display, value1);
	DE1SoC_SevenSeg_SetSingle(display+1, value2);  //Call the function twice for 2 digits.
}
