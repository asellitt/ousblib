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

#include "openusbio.h"


/**
 * Displays a character in binary on the leds
 *    char num : The character to display
 */
void ledDisp(
      char num
) 
{
   char ddr;

   // back up port b direction register
   ddr = DDRB;

   // display number
   DDRB = FULL_OUTPUT;
   PORTB = num;

   // restore port b direction register
   DDRB = ddr;
}


/**
 * Performs a single LED chaser iteration.
 *    int speed : the delay between light seqences
 */
void ledChase(
      int speed
) 
{
   int i;
   int lights = 0x01;
   int dir = TRUE;
   char ddr;

   //back up port b direction register
   ddr = DDRB;

   //lights set to output
   DDRB = FULL_OUTPUT;

   for(i = 0; i < 2 * LEDS - 1; i++) 
   {
      //light up LEDS
      PORTB = lights;

      //determine which LED to light next
      if(dir)
      {
         lights <<= 1;
      }
      else
      {
         lights >>= 1;
      }
      
      //check overflow
      if(lights == 0x01 || lights == 0x80)
      {
         dir = !dir;
      }
      
      //visible delay
      _delay_ms(speed);
   }

   //restore port b direction register
   DDRB = ddr;
}


/**
 * Performs a single LED chaser iteration.
 *    int speed : the delay between light seqences
 */
void ledCycle(
      int speed
) 
{
   int i;
   int lights = 0x01;
   int dir = TRUE;
   char ddr;

   //back up port b direction register
   ddr = DDRB;

   //lights set to output
   DDRB = FULL_OUTPUT;

   for(i = 0; i < 2 * LEDS - 1; i++) 
   {
      //light up LEDS
      PORTB = lights;

      //determine which LED to light next
      if(dir) 
      {
         lights <<= 1;
         lights += 1;
      }
      else 
      {
         lights >>= 1;
      }

      //check overflow
      if(lights == 0x01 || lights == 0xFF)
      {
         dir = !dir;
      }

      //visible delay
      _delay_ms(speed);
   }

   //restore port b direction register
   DDRB = ddr;
}


/**
 * Measures the time of a high pulse of a PWM signal in micro secconds.
 * Can measure a maximum high pulse of 65,536 micro seconds.
 * WARNING: The signal MUST be PWM, else it will cause the program to lock
 *    Loc pwm : the pwm pin location
 */
uint16_t highTime(
      Loc pwm
) 
{
   uint16_t time = 0;

   // wait for leading edge
   while(BIT_SET(*(pwm.reg), pwm.pin))
   {
      ;
   }
   
   while(BIT_CLEAR(*(pwm.reg), pwm.pin))
   {
      ;
   }
   
   // sample signal
   while(BIT_SET(*(pwm.reg), pwm.pin)) 
   {
      _delay_us(10);
      time++;
   }

   // return time in microseconds
   return time * 10;
}


/**
 * Measures the time of a low pulse of a PWM signal
 * Can measure a maximum high pulse of 65,536 micro seconds.
 * WARNING: The signal MUST be PWM, else it will cause the program to lock
 *    Loc pwm : the pwm pin location
 */
uint16_t lowTime(
      Loc pwm
) 
{
   uint16_t time = 0;

   // wait for falling edge
   while(BIT_CLEAR(*(pwm.reg), pwm.pin))
   {
      ;
   }
   
   while(BIT_SET(*(pwm.reg), pwm.pin))
   {
      ;
   }
   
   // sample signal
   while(BIT_CLEAR(*(pwm.reg), pwm.pin)) 
   {
      _delay_us(10);
      time++;
   }

   // return time in microseconds
   return time * 10;
}


/**
 * Measures the duty cycle of a PWM signal
 * WARNING: The signal MUST be PWM, else it will cause the program to lock
 *    Loc pwm : the pwm pin location
 */
uint16_t dutyCycle(
      Loc pwm
) 
{
   uint16_t pulse = 0;
   uint16_t period = 0;

   // wait for leading edge
   while(BIT_SET(*(pwm.reg), pwm.pin))
   {
      ;
   }
   while(BIT_CLEAR(*(pwm.reg), pwm.pin))
   {
      ;
   }
   
   // sample pulse
   while(BIT_SET(*(pwm.reg), pwm.pin)) 
   {
      _delay_us(10);
      pulse++;
   }

   // sample signal
   while(BIT_CLEAR(*(pwm.reg), pwm.pin)) 
   {
      _delay_us(10);
      period++;
   }

   // return duty cycle
   return (100 * pulse)/(pulse + period);
}


