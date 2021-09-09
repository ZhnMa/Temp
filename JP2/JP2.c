/*
 * 	Source file for controlling General Purpose I/O Expansion
 * 		For: Embedded System Mini-Project
 * 		By:  Zhaonan Ma [el20z2m], SID: 201397999
 *
 * 	Ref:
 *		DHT11 Manual, www.aosong.com
 *		DHT11 Driver Code for STM32 (Chinese text), https://zhuanlan.zhihu.com/p/347904660
 */

#include "JP2.h"
#include "HPS_usleep/HPS_usleep.h"

// Driver Base Addresses
volatile unsigned int *jp2_pio_ptr = 0x0;	//0xFF200070
//Driver initialised
bool jp2_initialised = false;

// bit map
#define PIN1 (1 << 9)		// data
#define PIN2 (1 << 11)		// vcc
#define PIN3 (1 << 12)		// GND

// address offsets
#define JP2_PIO_DIR	 (0x04/sizeof(unsigned int))
#define JP2_PIO_DATA (0x00/sizeof(unsigned int))		//read data

// initialise
signed int JP2_initialise(unsigned int pio_base_address)
{
	//set local base address pointers
	jp2_pio_ptr = (unsigned int *) pio_base_address;
	//initialise JP2 PIO direction
	jp2_pio_ptr[JP2_PIO_DIR] = 0xFFFFFFFF;		// set as outputs
	jp2_pio_ptr[JP2_PIO_DATA] = 0xFFFFFFFF;		// set all pins to 1
	jp2_initialised = true;
	return JP2_SUCCESS;
}

// Check if driver intialised
bool JP2_isInitialised()
{
	return jp2_initialised;
}

// DHT11 Reset and Check
// return: 1 - valid response
// 			0- invalid/ no response
signed int JP2_RST()
{
	volatile unsigned int *jp2_ptr;
	unsigned int timer = 0;
	//sanity check
	if (!JP2_isInitialised()) return JP2_ERRORNOINIT;
	// give signal
	jp2_ptr[JP2_PIO_DATA] = ~PIN1;	// set output as 0, start resetting
	usleep(20000);					// maintain 0 for at least 18ms
	jp2_ptr[JP2_PIO_DATA] = PIN1;	// back to 1, input signal finished
	usleep(30);						// maintain for 20~40 us
	// read feedback
	jp2_ptr[JP2_PIO_DIR] = ~PIN1;	// set as input
	while(!jp2_ptr[JP2_PIO_DATA])
	{
		timer++;						// count every 1 us
		usleep(1);
	}
	if (timer > 88 || timer < 78) return 0;	// check 83 us low signal
	timer = 0;
	while(jp2_ptr[JP2_PIO_DATA])
	{
		timer++;
		usleep(1);
	}
	if (timer > 92 || timer < 82) return 0;	// check 87 us high siganl
	return 1;
}

// read one byte
// each bit data comes after a 54 us low signal
// bit 0: stay high for 23~27 us
// bit 1: stay high for 68~74 us
//
// when data begins do 40 us delay then check the signal and so on
// Ref:
//		DHT11 Manual, www.aosong.com
//		DHT11 Driver Code for STM32 (Chinese text), https://zhuanlan.zhihu.com/p/347904660
unsigned int JP2_readByte()
{
	volatile unsigned int *jp2_ptr;
	unsigned int i;
	unsigned int byte = 0;
	// sanity check takes place in JP2_RST()
	for (i = 0; i < 8; i++)
	{
		while(jp2_ptr[JP2_PIO_DATA]);	// wait for the previous data to pass
		while(!jp2_ptr[JP2_PIO_DATA]);	// wait for low signal to pass
		usleep(40);
		byte <<= 1;
		if (jp2_ptr[JP2_PIO_DATA]) byte |= 0x01;	// read input value
	}
	return byte;
}

// read all the current pin values
signed int JP2_readValue()
{
	volatile unsigned int *jp2_ptr;
	unsigned int KEY1 = 0x0000;
	unsigned int KEY2 = 0x0000;
	unsigned int KEY3 = 0x0000;
	unsigned int KEY4 = 0x0000;
	unsigned int KEY = 0x0000;
	//sanity check
	if (!JP2_isInitialised()) return JP2_ERRORNOINIT;
	//configure value register
	jp2_ptr = (unsigned int *) jp2_pio_ptr;
	//do the scanning, find out which key(s) pressed
	KEY1 = JP2_oneScan(PIN1);
	usleep(1000);				//there needs to be a time delay!
	KEY2 = JP2_oneScan(PIN2);
	usleep(1000);
	KEY3 = JP2_oneScan(PIN3);
	usleep(1000);
	KEY4 = JP2_oneScan(PIN4);
	usleep(1000);
	KEY = (KEY1 | KEY2 | KEY3 | KEY4);
	return KEY;
}

//scan only one pin
unsigned int JP2_oneScan(unsigned int pin)
{
	volatile unsigned int *jp2_ptr;
	unsigned int JP2_VALUE = 0xFFFFFFFF;
	unsigned int key = 0x0000;
	//configure register
	jp2_ptr = (unsigned int *) jp2_pio_ptr;
	//do the scanning
	jp2_ptr[JP2_PIO_DIR] = pin;
	//jp2_ptr[JP2_PIO_DATA] = pin;		// not needed
	JP2_VALUE = ~(jp2_ptr[JP2_PIO_DATA] | HALF_PIN);	// read inverted pin data, 1 is on, 0 is off
	jp2_ptr[JP2_PIO_DIR] = 0x00000000;

	switch (pin) {
	case PIN1: {	// 4th column
		if (JP2_VALUE & PIN5) key = key | 0x8;
		if (JP2_VALUE & PIN6) key = key | 0x80;
		if (JP2_VALUE & PIN7) key = key | 0x800;
		if (JP2_VALUE & PIN8) key = key | 0x8000;
	} break;
	case PIN2: {	// 3rd column
		if (JP2_VALUE & PIN5) key = key | 0x4;
		if (JP2_VALUE & PIN6) key = key | 0x40;
		if (JP2_VALUE & PIN7) key = key | 0x400;
		if (JP2_VALUE & PIN8) key = key | 0x4000;
	} break;
	case PIN3: {	// 2nd column
		if (JP2_VALUE & PIN5) key = key | 0x2;
		if (JP2_VALUE & PIN6) key = key | 0x20;
		if (JP2_VALUE & PIN7) key = key | 0x200;
		if (JP2_VALUE & PIN8) key = key | 0x2000;
	} break;
	case PIN4: {	// 1st column
		if (JP2_VALUE & PIN5) key = key | 0x1;
		if (JP2_VALUE & PIN6) key = key | 0x10;
		if (JP2_VALUE & PIN7) key = key | 0x100;
		if (JP2_VALUE & PIN8) key = key | 0x1000;
	} break;
	default: key = 0x0000;
	}

	return key;
}
