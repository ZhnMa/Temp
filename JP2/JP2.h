/*
 * Header file for GPIO1/JP2 driver, contains function declarations
 * 	For: Individual Project
 * 	By: Zhaonan Ma [el20z2m] SID: 201397999
 *
 */

//Include required header files
#include <stdbool.h> //Boolean variable type "bool" and "true"/"false" constants.

//status codes
#define JP2_SUCCESS       0
#define JP2_ERRORNOINIT  -1

// bit map, needed in main.c, so include here in the header file
#define PIN1 (1 << 9)		// DHT11 data input
	// potential extra inputs
#define PIN2 (1 << 11)
#define PIN3 (1 << 13)
#define PIN4 (1 << 14)
#define PIN_MASK (PIN1 | PIN2 | PIN3 | PIN4)

//function declarations
// initialise
signed int JP2_initialise(unsigned int pio_base_address);

// Check if driver intialised
bool JP2_isInitialised(void);

// reset and check the sensor
signed int JP2_RST(unsigned int pin);

// read one byte data
unsigned int JP2_readByte(unsigned int pin);

//
// read real time data
//
unsigned int JP2_rtData(unsigned int pin);

//
// Scan all the pins
// get valid data further process
//
// unsigned int bits: define how many bits of data to return
// 						1 - 16 bits
//						2 - 32 bits
//
//unsigned int JP2_fullScan(unsigned short bits);

//
// function designed to deal with 16 bit output only
//		must not be called under other conditions
// unsigned int pin: define input pin
//
unsigned short JP2_16Bits(unsigned int pin, bool deci);
