// Vinicius Meng - 0250583
// PICGenios - PIC16F877A
// A ideia eh ser um joguinho estilo shooter
// Pode ir para baixo e para cima com a nave e atirar
// Deve ligar o buzzer na placa
// Clk deve ser de 20MHz
// LCD tem que ser 16x4

#include <xc.h>
#include <stdlib.h>

// = PIC16F877A =

#pragma config FOSC  = HS  // Oscillator Selection bits (HS oscillator)
#pragma config WDTE  = OFF // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = OFF // Brown-out Reset Enable bit (BOR disabled)
#pragma config LVP   = ON  // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3/PGM pin has PGM function; low-voltage programming enabled)
#pragma config CPD   = OFF // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT   = OFF // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP    = OFF // Flash Program Memory Code Protection bit (Code protection off)

// ============= 
// ==== LCD ====

#define LENA  PORTEbits.RE1
#define LDAT  PORTEbits.RE2
#define LPORT PORTD

#define L_ON	0x0F
#define L_OFF	0x08
#define L_CLR	0x01
#define L_L1	0x80
#define L_L2	0xC0
#define L_L3    L_L1 + 16
#define L_L4    L_L2 + 16
#define L_CR	0x0F		
#define L_NCR	0x0C	

#define L_CFG   0x38

// =============
// ==== I2C ====

#define ICLK PORTCbits.RC3
#define IDAT PORTCbits.RC4
#define TIDAT TRISCbits.TRISC4

// =============
// ==== GAME ===

#define PLAYER 62
#define BULLET 165
#define ENEMY  219

#define MENU 0
#define TUTORIAL 1
#define GAME 2
#define GAMEOVER 3

#define FRAMESBLINK 30
#define FRAMESBULLET 45
#define FRAMESENEMY 40

