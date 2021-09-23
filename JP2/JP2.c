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

// bit map
#define PIN1 (1 << 9)		// DHT11 data input
	// potential extra inputs
#define PIN2 (1 << 11)
#define PIN3 (1 << 12)
#define PIN4 (1 << 13)
#define PIN_MASK (PIN1 | PIN2 | PIN3 | PIN4)

// address offsets
#define JP2_PIO_DIR	 (0x04/sizeof(unsigned int))
#define JP2_PIO_DATA (0x00/sizeof(unsigned int))		//read data

// initialise
signed int JP2_initialise(unsigned int pio_base_address)
{
	//set local base address pointers
	jp2_pio_ptr = (unsigned int *) pio_base_address;
	//initialise JP2 PIO direction
	//jp2_pio_ptr[JP2_PIO_DIR] = 0xFFFFFFFF;		// set as outputs
	jp2_initialised = true;
	return JP2_SUCCESS;
}

// Check if driver intialised
bool JP2_isInitialised()
{
	return jp2_initialised;
}

//
// DHT11 Reset and Check
// return:
// 			0- invalid/ no response
//			16 bit data - defines which pins are valid
// Pin-scanning is implemented for multiple inputs
//
unsigned int JP2_RST()
{
	volatile unsigned int *jp2_ptr;
	unsigned int timer = 0;
	unsigned int pin = 0;
	// sanity check takes place in JP2_readData()
	// configure value register
	jp2_ptr = (unsigned int *) jp2_pio_ptr;
	// give signal
	jp2_ptr[JP2_PIO_DIR] = PIN_MASK;	// set potential pins as output, export 0
	usleep(20000);						// maintain 0 for at least 18ms
	// read feedback
	jp2_ptr[JP2_PIO_DIR] = ~PIN_MASK;	// set as input
	usleep(25);							// maintain for 20~40 us

	// check validity, exact pins
	while(!(jp2_ptr[JP2_PIO_DATA] & PIN_MASK))
	{
		timer++;							// count every 1 us
		usleep(1);
	}
	if (timer > 58 || timer < 8) return 0;	// check 83 us low signal
	timer = 0;
	pin = jp2_ptr[JP2_PIO_DATA] & 0xFFFF;	// store valid pins

	while((jp2_ptr[JP2_PIO_DATA] & PIN_MASK))
	{
		timer++;
		usleep(1);
	}

	if (timer > 42 || timer < 5) return 0;	// check 87 us high siganl
	return pin;
}

//
// read one byte
// each bit data comes after a 54 us low signal
// bit 0: stay high for 23~27 us
// bit 1: stay high for 68~74 us
//
// when data begins do 40 us delay then check the signal and so on
// Ref:
//		DHT11 Manual, www.aosong.com
//		DHT11 Driver Code for STM32 (Chinese text), https://zhuanlan.zhihu.com/p/347904660
//
unsigned int JP2_readByte()
{
	volatile unsigned int *jp2_ptr;
	unsigned int i;
	unsigned int byte = 0;
	// sanity check takes place in JP2_readData()
	//configure value register
	jp2_ptr = (unsigned int *) jp2_pio_ptr;
	// ToDO: read data from other pins too
	for (i = 0; i < 8; i++)
	{
		while((jp2_ptr[JP2_PIO_DATA] & PIN1));	// wait for the previous data to pass
		while(!(jp2_ptr[JP2_PIO_DATA] & PIN1));	// wait for low signal to pass
		usleep(40);
		byte <<= 1;
		if ((jp2_ptr[JP2_PIO_DATA] & PIN1)) byte |= 0x1;	// read input value
	}
	return byte;
}

//
// read all 40 bit data
// the only function that gets called, sanity check takes place
// return: 1 - valid, 0 - invalid
//
unsigned int JP2_readData()
{
	unsigned int Humi = 0;
	unsigned int Temp = 0;
	unsigned int i;
	unsigned int buff[5];	// store data of 5 bytes
	//sanity check
	if (!JP2_isInitialised()) return JP2_ERRORNOINIT;
	usleep(2000000);		// make sure 2 seconds has passed
	if (JP2_RST())			// check input validity
	{
		for (i = 0; i < 5; i++)	buff[i] = JP2_readByte();	// read 5 bytes, 40 bits
		// validation
		if (buff[0] + buff[1] + buff[2] + buff[3] == buff[4])
		{
			Humi = buff[0];
			Temp = buff[2];
		}
	}
	else
	{
		Humi = 0xFF;
		Temp = 0xFF;
	}
	//jp2_ptr[JP2_PIO_DIR] = PIN1;	// release bus, GPIO is output again
	//jp2_ptr[JP2_PIO_DATA] = PIN1;
	return (Humi << 8) + Temp;
}

//
// read real time data
// DHT11 only returns the previous data, current data needs to be read 2s later
// a 2-second delay is applied
//
unsigned int JP2_rtData()
{
	// stimulate sensor twice
	if (JP2_RST()) usleep(2000000);	// delay 2 seconds
	return JP2_readData();
}
