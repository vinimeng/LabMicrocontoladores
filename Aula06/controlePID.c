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
/* Template source file generated by piklab */

#include <xc.h>

#include "config_877A.h"
#include "atraso.h"
#include "lcd.h"
#include "display7s.h"
#include "i2c.h"
#include "serial.h"
#include "eeprom.h"
#include "eeprom_ext.h"
#include "adc.h"
#include "itoa.h"

#define _XTAL_FREQ 4000000UL
#define BitSet(arg,bit) ((arg) |= (1<<bit))
#define BitClr(arg,bit) ((arg) &= ~(1<<bit)) 
#define BitFlp(arg,bit) ((arg) ^= (1<<bit)) 
#define BitTst(arg,bit) ((arg) & (1<<bit)) 

// Funções
int pid(int ideal, int atual, int kp, int ki, int kd);
void pwmInit();
void pwmFrequency(unsigned int freq);
void pwmSet1(unsigned char porcento);
void interrupt isrh(void);

// Variáveis
unsigned char cnt;
unsigned int t1cont;
bit ligado, btnRB0, btnRB1, btnRB2, btnRB3;

void main() {
    unsigned int temperaturaAtual;
    char temperaturaString[6];

    unsigned int setpoint = 37 * 10; // temperatura desejada (vezes 10 porque a parte fracionária é o último digito do número inteiro)
    int kp = 1;
    int ki = 0;
    int kd = 0;
    int pidvar = 0;
    int pwm = 0;

    TRISA = 0xFF;
    TRISB = 0x00;
    TRISC = 0x01;
    TRISD = 0x00;
    TRISE = 0x00;

    lcd_init();
    i2c_init();
    adc_init();

    ADCON1 = 0x02;

    TRISAbits.TRISA5 = 0;

    lcd_cmd(L_CLR);
    lcd_cmd(L_L1);
    lcd_str("REF   OFF  SAIDA");

    //PORTCbits.RC2 = 1;
    ligado = 0;

    while (1) {
        pwm = 0; //inicializa pwm
        pidvar = 0; //inicializa PID

        TRISB = 0x0F;

        if (PORTBbits.RB0 != 0) {
            btnRB0 = 1;
        }

        if (PORTBbits.RB1 != 0) {
            btnRB1 = 1;
        }

        if (PORTBbits.RB2 != 0) {
            btnRB2 = 1;
        }

        if (PORTBbits.RB3 != 0) {
            btnRB3 = 1;
        }

        if (PORTBbits.RB0 == 0 && btnRB0) {
            setpoint += 5;
            btnRB0 = 0;
        }

        if (PORTBbits.RB1 == 0 && btnRB1) {
            setpoint -= 5;
            btnRB1 = 0;
        }

        if (PORTBbits.RB2 == 0 && btnRB2) {
            ligado = 1;
            lcd_cmd(L_L1);
            lcd_str("REF   ON   SAIDA");
            btnRB2 = 0;
        }

        if (PORTBbits.RB3 == 0 && btnRB3) {
            ligado = 0;
            lcd_cmd(L_L1);
            lcd_str("REF   OFF  SAIDA");
            btnRB3 = 0;
        }

        lcd_cmd(L_L2);
        itoa(setpoint, temperaturaString);
        lcd_dat(temperaturaString[2]);
        lcd_dat(temperaturaString[3]);
        lcd_dat(',');
        lcd_dat(temperaturaString[4]);
        lcd_dat('C');

        temperaturaAtual = (((unsigned int) adc_amostra(0) * 10) / 8) - 150;
        lcd_cmd(L_L2 + 11);
        itoa(temperaturaAtual, temperaturaString);
        lcd_dat(temperaturaString[2]);
        lcd_dat(temperaturaString[3]);
        lcd_dat(',');
        lcd_dat(temperaturaString[4]);
        lcd_dat('C');

        pidvar = pid(setpoint, temperaturaAtual, kp, kd, ki); // guarda cálculo do PID

        pwm = ((pidvar) + 128) * 100 / 256; // converte valor de pwm para porcentagem

        if (pwm == 256) {
            pwm = 255;
        }

        if (pwm < 0) {
            pwm = pwm * (-1);
        }

        if (ligado) {
            if (temperaturaAtual >= setpoint) {
                BitClr(T2CON, 2);
                BitClr(PORTC, 5);
            } else if (temperaturaAtual <= setpoint) {
                pwmFrequency(1000); //frequência 2000 do pwm
                pwmSet1(pwm); //atualiza duty cycle do pwm
                BitSet(PORTC, 5);
            }
        } else {
            BitClr(T2CON, 2);
            BitClr(PORTC, 5);
        }
    }
}

/**
 * PID
 * 
 * @param ideal
 * @param atual
 * @param kp
 * @param ki
 * @param kd
 * @return 
 */
int pid(int ideal, int atual, int kp, int ki, int kd) {
    int PID;
    int proporcional;
    int derivativo;
    int integrativo;
    int ultima = 0;
    int erroaux = 0;
    int erro = 0;

    erroaux = (ideal - atual);

    if (erroaux < 0) {
        erro = erroaux * (-1);
    } else {
        erro = erroaux;
    }

    proporcional = erro * kp;
    integrativo += (erro * ki);
    derivativo = (ultima - atual) * kd;
    ultima = atual;
    PID = proporcional + integrativo + derivativo;

    //PID = PID / 4;
    //pwm = (PID + 128) * 100 / 256;

    return PID;
}

/**
 * pwmInit
 * 
 * inicializa pwm
 */
void pwmInit() {
    T2CON |= 0b00000011; //configura o prescale do timer 2 para 1:16
    BitSet(T2CON, 2); //Liga o timer 2

    CCP1CON |= 0b00001100; //configura CCP1 como um PWM
    //CCP2CON |= 0b00001100; //configura CCP2 como um PWM
}

/**
 * pwmFrequency
 * 
 * configura frequência do pwm
 * 
 * @param freq
 */
void pwmFrequency(unsigned int freq) {
    PR2 = (125000 / (freq)) - 1;
}

/**
 * pwmSet1
 * 
 * configura duty cycle do pwm
 * 
 * @param porcento
 */
void pwmSet1(unsigned char porcento) {
    // inicializa pwm
    pwmInit();

    // formula do duty cycle:
    // DC_porcento = V / ((PR2+1)?4;
    // V = DC/100 ? (PR2+1) ? 4
    // V = DC ? (PR2+1) /25

    unsigned int val = ((unsigned int) porcento) * (PR2 + 1);
    val = val / 25;
    // garante que tem apenas 10 bits
    val &= 0x03ff;
    // os 8 primeiros bits sao colocados no CCPR1L
    CCPR1L = val >> 2;
    // os ultimos dois são colocados na posição 5 e 4 do CCP1CON
    CCP1CON |= (val & 0x0003) << 4;
}

/**
 * High priority interrupt routine
 */
void interrupt isrh() {
    cnt--;

    if (!cnt) { //executada a cada 1 segundo
        T1CONbits.TMR1ON = 0;
        t1cont = (((unsigned int) TMR1H << 8) | (TMR1L)) / 7; //ventilador com 7
        cnt = 125;
        TMR1H = 0;
        TMR1L = 0;
        T1CONbits.TMR1ON = 1;
    }

    INTCONbits.T0IF = 0;
    TMR0 = 6;
}

