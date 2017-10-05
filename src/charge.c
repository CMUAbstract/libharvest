#include <msp430.h>

#include <libmsp/periph.h>
#include <libmsp/sleep.h>

#include "charge.h"

#define SLEEP_BITS LPM3_bits
#define VREF_SETTLE_CYCLES 4 // @ libmsp's sleep clock (by default ACLK/64)

// Wait for VCC above 1.2V (attach external signal to P9.3)
void harvest_charge()
{
    // COMPE sel
    GPIO(LIBHARVEST_COMP_PORT, SEL0) |= BIT(LIBHARVEST_COMP_PIN);
    GPIO(LIBHARVEST_COMP_PORT, SEL1) |= BIT(LIBHARVEST_COMP_PIN);

    // Setup Comparator_E
    CECTL0 = CEIPEN | COMP_CHAN_SEL(LIBHARVEST_COMP_CHAN); // Enable V+, input channel
    CECTL1 = CEPWRMD_2;                       // ultra low power mode

#ifdef LIBHARVEST_COMP_REF
    // VREF from internal 1.2v reference at given resistor ladder setting
    CECTL2 = CEREFL_1 | CERS_2 | CERSEL | COMP_REF(LIBHARVEST_COMP_REF);
#else // !LIBHARVEST_COMP_REF
    // VREF directly from internal 1.2v reference directly
    CECTL2 = CEREFL_1 | CERS_3 | CERSEL;      // VREF is applied to -terminal
#endif // !LIBHARVEST_COMP_REF

    CECTL3 = COMP_PIN_INPUT_BUF_DISABLE(LIBHARVEST_COMP_CHAN); // Input Buffer Disable for C11 
    CECTL1 |= CEON;                           // Turn On Comparator_E
    msp_sleep(VREF_SETTLE_CYCLES);

    __disable_interrupt(); // atomic check -> sleep -> enable int

    CEINT &= ~CEIFG; // clear int flag
    CEINT |= CEIE;
    if (!(CECTL1 & CEOUT)) { // if event happens after we check, we'll get the ISR after we sleep, b/c GIE
        __bis_SR_register(SLEEP_BITS + GIE);      // LPM3, COMPE_ISR will force exit
        __no_operation();                        // For debug only
    } else {
        __enable_interrupt(); // exit atomic region entered above
    }
}

__attribute__ ((interrupt(COMP_E_VECTOR)))
void COMPE_ISR (void)
{
    CEINT &= ~CEIE;
    CECTL1 &= ~CEON;                           // Turn Off Comparator_E
    __bic_SR_register_on_exit(SLEEP_BITS); // Exit active CPU
}
