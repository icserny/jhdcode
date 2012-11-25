// ad10pwm3.c - PWM controlled by input voltage using ADC10
// ADC10 def refs (VSS, VCC), int clock, auto disabled between readings
// Repeated conversions initiated by OUT0 in toggle mode
// PWM updated by data transfer controller (DTC) so CPU not needed!
// MSP430 Launchpad board (MSP430G2231, MSP430G2452 or MSP430G2553)
// Analog potmeter on P1.5 LED active high on P1.6 driven from TA0.1
// Default DCO, no crystal, no ACLK, power from FET
// Written by J H Davies, 2007-08-30; IAR Kickstart version 3.42A
// Launchpad adaptation by I Cserny, 2012-11-23; IAR version 5.51
//----------------------------------------------------------------------
#include <msp430.h>                    // Header file for this device

void main (void)
{
    WDTCTL = WDTPW | WDTHOLD;          // Stop watchdog
    P1OUT = 0;                         // Drive unused pins low
    P1DIR = ~(BIT2|BIT3|BIT5);         // Only RXD, SW2 and AN5 are inputs
    P1SEL |= BIT6;                     // Configure P1.6 as TA0.1 output
//-- Enable P1.3 inner pullup ---------
    P1OUT |= BIT3;                     // Pull up (not down)
    P1REN |= BIT3;                     // Enable inner pullup/pulldown for P1.3
//-- Timer_A for PWM at 125Hz on OUT1, SMCLK/8, Up mode
    TACCR0 = BITA - 1;                 // Upper limit to match ADC10MEM (1023)
    TACCTL0 = OUTMOD_4;                // Toggle OUT0 to stimulate ADC10
    TACCR1 = BIT9;                     // About 50% to start PWM (512)
    TACCTL1 = OUTMOD_7;                // Reset/set for positive PWM
    TACTL = TASSEL_2|ID_3|MC_1|TACLR;  // SMCLK/8, Up mode, clear
//-- ADC on, refs VCC, VSS, sample 4 cycles, int ref off, no interrupts
    ADC10CTL0 = SREF_0 | ADC10SHT_0 | ADC10ON;
//-- Input channel 5, start on rising edge of OUT0, no clock division,
//   internal ADC clock, single channel repeated conversions
    ADC10CTL1 = INCH_5 | SHS_2 | ADC10DIV_0 | ADC10SSEL_0 | CONSEQ_2;
    ADC10AE0 = BIT5;                   // Enable analog input on AN5/P1.5
//-- Data transfer controller: copy one result continuously to TACCR1
    ADC10DTC0 = ADC10CT;               // Continuous transfers, one block
    ADC10DTC1 = 1;                     // Single target address in memory
    ADC10SA = (unsigned short) &TACCR1;// (Starting) target address
    ADC10CTL0 |= ENC;                  // Enable conversions
    for (;;) {                         // Loop forever taking measurements
        __low_power_mode_0();          // CPU no longer needed,
    }                                  // not even for interrupts!
}
