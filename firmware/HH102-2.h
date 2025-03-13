#include <16F676.h>
#device ADC=10

#FUSES INTRC_IO                       // WDT, INTRC_IO Watch Dog Timer

#use delay(internal=4000000,restart_wdt)
#use fast_io(A)
#use fast_io(C)

#define DCDC_DISABLE PIN_C4
#define LED PIN_C5
#define DELAY 200

