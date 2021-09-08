/*
 * Header file for GPIO1/JP2 driver, contains function declarations
 * 	For: Mini-Project
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
bool Timer_isInitialised(void);

// read the current pin value
signed int JP2_readValue(void);

//scan only one pin
unsigned int JP2_oneScan(unsigned int pin);

//read one value
signed int JP2_readOneValue(void);
