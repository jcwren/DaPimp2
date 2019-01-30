#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <stdio.h>

//
//
//
#define MUX_OFFSET 0x40         // MUX value for REFS0 to be enabled for AREV <---> VCC tie
#define ADC_SAVE     64         // Average ADC values for sensors

#define CC_D1 PB0
#define CC_D2 PB1
#define CC_D3 PB2
#define ADC0  PC0

#define CLR(x,y) (x&=(~(1<<y)))
#define SET(x,y) (x|=(1<<y))

//
//  Globals
//
static uint16_t adc_volt;
static uint16_t adc_read;
static uint16_t adc_avg_total;
static uint8_t  adc_count;
static uint16_t avg_adc [ADC_SAVE];
static uint8_t  volt_digits [3];

//
//  Pretty close at 1MHz...
//
static void delay_ms (unsigned char ms)
{
  volatile int i;

  while (ms--)
    for (i = 0; i < 54; i++)
      ;
}

//
//  Collect analog values for averaging
//
ISR (ADC_vect)
{
  avg_adc [adc_count] = ADCW;
  adc_count++;
}

//
//  Average sensor data
//
static void adc2avg (void)
{
  static uint8_t i;

  //
  // Reset global vars to zero
  //
  adc_count = 0;
  adc_volt = 0;
  adc_read = 0;
  adc_avg_total = 0;

  //
  // Slow down and average ADC readings to reduce noise, reduce ADC_SAVE to
  // speed up response
  //
  for (i = 0; i <= ADC_SAVE; i ++)
    adc_avg_total += avg_adc [adc_count];

  adc_read += adc_avg_total / ADC_SAVE;
  adc_volt = adc_read;
}

//
//  Convert ADC values into voltage
//
static void adc2volt (void)
{
  //
  //  External spreadsheet used for calculating adc_divider
  //
  float adc_divider = 3.50;

  adc_volt = 10 * (adc_volt / adc_divider);

  //
  //  5% boost on readings below 15v
  //
  if (adc_volt >= 150)
    adc_volt = adc_volt * .985;
}

//
//  Reformat voltage for 7 segment
//
static void digit_breakup (void)
{
  volt_digits [0] = (uint8_t) (adc_volt / 100);
  volt_digits [1] = (uint8_t) ((adc_volt / 10) % 10);
  volt_digits [2] = (uint8_t) (adc_volt % 10);
}

//
//
//
static void write_sev_seg (uint8_t hexcode, uint8_t position)
{
  if (position == 0)
    CLR (PORTB, CC_D1);
  else if (position == 1)
    CLR (PORTB, CC_D2);
  else if (position == 2)
    CLR (PORTB, CC_D3);

  PORTD = hexcode;

  delay_ms (2);

  SET (PORTB, CC_D1);
  SET (PORTB, CC_D2);
  SET (PORTB, CC_D3);
}

static uint8_t digit_2_hex (uint8_t display_digit)
{
  static const uint8_t table [] PROGMEM = { 0x77, 0x12, 0x3d, 0x3e, 0x5a, 0x6e, 0x6f, 0x32, 0x7f, 0x7a };

  if (display_digit > 9)
    return 0x11;

  return pgm_read_byte (&(table [display_digit]));
}

//
//  Send voltage to 7-segment, one digit at a time
//
static void display_digits (int8_t dpPos)
{
  int8_t disp_pos;

  for (disp_pos = 0; disp_pos < 3; disp_pos++)
  {
    uint8_t hexcode = digit_2_hex (volt_digits [disp_pos]);

    if (disp_pos == dpPos)
      hexcode |= 0x80;

    write_sev_seg (hexcode, disp_pos);
  }
}

//
//
//
static void port_init (void)
{
  DDRD = 0xff;        // Display port all outputs
  PORTD = 0x00;       // Disable all pull-ups

  SET (DDRB, CC_D1);  // Output for display, start high, pull low to use
  SET (DDRB, CC_D2);  // Output for display, start high, pull low to use
  SET (DDRB, CC_D3);  // Output for display, start high, pull low to use
  SET (PORTB, CC_D1); // Output for display, start high, pull low to use
  SET (PORTB, CC_D2); // Output for display, start high, pull low to use
  SET (PORTB, CC_D3); // Output for display, start high, pull low to use
}

static void adc_init (void)
{
  SET (ADCSRA,ADPS0);      // ADC pre-scalar0 set division by 8
  SET (ADCSRA,ADPS1);      // ADC pre-scalar1 set division by 8
  CLR (ADCSRA,ADPS2);      // ADC pre-scalar2 cleared division by 8
  SET (ADMUX, REFS0);      // REFS0 (bit 6) on for AREV <--> AVCC
  SET (ADCSRA,ADIE);       // ADC interrupt enable
  SET (ADCSRA,ADEN);       // ADC enable
  SET (ADCSRA, ADSC);      // start adc conversion
}

static void self_test (void)
{
  size_t i;
  size_t j;

  for (i = 0; i < 10; i++)
  {
    volt_digits [0] = i;
    volt_digits [1] = i;
    volt_digits [2] = i;

    for (j = 0; j < 30; j++)
      display_digits (i % 3);
  }
}

//
//
//
int main (void)
{
  port_init ();
  adc_init ();
  sei ();

  self_test ();

  while (1)
  {
    SET (ADCSRA, ADSC);

    if (adc_count == ADC_SAVE)
    {
      adc_count = 0;

      adc2avg ();
      adc2volt ();
      digit_breakup ();
    }

    display_digits (1);
  }

  return 0;
}
