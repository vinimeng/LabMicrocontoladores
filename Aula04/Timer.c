// Vinícius Meng - 0250583 - Usar clock em 4MHz

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

/*
    1 ciclo de maquina = 1/(clock/4)
    1 ciclo de maquina = 1/(4Mhz/4) = 1us
    timer0 = [0 255] = 256
    prescaler = 1:4 
    T = 1 ciclo de maquina * prescaler * 256
    T = 1us * 4 * 256 = 1024 us
    tempo de troca de RB0 desejado 500ms
    tempo de troca de RB0 = T * 500 = 0.512 s
    inicializar timer0 = 6
    tempo de troca de RB0 = 1us * 4 * 250 * 500 = 0.500 s
 */

int counter1 = 0x00; // Variável de contagem auxiliar

void __interrupt() IRS(void) // Vetor de interrupção 
{
    if (INTCONbits.T0IF == 1) // Houve o estouro do TIMER0?
    {
        counter1++; // Incrementa o counter

        TMR0 = 0x06; // Reinicia o registrador TMR0

        T0IF = 0x00; // Limpa a flag para a próxima interrupção
    }
} //end interrupt

void main() {
    unsigned int min1;
    unsigned int min2;
    unsigned int min3;
    unsigned int mili;
    unsigned char i;
    unsigned char str[2];

    OPTION_REG = 0x81; //Desabilita os resistores de pull-up internos pagina 24
    //Configura o prescaler para 1:4 associado ao TMR0
    // pagina 26
    GIE = 0x01; //Habilita a interrupção global
    PEIE = 0x01; //Habilita a interrupção por periféricos
    T0IE = 0x01; //Habilita a interrupção por estouro do TMR0

    TMR0 = 0x06; //Inicia a contagem em 0

    CMCON = 0x07;
    TRISA = 0x30;
    TRISB = 0xE7;

    PORTA = 0xFE;
    PORTB = 0x00;

    RA1 = 1;
    RA2 ^= 1;
    PORTA ^= 0x40;
    PORTA ^= 0x80;

    lcd_init();

    i2c_init();

    i = 0;
    
    str[1] = '\0';

    while (i < 2) {
        if (i < 1) {
            min1 = (int) tc_tecla(0);
            str[0] = min1 + '0';
            lcd_str(str);
        } else {
            min2 = (int) tc_tecla(0);
            str[0] = min2 + '0';
            lcd_str(str);
        }

        i++;
    }

    min3 = (min1 * 10) + min2;
    mili = min3 * 60 * 1000;

    while (1) {
        if (counter1 >= mili) // 1us * 4 * 250 * 500 = 500ms
        {
            RA1 = ~RA1;

            counter1 = 0;
        }
    }
}
