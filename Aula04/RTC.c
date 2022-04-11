// Vinícius Meng - 0250583

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

#include "config_628A.h"

#include "atraso.h"
#include "lcd.h"
#include "teclado.h"
#include "i2c.h"
#include "eeprom_ext.h"
#include "rtc.h"
#include "serial.h"

#include <string.h>

void main() {
    unsigned int i, j, tmp, asteriscoPressionado, hashtagPressionado;
    unsigned char dataPressionada[9];
    unsigned char tempoPressionado[9];
    unsigned char buffer[9];
    unsigned char buffer2[9];

    CMCON = 0x07;
    TRISA = 0x30;
    TRISB = 0xE7;

    PORTA = 0xFE;
    PORTB = 0x00;

    lcd_init();
    i2c_init();
    
    i = 0;
    tmp = 0;
    asteriscoPressionado = 0;
    hashtagPressionado = 0;
    lcd_cmd(L_CLR);
    
    while (!hashtagPressionado) {
        rtc_r();
        lcd_cmd(L_L1);
        lcd_str((char *) date);
        lcd_cmd(L_L2);
        lcd_str((char *) time);
        
        atraso_ms(500);
        tmp = (int) tc_tecla(4000);
        atraso_ms(500);
        
        if (tmp == 10) {
            rtc_r();
            strcpy(dataPressionada, date);
            strcpy(tempoPressionado, time);
            
            while (i < 9) {
                e2pext_w(i, dataPressionada[i]);
                i++;
            }
            
            i = 0;
            
            while (i < 9) {
                e2pext_w(i + 9, tempoPressionado[i]);
                i++;
            }
            
            asteriscoPressionado = 1;
        }
        
        if (tmp == 12 && asteriscoPressionado) {
            hashtagPressionado = 1;
        }
    }
    
    i = 0;
    lcd_cmd(L_CLR);
    lcd_cmd(L_L1);
    
    while (i < 9) {
        char m = (char) e2pext_r(i);
        buffer[i] = m;
        i++;
    }
    lcd_str(buffer);
    lcd_cmd(L_L2);
    
    i = 0;
    
    while (i < 9) {
        char m = (char) e2pext_r(i + 9);
        buffer2[i] = m;
        i++;
    }
    lcd_str(buffer2);
    
    while (1);
}