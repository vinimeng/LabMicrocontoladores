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

void main() {
    unsigned char i, tmp[2];

    CMCON=0x07; 
    TRISA=0x30;
    TRISB=0xE7;

    PORTA=0xFE;
    PORTB=0x00;

    lcd_init();
    i2c_init();
    
    lcd_cmd(L_CLR); // limpa LCD
    lcd_cmd(L_L1); // posiciona cursor no inicio da linha 1
    lcd_str("Dig. 4 numeros:"); // imprime string
    lcd_cmd(L_L2); // posiciona cursor no inicio da linha 2
    
    i = 0;
    
    tmp[1] = '\0'; // coloca null terminator na string (array de char)
    
    while (i < 4) {
        // mais 0x30 porque do 0x00 ao 0x30 são comandos e simbolos na tabela ASCII
        // tc_tecla(0) para esperar infinitamente pelo comando
        // o parâmetro é o tempo de espera
        tmp[0] = tc_tecla(0) + 0x30; 
        
        if (tmp[0] >= '0' && tmp[0] <= '9') {
            lcd_str(tmp);
            e2pext_w(i, tmp[0]); // Salva na memória EEPROM | posição, char
            i++;
        }
    }
    
    lcd_cmd(L_CLR);
    lcd_cmd(L_L1 + 6);
    i = 0;

    while (i < 4) {
        tmp[0] = (char) e2pext_r(i); // Lê na memória EEPROM | posição
        lcd_str(tmp);
        i++;
    }
    
    while(1);
}
