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

// PIC16F628A Configuration Bit Settings

#include "config_628A.h"
#include "atraso.h"
#include "display7s.h"
#include "eeprom.h"

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

int counter1 = 0; // Variável de contagem auxiliar
int sobeOuDesce = 0;
int dutyCicle = 500;
int dutyCicleEscolhido = 500;
int display = 8;

void __interrupt() IRS(void) // Vetor de interrupção 
{
    if (INTCONbits.T0IF == 1) // Houve o estouro do TIMER0?
    {
        counter1++; // Incrementa o counter
        //RA0 = ~RA0;
        TMR0 = 0x06; // Reinicia o registrador TMR0

        T0IF = 0x00; // Limpa a flag para a próxima interrupção
    }
} //end interrupt

void main() {
    OPTION_REG = 0x81; //Desabilita os resistores de pull-up internos pagina 24
    //Configura o prescaler para 1:4 associado ao TMR0
    // pagina 26
    GIE = 0x01; //Habilita a interrupção global
    PEIE = 0x01; //Habilita a interrupção por periféricos
    T0IE = 0x01; //Habilita a interrupção por estouro do TMR0

    TMR0 = 0x06; //Inicia a contagem em 0

    TRISB = 0X00;
    TRISA0 = 0x00; //Configura o RB0 como saída digital
    RA0 = 1; //Inicia RB0 em LOW

    while (1) {
        if (RA1 == 0) {
            if (dutyCicleEscolhido > 938) dutyCicleEscolhido = 1000;
            else dutyCicleEscolhido = dutyCicleEscolhido + 62;
            if (display <= 1) display = 0;
            else display--;
            while (RA1 == 0);
        }
        
        if (RA2 == 0) {
            if (dutyCicleEscolhido < 62) dutyCicleEscolhido = 0;
            else dutyCicleEscolhido = dutyCicleEscolhido - 62;
            if (display >= 14) display = 15;
            else display++;
            while (RA2 == 0);
        }
        
        PORTB = display7s(display) | 0x10;
        atraso_ms(5);
        
        if (counter1 >= dutyCicle) // 1us * 4 * 250 * 500 = 500ms
        {
            if (sobeOuDesce) {
                sobeOuDesce = 0;
                dutyCicle = 1000 - dutyCicleEscolhido;
            } else {
                sobeOuDesce = 1;
                dutyCicle = dutyCicleEscolhido;
            }
            
            RA0 = ~RA0;
            counter1 = 0;
        }
    } //end while
}
