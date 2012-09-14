/**
 *
 *  OPEN-USB-IO Library Version 2.0
 *  Copyright (C) 2010  Anthony Sellitti
 *  
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>. 
 **/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "usbdrv/main.h"

/*
 * System constants
 */


/*
 * Useful constants
 */

// for use with boolean logic
#define TRUE        1
#define FALSE       0

// mainly used in the led chaser function
#define FULL_OUTPUT 0xFF
#define FULL_INPUT  0x00

// the number of LEDS on the board
#define SIZEOF_PORT 8
#define LEDS        8

// standard return values
#define ERROR       -1
#define SUCCESS     0
#define FAILURE     1


/*
 * Useful macros
 */

// set bit (at location pin) in register (location write)
#define SET_BIT(_write, _pin)   _write |= (1 << _pin)
#define SET_OUTPUT(_ddr, _pin)  _ddr |= (1 << _pin)

// clear bit (at location pin) in register (location write)
#define CLEAR_BIT(_write, _pin) _write &= ~(1 << _pin)
#define SET_INPUT(_ddr, _pin)   _ddr &= ~(1 << _pin)

// check if bit is set (at location pin) in register (location write)
#define BIT_SET(_read, _pin)    ((_read & (1 << _pin)) == (1 << _pin))

// check if bit is clear (at location pin) in register (location write)
#define BIT_CLEAR(_read, _pin)  ((_read & (1 << _pin)) == 0)

// calculate the absolute value of the number
#define ABS(n) if(n<1){n*=-1;}

/*
 * Structures
 */

typedef struct 
{
   volatile uint8_t* reg;
   uint8_t pin;
} Loc;

/*
 * Function Prototypes
 */

void ledDisp(char);
void ledChase(int);
void ledCycle(int);

uint16_t highTime(Loc);
uint16_t lowTime(Loc);
uint16_t dutyCycle(Loc);

void clock(Loc, uint16_t);
void shiftout(Loc, Loc, uint16_t, uint16_t, uint8_t);
uint16_t shiftin(Loc, Loc, uint16_t, uint8_t);

void initADC(void);
uint16_t readADC(uint8_t);

void steppedSine(volatile uint8_t*, int);