unsigned char grid[4][16] = {
    {PLAYER, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};
unsigned char gameState; // MENU = 0, TUTORIAL = 1, GAME = 2, GAMEOVER = 3
unsigned char framesBlink;
unsigned char blinkOnOff;
unsigned char btnRB0, btnRB1, btnRB2;
unsigned char playerY;
unsigned char framesBullet;
unsigned char framesEnemy;
unsigned char randomNumber;

unsigned char sleepICounter;
unsigned char sleepJCounter;
unsigned char drawICounter;
unsigned char drawJCounter;
unsigned char tickICounter;
unsigned char tickJCounter;

void i2cDelay() {
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
}

void i2cInit() {
    TIDAT = 0;
    ICLK = 1;
    IDAT = 1;
}

void i2cStart() {
    ICLK = 1;
    IDAT = 1;
    i2cDelay();
    IDAT = 0;
    i2cDelay();
}

void i2cStop() {
    ICLK = 1;
    IDAT = 0;
    i2cDelay();
    IDAT = 1;
    i2cDelay();
}

void i2cWb(unsigned char val) {
    unsigned char i;
    ICLK = 0;

    for (i = 0; i < 8; i++) {
        IDAT = ((val >> (7 - i)) & 0x01);
        ICLK = 1;
        i2cDelay();
        ICLK = 0;
    }

    IDAT = 1;
    i2cDelay();
    ICLK = 1;
    i2cDelay();
    ICLK = 0;
}

unsigned char i2cRb(unsigned char ack) {
    char i;
    unsigned char ret = 0;

    ICLK = 0;
    TIDAT = 1;
    IDAT = 1;

    for (i = 0; i < 8; i++) {
        ICLK = 1;
        i2cDelay();
        ret |= (IDAT << (7 - i));
        ICLK = 0;
    }

    TIDAT = 0;

    if (ack) {
        IDAT = 0;
    } else {
        IDAT = 1;
    }

    i2cDelay();
    ICLK = 1;
    i2cDelay();
    ICLK = 0;

    return ret;
}

unsigned char rtcReadSeconds() {
    unsigned char tmp;

    i2cStart();
    i2cWb(0xD0);
    i2cWb(0);

    i2cStart();
    i2cWb(0xD1);
    tmp = 0x7F & i2cRb(1);
    i2cStop();
    
    return tmp;
}

void sleep(unsigned char sleepTime) {
    for (sleepICounter = 0; sleepICounter < sleepTime; sleepICounter++) {
        for (sleepJCounter = 0; sleepJCounter < 200; sleepJCounter++) {
            asm("NOP");
            asm("NOP");
            asm("NOP");
            asm("NOP");
            asm("NOP");
            asm("NOP");
        }
    }
}

void lcdCommand(unsigned char value) {
    LENA = 1;
    LPORT = value;
    LDAT = 0;
    LENA = 0;
    LENA = 1;
}

void lcdChar(unsigned char value) {
    LENA = 1;
    LPORT = value;
    LDAT = 1;
    LENA = 0;
    LENA = 1;
}

void lcdString(const unsigned char* str) {
    unsigned char i = 0;

    while (str[i] != 0) {
        lcdChar(str[i]);
        i++;
    }
}

void lcdInit() {
    LENA = 0;
    LDAT = 0;
    LENA = 1;

    lcdCommand(L_CFG);
    lcdCommand(L_CFG);
    lcdCommand(L_CFG); // configura
    lcdCommand(L_OFF);
    lcdCommand(L_ON); // liga
    lcdCommand(L_CLR); // limpa
    lcdCommand(L_CFG); // configura
    lcdCommand(L_L1);
}

void drawMenu() {
    for (drawICounter = 0; drawICounter < 4; drawICounter++) {
        switch (drawICounter) {
            case 0:
                lcdCommand(L_L1);
                if (blinkOnOff) {
                    for (drawJCounter = 0; drawJCounter < 16; drawJCounter++) {
                        lcdChar(0);
                    }
                } else {
                    lcdString(" SPACE  SHOOTER ");
                }
                break;
            case 1:
                lcdCommand(L_L2);
                for (drawJCounter = 0; drawJCounter < 16; drawJCounter++) {
                    lcdChar(0);
                }
                break;
            case 2:
                lcdCommand(L_L3);
                lcdString(" JOGAR      RB1 ");
                break;
            case 3:
                lcdCommand(L_L4);
                lcdString(" TUTORIAL   RB0 ");
                break;
        }
    }
}

void drawTutorial() {
    for (drawICounter = 0; drawICounter < 4; drawICounter++) {
        switch (drawICounter) {
            case 0:
                lcdCommand(L_L1);
                lcdString("IR P/ CIMA   RB2");
                break;
            case 1:
                lcdCommand(L_L2);
                lcdString("ATIRAR       RB1");
                break;
            case 2:
                lcdCommand(L_L3);
                lcdString("IR P/ BAIXO  RB0");
                break;
            case 3:
                lcdCommand(L_L4);
                lcdString(" volta menu RB1 ");
                break;
        }
    }
}

void drawGameOver() {
    for (drawICounter = 0; drawICounter < 4; drawICounter++) {
        switch (drawICounter) {
            case 0:
                lcdCommand(L_L1);
                for (drawJCounter = 0; drawJCounter < 16; drawJCounter++) {
                    lcdChar(0);
                }
                break;
            case 1:
                lcdCommand(L_L2);
                if (blinkOnOff) {
                    for (drawJCounter = 0; drawJCounter < 16; drawJCounter++) {
                        lcdChar(0);
                    }
                } else {
                    lcdString("   GAME OVER!   ");
                }
                break;
            case 2:
                lcdCommand(L_L3);
                for (drawJCounter = 0; drawJCounter < 16; drawJCounter++) {
                    lcdChar(0);
                }
                break;
            case 3:
                lcdCommand(L_L4);
                lcdString(" IR AO MENU RB1 ");
                break;
        }
    }
}

void drawGame() {
    for (drawICounter = 0; drawICounter < 4; drawICounter++) {
        switch (drawICounter) {
            case 0:
                lcdCommand(L_L1);
                for (drawJCounter = 0; drawJCounter < 16; drawJCounter++) {
                    lcdChar(grid[drawICounter][drawJCounter]);
                }
                break;
            case 1:
                lcdCommand(L_L2);
                for (drawJCounter = 0; drawJCounter < 16; drawJCounter++) {
                    lcdChar(grid[drawICounter][drawJCounter]);
                }
                break;
            case 2:
                lcdCommand(L_L3);
                for (drawJCounter = 0; drawJCounter < 16; drawJCounter++) {
                    lcdChar(grid[drawICounter][drawJCounter]);
                }
                break;
            case 3:
                lcdCommand(L_L4);
                for (drawJCounter = 0; drawJCounter < 16; drawJCounter++) {
                    lcdChar(grid[drawICounter][drawJCounter]);
                }
                break;
        }
    }
}

void draw() {
    switch (gameState) {
        case MENU:
            drawMenu();
            break;
        case TUTORIAL:
            drawTutorial();
            break;
        case GAME:
            drawGame();
            break;
        case GAMEOVER:
            drawGameOver();
            break;
    }

    framesBlink++;

    if (framesBlink >= FRAMESBLINK) {
        blinkOnOff = ~blinkOnOff;
        framesBlink = 0;
    }
}

void tickPlayer() {
    if (PORTBbits.RB0 != 0) {
        btnRB0 = 1;
    }

    if (PORTBbits.RB1 != 0) {
        btnRB1 = 1;
    }

    if (PORTBbits.RB2 != 0) {
        btnRB2 = 1;
    }

    if (PORTBbits.RB0 == 0 && btnRB0) {
        if (playerY < 3) {
            grid[playerY][0] = 0;
            playerY++;
            if (grid[playerY][0] == ENEMY) {
                gameState = GAMEOVER;
            } else {
                grid[playerY][0] = PLAYER;
            }
        }
        btnRB0 = 0;
    }

    if (PORTBbits.RB1 == 0 && btnRB1) {
        grid[playerY][1] = BULLET;
        PORTCbits.RC1 = 1;
        sleep(33);
        PORTCbits.RC1 = 0;
        btnRB1 = 0;
    }

    if (PORTBbits.RB2 == 0 && btnRB2) {
        if (playerY > 0) {
            grid[playerY][0] = 0;
            playerY--;
            if (grid[playerY][0] == ENEMY) {
                gameState = GAMEOVER;
            } else {
                grid[playerY][0] = PLAYER;
            }
        }
        btnRB2 = 0;
    }
}

void tickBullet() {
    for (tickICounter = 0; tickICounter < 4; tickICounter++) {
        for (tickJCounter = 15; tickJCounter > 0; tickJCounter--) {
            if (grid[tickICounter][tickJCounter] == BULLET) {
                if (tickJCounter >= 15) {
                    grid[tickICounter][tickJCounter] = 0;
                } else {
                    grid[tickICounter][tickJCounter] = 0;
                    if (grid[tickICounter][tickJCounter + 1] == ENEMY) {
                        grid[tickICounter][tickJCounter + 1] = 0;
                    } else {
                        grid[tickICounter][tickJCounter + 1] = BULLET;
                    }
                }
            }
        }
    }
}

void tickEnemy() {
    for (tickICounter = 0; tickICounter < 4; tickICounter++) {
        if (grid[tickICounter][15] == 0) {
            grid[tickICounter][15] = ENEMY;
        }
        for (tickJCounter = 0; tickJCounter < 16; tickJCounter++) {
            if (grid[tickICounter][tickJCounter] == ENEMY) {
                randomNumber = rand() % 3;
                
                if (tickJCounter - randomNumber >= 0) {       
                    if (grid[tickICounter][tickJCounter - randomNumber] == PLAYER) {
                        gameState = GAMEOVER;
                    } else {
                        grid[tickICounter][tickJCounter - randomNumber] = ENEMY;
                    }
                }
            }
        }
    }
}

void tick() {
    switch (gameState) {
        case MENU:
            if (PORTBbits.RB0 != 0) {
                btnRB0 = 1;
            }

            if (PORTBbits.RB1 != 0) {
                btnRB1 = 1;
            }

            if (PORTBbits.RB0 == 0 && btnRB0) {
                gameState = TUTORIAL;
                PORTCbits.RC1 = 1;
                sleep(33);
                PORTCbits.RC1 = 0;
                btnRB0 = 0;
            }

            if (PORTBbits.RB1 == 0 && btnRB1) {
                gameState = GAME;
                PORTCbits.RC1 = 1;
                sleep(33);
                PORTCbits.RC1 = 0;
                btnRB1 = 0;
            }
            break;
        case TUTORIAL:
            if (PORTBbits.RB1 != 0) {
                btnRB1 = 1;
            }
            if (PORTBbits.RB1 == 0 && btnRB1) {
                gameState = MENU;
                PORTCbits.RC1 = 1;
                sleep(33);
                PORTCbits.RC1 = 0;
                btnRB1 = 0;
            }
            break;
        case GAME:
            if (framesEnemy >= FRAMESENEMY) {
                tickEnemy();
                framesEnemy = 0;
            }
            if (framesBullet >= FRAMESBULLET) {
                tickBullet();
                framesBullet = 0;
            }
            tickPlayer();

            framesBullet++;
            framesEnemy++;
            break;
        case GAMEOVER:
            for (tickICounter = 0; tickICounter < 4; tickICounter++) {
                for (tickJCounter = 0; tickJCounter < 16; tickJCounter++) {
                    grid[tickICounter][tickJCounter] = 0;
                }
            }

            grid[0][0] = PLAYER;

            if (PORTBbits.RB1 != 0) {
                btnRB1 = 1;
            }
            if (PORTBbits.RB1 == 0 && btnRB1) {
                gameState = MENU;
                PORTCbits.RC1 = 1;
                sleep(33);
                PORTCbits.RC1 = 0;
                btnRB1 = 0;
            }
            break;
    }
}

void main() {
    TRISB = 0x0F;
    TRISC = 0x01;
    TRISD = 0x00;
    TRISE = 0x00;

    i2cInit();
    lcdInit();
    lcdCommand(L_NCR);

    gameState = MENU;
    framesBlink = 0;
    blinkOnOff = 0;
    playerY = 0;
    framesBullet = 0;
    framesEnemy = 0;
    
    srand(rtcReadSeconds());

    while (1) {
        tick();
        draw();
        sleep(30);
    }
}
