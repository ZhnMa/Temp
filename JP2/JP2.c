/*
 * 	Source file for controlling General Purpose I/O Expansion
 * 		For: Individual Project, DHT sensor
 * 		By:  Zhaonan Ma el20z2m@leeds.ac.uk
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

// address offsets
#define JP2_PIO_DIR	 (0x04/sizeof(unsigned int))
#define JP2_PIO_DATA (0x00/sizeof(unsigned int))		//read data

// initialise
signed int JP2_initialise(unsigned int pio_base_address)
{
	//set local base address pointers
	jp2_pio_ptr = (unsigned int *) pio_base_address;
	//initialise JP2 PIO direction
	jp2_pio_ptr[JP2_PIO_DIR] = 0x00000000;		// set as inputs
	jp2_initialised = true;
	return JP2_SUCCESS;
}

// Check if driver intialised
bool JP2_isInitialised()
{
	return jp2_initialised;
}

//
// DHT11 Reset and Check one input
// unsigned pin: input GPIO pin, check such pin whether there is a sensor
// return:
// 			0- invalid/ no response
//			1- valid
// Pin-scanning is implemented for multiple inputs
//
signed int JP2_RST(unsigned int pin)
{
	volatile unsigned int *jp2_ptr;
	unsigned int timer = 0;

	//sanity check
	if (!JP2_isInitialised()) return JP2_ERRORNOINIT;
	// configure value register
	jp2_ptr = (unsigned int *) jp2_pio_ptr;
	// give signal
	jp2_ptr[JP2_PIO_DIR] = pin;	// set potential pins as output, export 0
	usleep(20000);						// maintain 0 for at least 18ms
	// read feedback
	jp2_ptr[JP2_PIO_DIR] = 0x00000000;	// set as input
	usleep(25);							// maintain for 20~40 us

	// check validity, exact pins
	while(!(jp2_ptr[JP2_PIO_DATA] & pin))
	{
		timer++;							// count every 1 us
		usleep(1);
		if (timer > 100) break;
	}
	if (timer > 58 || timer < 8) return 0;	// check 83 us low signal
	timer = 0;
	while((jp2_ptr[JP2_PIO_DATA] & pin))
	{
		timer++;
		usleep(1);
		if (timer > 100) break;
	}

	if (timer > 42 || timer < 5) return 0;	// check 87 us high siganl
	return 1;
}

//
// read one byte
// each bit data comes after a 54 us low signal
// bit 0: stay high for 23~27 us
// bit 1: stay high for 68~74 us
//
// when data begins do 40 us delay then check the signal and so on
//
// multiple reads: do serial reading, one sensor after another
//
// Ref:
//		DHT11 Manual, www.aosong.com
//		DHT11 Driver Code for STM32 (Chinese text), https://zhuanlan.zhihu.com/p/347904660
//
unsigned int JP2_readByte(unsigned int pin)
{
	volatile unsigned int *jp2_ptr;
	unsigned int i,j;
	unsigned int byte = 0;
	unsigned int buff[5];	// store data of 5 bytes
	// sanity check takes place in JP2_RST()
	//configure value register
	jp2_ptr = (unsigned int *) jp2_pio_ptr;
	for (j = 0; j < 5; j++)
	{
		for (i = 0; i < 8; i++)
		{
			while((jp2_ptr[JP2_PIO_DATA] & pin));	// wait for the previous data to pass
			while(!(jp2_ptr[JP2_PIO_DATA] & pin));	// wait for low signal to pass
			usleep(40);
			byte <<= 1;
			if ((jp2_ptr[JP2_PIO_DATA] & pin)) byte |= 0x1;	// read input value
		}
		buff[j] = byte;
		byte = 0;
	}
	jp2_pio_ptr[JP2_PIO_DIR] = 0x00000000;
	if (buff[0] + buff[1] + buff[2] + buff[3] == buff[4]) {
	return (buff[0] << 24) + (buff[1] << 16) + (buff[2] << 8) + buff[3];}
	else return 0;
}

//
// read real time data
// DHT11 only returns the previous data, current data needs to be read 2s later
// a 2-second delay is applied
//
//unsigned int JP2_rtData(unsigned int pin)
//{
//	// stimulate sensor twice
//	if (JP2_RST(pin)) usleep(2000000);	// delay 2 seconds
//	return JP2_readData(pin);
//}

//
// function designed to deal with 16 bit output only
//		must not be called under other conditions
// unsigned int pin: define input pin
//
//unsigned short JP2_16Bits(unsigned int pin, bool deci)
//{
//	unsigned int data = JP2_readByte(pin);
//	if (deci) {
//		if (PRIO) data >>= 16;	// return only temperature with decimals under PRIORITY
//		else return 0xFFFFFFFF;	// no priority allowed, out of range
//	}
//	else {
//		// process, leave 16 bit valid data
//		data &= 0xFF00FF00;
//		data >>= 8;
//		data += (data >> 8);
//	}
//	return data;
//}

//
// Scan all the pins
// get valid data further process
//
// unsigned short bits: define how many bits of data to return
// 						1 - 16 bits
//						2 - 32 bits
//
// bool deci: define whether the decimals are needed
//						1 - yes
//						0 - no
//
//unsigned int JP2_fullScan(unsigned short bits, bool deci)
//{
//	unsigned int data = 0;
//	unsigned short validPin = 0;
//	if (bits == 1)	// only need 16 bit data
//	{
//		// scan each pin to check valid correspond
//		// ToDo: pretty shit, make sure multiple valid inputs work
//		if (JP2_RST(PIN1)) {
//			if (!validPin) {
//				data = JP2_16Bits(PIN1, deci);
//				validPin++;
//			} else {
//
//			}
//		}
//		else if (JP2_RST(PIN2)) data = JP2_16Bits(PIN2, deci);
//		else if (JP2_RST(PIN3)) data = JP2_16Bits(PIN3, deci);
//		else if (JP2_RST(PIN4)) data = JP2_16Bits(PIN4, deci);
//
//	}
//}

