// ad10tmp1.c - measure temperature in celsius using sensor in ADC10
// ADC10 and ref disabled between readings set by watchdog timer
// ADC10 internal 1.5V ref, internal clock/4, interrupt on finish
// MSP430 Launchpad board (MSP430G2231, MSP430G2452 or MSP430G2553)
// The green LED at P1.6 will be lit if temperature higher than 25C
// Default DCO, no crystal, ACLK from VLO, power from FET
// Originalal version: J H Davies, 2007-08-30; IAR Kickstart version 3.42A
// Modifications: I Cserny, 2012-11-23; IAR version 5.51
//----------------------------------------------------------------------
#include <io430.h>
#include <stdint.h>                    // Standard integer types

#define    LED      P1OUT_bit.P6       // Output bit for LED

void main (void)
{
    uint32_t temperature;              // Converted value of temperature

    BCSCTL3 = LFXT1S_2;                // Select ACLK from VLO (no crystal)
    WDTCTL = WDT_ADLY_1000;            // Watchdog as interval timer, ACLK/32768
    IE1_bit.WDTIE = 1;                 // Enable WDT interval timer interrupts
    P1OUT = 0;                         // Drive unused pins low
    P1DIR = ~(BIT2+BIT3);              // Only RXD and SW2 are inputs
//--- Enable P1.3 inner pullup --------
    P1OUT |= BIT3;                     // Pull up (not down)
    P1REN |= BIT3;                     // Enable inner pullup/pulldown for P1.3
    ADC10CTL0 = ADC10SHT_3             // Sampling for 64 ticks of ACD10OSC
                | ADC10ON              // Switch on ADC
                | SREF_1               // VR+ = VREF+ and VR- = AVSS
                | REFON                // Enable 1.5V inner reference
                | ADC10IE;             // Enable ADC interrupts
    ADC10CTL1 = INCH_10                // Select inner temperature sensor
                | SHS_0                // Trigger conversion by softvare (ADC10SC)
                | ADC10DIV_3           // Set ADC clock divider to 1:4
                | CONSEQ_0;            // Single-channel-single-conversion
    for (;;) {                         // Loop forever taking measurements
        __low_power_mode_3();          // Wait for WDT between measurements
        ADC10CTL0 |= (REFON|ADC10ON);  // Enable ADC10 and reference
        ADC10CTL0 |= (ENC|ADC10SC);    // Enable ADC10, start conversion
        __low_power_mode_3();          // Sleep during measurement
        ADC10CTL0_bit.ENC = 0;         // Disable further ADC10 conversions
        ADC10CTL0 &= ~(REFON|ADC10ON); // Disable ADC10 and reference
        temperature = ADC10MEM;        // Raw converted value
        temperature *= 420;            // Scaling factor (Vref/temp coeff)
        temperature += 512;            // Allow for rounding in division
        temperature /= 1024;           // Divide by range of ADC10
        temperature -= 278;            // Subtract offset to give celsius
        if (temperature > 25) {        // Is temperature above 25 celsius?
            LED = 1;                   // Yes: Turn LED on
        } else {
            LED = 0;                   // No: Turn LED off
        }
    }
}
//----------------------------------------------------------------------
// Interrupt service routine for watchdog after sleep: return to active
//----------------------------------------------------------------------
#pragma vector = WDT_VECTOR
__interrupt void WDT_ISR (void)        // Acknowledged automatically
{
    __low_power_mode_off_on_exit();    // Return to active mode on exit
}
//----------------------------------------------------------------------
// Interrupt service routine for ADC10 after converson: return to active
//----------------------------------------------------------------------
#pragma vector = ADC10_VECTOR
__interrupt void ADC10_ISR (void)      // Acknowledged automatically
{
    __low_power_mode_off_on_exit();    // Return to active mode on exit
}
