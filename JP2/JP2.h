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


//function declarations
// initialise
signed int JP2_initialise(unsigned int pio_base_address);

// Check if driver intialised
bool JP2_isInitialised(void);

// reset and check the sensor
signed int JP2_RST(void);

// read one byte data
unsigned int JP2_readByte(void);

// read 40 bit data
unsigned int JP2_readData(void);

//
// read real time data
//
unsigned int JP2_rtData(void);
