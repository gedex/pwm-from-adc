/**
 * PWM from ADC for Robot's Joystick.
 *
 * @author Akeda Bagus <admin@gedex.web.id>
 * @copyright 2006
 * @license MIT
 * @compiler CodeVisionAVR
 * @mcu ATMega16
 * @clock_frequency 4 MHz
 * @external_sram_size 0
 * @data_stack_size 256
 */

#include <mega16.h>
#include <stdio.h>

// Alphanumeric LCD Module functions
#asm
   .equ __lcd_port=0x18 ;PORTB
#endasm
#include <lcd.h>
void tampil(unsigned char dat)
{
    unsigned char data;

    data  = dat / 100;
    data += 0x30;
    lcd_putchar(data);

    dat  %= 100;
    data  = dat / 10;
    data += 0x30;
    lcd_putchar(data);

    dat %= 10;
    data = dat + 0x30;
    lcd_putchar(data);
}

#define dirA_Ki PORTD.0     // Direction A for left motor
#define dirB_Ki PORTD.1     // Direction B for left motor
#define EnKi PORTD.2        // Enable L298 for left motor
#define EnKa PORTD.5        // Enable L298 for right motor
#define dirC_Ka PORTD.6     // Direction C for right motor
#define dirD_Ka PORTD.3     // Direction D for right motor

unsigned char xcount,lpwm,rpwm;
// Timer 0 overflow interrupt service routine
interrupt [TIM0_OVF] void timer0_ovf_isr(void)
{
    xcount++;
    if ( xcount <= lpwm )
        EnKi = 1;
    else
        EnKi = 0;

    if ( xcount <= rpwm )
        EnKa = 1;
    else
        EnKa = 0;

    TCNT0 = 0xc0;
}

/* Moves left clockwise */
void ki_cw()
{
    dirA_ki = 1;
    dirB_ki = 0;
}

/* Moves left counter-clockwise */
void ki_ccw()
{
    dirA_ki = 0;
    dirB_ki = 1;
}

/* Moves right clockwise */
void ka_cw()
{
    dirC_ka = 1;
    dirD_ka = 0;
}

/* Moves left counter-clockwise */
void ka_ccw()
{
    dirC_ka = 0;
    dirD_ka = 1;
}

unsigned char data_adc;
char tampil_adc[12];
#define ADC_VREF_TYPE 0x20

// Read the 8 most significant bits
// of the AD conversion result
unsigned char read_adc(unsigned char adc_input)
{
    ADMUX = adc_input | (ADC_VREF_TYPE & 0xff);
    // Start the AD conversion
    ADCSRA |= 0x40;
    // Wait for the AD conversion to complete
    while ((ADCSRA & 0x10) == 0);

    ADCSRA |= 0x10;
    return ADCH;
}

void main(void)
{
    // Input/Output Ports initialization
    // Port A initialization
    PORTA = 0x00;
    DDRA  = 0x00;

    // Port B initialization
    PORTB = 0x00;
    DDRB  = 0x00;

    // Port C initialization
    PORTC = 0x00;
    DDRC  = 0x00;

    // Port D initialization
    PORTD = 0x00;
    DDRD  = 0x00;

    // Timer/Counter 0 initialization
    // Clock source: System Clock
    // Clock value: Timer 0 Stopped
    // Mode: Normal top=FFh
    // OC0 output: Disconnected
    TCCR0 = 0x01;
    TCNT0 = 0xc0;
    OCR0  = 0x00;

    // Timer/Counter 1 initialization
    // Clock source: System Clock
    // Clock value: Timer 1 Stopped
    // Mode: Normal top=FFFFh
    // OC1A output: Discon.
    // OC1B output: Discon.
    // Noise Canceler: Off
    // Input Capture on Falling Edge
    // Timer 1 Overflow Interrupt: Off
    // Input Capture Interrupt: Off
    // Compare A Match Interrupt: Off
    // Compare B Match Interrupt: Off
    TCCR1A = 0x00;
    TCCR1B = 0x00;
    TCNT1H = 0x00;
    TCNT1L = 0x00;
    ICR1H  = 0x00;
    ICR1L  = 0x00;
    OCR1AH = 0x00;
    OCR1AL = 0x00;
    OCR1BH = 0x00;
    OCR1BL = 0x00;

    // Timer/Counter 2 initialization
    // Clock source: System Clock
    // Clock value: Timer 2 Stopped
    // Mode: Normal top=FFh
    // OC2 output: Disconnected
    ASSR  = 0x00;
    TCCR2 = 0x00;
    TCNT2 = 0x00;
    OCR2  = 0x00;

    // External Interrupt(s) initialization
    // INT0: Off
    // INT1: Off
    // INT2: Off
    MCUCR  = 0x00;
    MCUCSR = 0x00;

    // Timer(s)/Counter(s) Interrupt(s) initialization
    TIMSK = 0x01;

    // Analog Comparator initialization
    // Analog Comparator: Off
    // Analog Comparator Input Capture by Timer/Counter 1: Off
    ACSR  = 0x80;
    SFIOR = 0x00;

    // ADC initialization
    // ADC Clock frequency: 125.000 kHz
    // ADC Voltage Reference: AREF pin
    // ADC Auto Trigger Source: None
    // Only the 8 most significant bits of
    // the AD conversion result are used
    ADMUX  = ADC_VREF_TYPE & 0xff;
    ADCSRA = 0x85;

    // LCD module initialization
    lcd_init(16);

    // Global enable interrupts
    #asm("sei")

    lpwm = 0;
    rpwm = 0;

    // LCD is used for debugging.
    while (1) {
        // Left motor
        data_adc = read_adc(0);
        if (data_adc > 128) { // CW
            lcd_gotoxy(4,0);
            lcd_putchar('C');
            lcd_putchar('W');
            lcd_putchar(' ');
            lpwm = ( data_adc - 128 ) * 2;
            ki_cw();
        } else if (data_adc < 126) {  // CCW
            lcd_gotoxy(4,0);
            lcd_putchar('C');
            lcd_putchar('C');
            lcd_putchar('W');
            lpwm = 255 - (data_adc * 2);
            ki_ccw();
        } else if ( (data_adc >= 126) && (data_adc <= 128) ) { // stop
            lcd_gotoxy(4,0);
            lcd_putchar('O');
            lcd_putchar('F');
            lcd_putchar('F');
            lpwm = 0;
        }
        lcd_gotoxy(0,0);
        tampil(lpwm);
        lcd_gotoxy(0,1);
        tampil(data_adc);

        // Right motor
        data_adc = read_adc(1);
        if (data_adc > 128) { // CW
            lcd_gotoxy(13,0);
            lcd_putchar('C');
            lcd_putchar('W');
            lcd_putchar(' ');
            rpwm = ( data_adc - 128 ) * 2;
            ka_cw();
        } else if (data_adc < 126) { // CCW
            lcd_gotoxy(13,0);
            lcd_putchar('C');
            lcd_putchar('C');
            lcd_putchar('W');
            rpwm = 255 - (data_adc * 2);
            ka_ccw();
        } else if ( (data_adc >= 126) && (data_adc <= 128) ) { // stop
            lcd_gotoxy(13,0);
            lcd_putchar('O');
            lcd_putchar('F');
            lcd_putchar('F');
            rpwm = 0;
        }
        lcd_gotoxy(9,0);
        tampil(rpwm);
        lcd_gotoxy(9,1);
        tampil(data_adc);

    }; // end while(1)
}