/**
 * Generates a single clock tick on the specified pin of the specified port
 *    Loc clk : the clock pin location
 *    uint16_t period : the period at which the clock signal is switched
 */
void clock(
      Loc clk, 
      uint16_t period
) 
{
   int i;

   SET_BIT(*(clk.reg), clk.pin);
   
   for(i = 0; i < period/2; i++)
   {
      asm("nop");
   }

   CLEAR_BIT(*(clk.reg), clk.pin);
   
   for(i = 0; i < period/2; i++)
   {
      asm("nop");
   }
}


/**
 * Shifts out a single 16 bit number on the specified pin of the specified port
 *    Loc shift : the shift pin location
 *    Loc clk : the clock pin location
 *    uint16_t period : the period at which the clock signal is switched
 *    uint16_t data : the data to shift out
 *    uint8_t length : the length of the significant part of the data
 */
void shiftout
(
       Loc shift,
       Loc clk, 
       uint16_t period,
       uint16_t data,
       uint8_t length
)
{
   int i;
   int mask = 1 << (length - 1);

   //shift out data
   for(i = 0; i < length; i++)
   {
      if(data & mask)
      {
         SET_BIT(*(shift.reg), shift.pin);
      }
      else
      {
         CLEAR_BIT(*(shift.reg), shift.pin);
      }
      
      mask >>= 1;
      clock(clk, period);
   }
}


/**
 * Shifts in a single 16 bit number (maximum length) on the specified pin of
 * the specified port
 *    Loc shift : the shift pin location
 *    Loc clk : the clock pin location
 *    uint16_t period : the period at which the clock signal is switched
 *    uint8_t length : the length of the significant part of the data
 */
uint16_t shiftin(
      Loc shift, 
      Loc clk, 
      uint16_t period, 
      uint8_t length
)
{
   int i, data;

   data = 0;
   for(i = 0; i < length; i++) 
   {
      data <<= 1;
      clock(clk, period);
      data += (*(shift.reg) & (1<<shift.pin));
   }

   return data;
}


/**
 * Initialises the ADC converter with the following options:
 *    Prescaler divider : 128 (approx 94kHz)
 *    Reference selection : AVCC
 */
void initADC(void) 
{
   ADCSRA = (1<<ADEN) | (1<<ADSC) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
   ADMUX = (1<<REFS0);
}


/**
 * Converts the voltage on the selected channel to a digital representation
 *    unit8_t channel : the channel to convert
 */
uint16_t readADC(
      uint8_t channel
) 
{
   //check that channel is valid
   if(channel >= SIZEOF_PORT)
   {
      return 0xFFFF;
   }
   
   ADMUX = 0x1F & 0x00;
   ADCSRA |= (1<<ADSC);
   asm("nop");
   
   return ADC;
}


/**
 * Generates a stepped sine wave output on the given port.
 * NOTE: Expects an R2R ladder connected to the port to give proper results
 *    uint8_t* port : the port on which the R2R ladder is connected
 *    int period : the period (in milliseconds) of the sine wave to generate
 */
void steppedSine(
      volatile uint8_t* port, 
      int period
) 
{
   int i;
   uint8_t sine = 0x00;
   int dir = TRUE;

   // calculate the slice of time allocated to each voltage step
   int timeslice = period / (2 * SIZEOF_PORT);

   for(i = 0; i < 2 * SIZEOF_PORT; i++)
   {
      // output stepped voltage and hold for time slice
      *port = sine;
      _delay_us(timeslice);

      // calculate next voltage level
      if(dir) 
      {
         sine <<= 1;
         sine += 1;
      }
      else 
      {
         sine >>= 1;
      }

      // if time to reverse level direction
      if(sine == 0xFF)
      {
         // output midlevel voltage and hold
         *port = sine;
         _delay_us(timeslice);
         dir = !dir;
      }
   }

   // output final level voltage
   *port = sine;
   _delay_us(timeslice);
}