/*
 * 3D212 Door Sensor
 *
 * File:   main.c
 * Author: shiftky
 * Created on January 8, 2014, 10:39 AM
 *
 * PIC 12F683   MPLAB X IDE XC8
 */

#include <stdio.h>
#include <stdlib.h>
#include <xc.h>

// configuration bit setting
#pragma config FOSC = INTOSCIO  // Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA4/OSC2/CLKOUT pin, I/O function on RA5/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = ON       // Power-up Timer Enable bit (PWRT enabled)
#pragma config MCLRE = OFF      // MCLR Pin Function Select bit (MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = ON       // Brown Out Detect (BOR enabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)

// clock setting
#define _XTAL_FREQ  8000000

// pin define
#define LED_R   GP5
#define LED_G   GP4
#define LIM_SW  GP3
#define BZ      GP2
#define P_SW_R  GP1
#define P_SW_G  GP0

// params
#define TIMER_INIT      0xBC
#define LIM_SW_WAIT     500
#define P_SW_WAIT       500
#define MONITOR_TIME    10000
#define PAUSE_TIME      300000
#define ALART_INTERVAL  400

// Function prototype
void init(void);
void beep1(void);
void beep2(void);
void alart(void);
void clear_cnt(void);

// global variables
int mode, psw_r, psw_g, lim_sw;
unsigned long cnt, cnt_psw_r, cnt_psw_g, cnt_lim_sw;

void init() {
    OSCCON = 0x70;
    ANSEL = 0x00;
    CMCON0 = 0x07;
    TRISIO = 0x0B;

    LED_R = LED_G = BZ = 0;
    mode = 0;
    psw_r = psw_g = lim_sw = 0;
    cnt = cnt_psw_r = cnt_psw_g = cnt_lim_sw = 0;

    TMR0 = TIMER_INIT;
    OPTION_REG = 0x83;
    T0IF = 0;
    T0IE = 1;
    GIE = 1;
}

int main(void) {
    init();

    while(1) {
        switch (mode) {
            case 0: // watch mode
                LED_R = 0;
                LED_G = 0;
                BZ = 0;
                if ( LIM_SW == 1 && cnt_lim_sw >= LIM_SW_WAIT ) {
                    clear_cnt();
                    mode = 1;
                }
                break;

            case 1: // monitor mode
                LED_R = 1;
                LED_G = 0;
                BZ = 0;
                if ( LIM_SW == 0 && cnt_lim_sw >= LIM_SW_WAIT ) {
                    beep1();
                    clear_cnt();
                    mode = 0;
                } else  if ( cnt >= MONITOR_TIME ) {
                    clear_cnt();
                    mode = 2;
                } else if ( P_SW_G == 0 && cnt_psw_g >= P_SW_WAIT ) {
                    beep2();
                    clear_cnt();
                    mode = 3;
                }
                break;

            case 2: // alart mode
                if ( LIM_SW == 0 && cnt_lim_sw >= LIM_SW_WAIT ) {
                    beep1();
                    clear_cnt();
                    mode = 0;
                } else if ( ( P_SW_R == 0 || P_SW_G == 0 )&& cnt_psw_r >= P_SW_WAIT ) {
                    beep2();
                    clear_cnt();
                    mode = 3;
                }
                else {
                    alart();
                }
                break;

            case 3: // pause mode
                LED_R = 1;
                LED_G = 1;
                BZ = 0;
                if ( LIM_SW == 0 && cnt_lim_sw >= LIM_SW_WAIT ) {
                    beep1();
                    clear_cnt();
                    mode = 0;
                }

                if ( cnt >= PAUSE_TIME ) {
                    clear_cnt();
                    mode = 2;
                }
                break;
        }
    }

    return (EXIT_SUCCESS);
}

void beep1(void) {
    BZ = 1;
    __delay_ms(10);
    BZ = 0;
}

void beep2(void) {
    int i;
    for (i=0; i<2; i++) {
        BZ = 1;
        __delay_ms(30);
        BZ = 0;
        __delay_ms(30);
    }
}

void alart(void) {
    if ( cnt > 0 && cnt < ALART_INTERVAL/2 ) {
        LED_R = 1;
        LED_G = 0;
        BZ = 1;
    } else if ( cnt >= ALART_INTERVAL/2 && cnt < ALART_INTERVAL ) {
        LED_R = 0;
        LED_G = 0;
        BZ = 0;
    } else if ( cnt >= ALART_INTERVAL ) {
        cnt = 0;
    }
}

void clear_cnt(void) {
    cnt_lim_sw = 0;
    cnt_psw_r = 0;
    cnt_psw_g = 0;
    cnt = 0;
}

static void interrupt intr(void) {
    if ( T0IF == 1 ) {
        TMR0 = TIMER_INIT;

        cnt++;
        cnt_psw_r++;
        cnt_psw_g++;
        cnt_lim_sw++;

        T0IF = 0;
    }
}