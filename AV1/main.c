// Vinícius Meng - 0250583
// PICGenios - PIC16F877A
// https://exploreembedded.com/wiki/PIC16f877a_Timer
// https://www.academia.edu/9801017/Interrup%C3%A7%C3%A3o_do_Timer_0_PIC16F877A

/* ########################################################################

   PICsim - PIC simulator http://sourceforge.net/projects/picsim/

   ########################################################################

   Copyright (c) : 2015  Luis Claudio Gambôa Lopes

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   For e-mail suggestions :  lcgamboa@yahoo.com
   ######################################################################## */

/* ----------------------------------------------------------------------- */

#include <xc.h>
#include"config_877A.h"
#include "atraso.h"
#include "lcd.h"
#include "display7s.h"
#include "i2c.h"
#include "eeprom_ext.h"
#include "rtc.h"
#include "serial.h"
#include "eeprom.h"
#include "adc.h"
#include "itoa.h"
#include "teclado.h"

char value = 0;
#define SBIT_PS1  5
#define SBIT_PS0  4

void interrupt timer_isr(void) { 
    if(TMR1IF==1)
    {
        value=~value;   // complement the value for blinking the LEDs
        TMR1H=0x0B;     // Load the time value(0xBDC) for 100ms delay
        TMR1L=0xDC;
        TMR1IF=0;       // Clear timer interrupt flag
    } 
}

void main() {
    TRISD=0x00;    //COnfigure PORTD as output to blink the LEDs

    T1CON = (1<<SBIT_PS1) | (1<<SBIT_PS0); // Timer0 with external freq and 8 as prescalar
    TMR1H=0x0B;     // Load the time value(0xBDC) for 100ms delay
    TMR1L=0xDC;       
    TMR1IE=1;       //Enable timer interrupt bit in PIE1 register
    GIE=1;          //Enable Global Interrupt
    PEIE=1;         //Enable the Peripheral Interrupt
    TMR1ON = 1;     //Start Timer1    

    while(1)
    {
        PORTD = value;
    }
}
