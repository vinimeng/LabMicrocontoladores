// Vinícius Meng - 020583

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

void digiteSenha() {
    lcd_cmd(L_CLR);
    lcd_cmd(L_L1 + 2);
    lcd_str("DIGITE SENHA");
}

void tenteNovamente() {
    lcd_cmd(L_CLR);
    lcd_cmd(L_L1);
    lcd_str("ERRADO");
    lcd_cmd(L_L2 + 7);
    lcd_str("TENTE DNV");
}

void bloqueado() {
    lcd_cmd(L_CLR);
    lcd_cmd(L_L1 + 1);
    lcd_str("BLOQUEADO");
    lcd_cmd(L_L2 + 1);
    lcd_str("ERROU D+");
    PORTA ^= 0x40;
    PORTA ^= 0x80;
    RA1 ^= 1;
}

void acertou() {
    lcd_cmd(L_CLR);
    lcd_cmd(L_L1);
    lcd_str("ACERTOU");
    lcd_cmd(L_L2 + 7);
    lcd_str("MEU GURI");
    PORTA ^= 0x40;
    PORTA ^= 0x80;
    RA2 ^= 1;
}

void main() {
    unsigned char i, tmp;
    int primeiroDigito = 1;
    int segundoDigito = 2;
    int terceiroDigito = 3;
    int quartoDigito = 4;
    
    int primeiroDigitoDigitado = 0;
    int segundoDigitoDigitado = 0;
    int terceiroDigitoDigitado = 0;
    int quartoDigitoDigitado = 0;

    CMCON = 0x07;
    TRISA = 0x30;
    TRISB = 0xE7;

    PORTA = 0xFE;
    PORTB = 0x00;

    lcd_init();
    lcd_cmd(L_CLR);
    digiteSenha();
    
    tmp = 0;
    while (tmp < 4) {
        lcd_cmd(L_L2);
        
        i = 0;
        while (i < 4) {
            int digito = tc_tecla(0);
            
            if (i == 0) {
                primeiroDigitoDigitado = digito;
                lcd_str("*");
            } else if (i == 1) {
                segundoDigitoDigitado = digito;
                lcd_str("*");
            } else if (i == 2) {
                terceiroDigitoDigitado = digito;
                lcd_str("*");
            } else if (i == 3) {
                quartoDigitoDigitado = digito;
                lcd_str("*");
            }
            
            i++;
        }
        
        if (
            primeiroDigitoDigitado == primeiroDigito &&
            segundoDigitoDigitado == segundoDigito &&
            terceiroDigitoDigitado == terceiroDigito &&
            quartoDigitoDigitado == quartoDigito
        ) {
            acertou();
            while (1);
        } else {
            tenteNovamente();
            primeiroDigitoDigitado = 0;
            segundoDigitoDigitado = 0;
            terceiroDigitoDigitado = 0;
            quartoDigitoDigitado = 0;
        }
        
        tmp++;
    }
    
    bloqueado();

    while (1);
}
