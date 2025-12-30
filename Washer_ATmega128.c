#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 8000000UL
#include <util/delay.h>

char led_pattern = 0b10000000;
volatile unsigned int timer0_count, stop_interrupt, timer2_count, timer_save, seconds;
int is_enabled = 0;
int load_switch = 0;
int load_save = 0b11101000;
unsigned int adc_result = 0xFFFF, adc_threshold = 0x0100;

void mode1(void)
{
    TimerMoter(2,1);
    TimerMoter(1,0);
    TimerMoter(2,1);
    TimerMoter(1,0);
    stop();
}

void mode2(void)
{
    TimerMoter(10,1);
    stop();
}

void mode3(void)
{
    TimerMoter(10,0);
    stop();
}

void stop(void)
{
    PORTB = (0<<PB0) | (0<<PB1);
    TIMSK &= 0xFE;
    PORTE = 0b11101000;
}

void loadingled(void)
{
    if(!load_switch)
    {
        PORTE = ((PORTE & 0b00001111) | led_pattern);
        led_pattern >>= 1;
        if(led_pattern == 0b00010000)
        {
            load_switch = 1;
        }
    }
    else
    {
        PORTE = ((PORTE & 0b00001111) | led_pattern);
        led_pattern <<= 1;
        if(led_pattern == 0b10000000)
        {
            load_switch = 0;
        }
    }
}

void Init_Timer0(void)
{
    TIMSK |= 0x01;
    TCCR0 = 0x02;
    TCNT0 = 0x38;
}

void Init_Timer2(void)
{
    TIMSK |= 0x40;
    TCCR2 = 0x02;
    TCNT2 = 0x38;
}

ISR(TIMER0_OVF_vect)
{
    TCNT0 = 0x38;
    timer0_count++;

    if(timer0_count >= 5000)
    {
        timer0_count = 0;
        loadingled();
    }
}

ISR(TIMER2_OVF_vect)
{
    adc_result = ADC;

    if(adc_result < adc_threshold)
    {
        is_enabled = 0;
        stop();
        stopsign();
    }

    if(stop_interrupt == 0)
    {
        TCNT2 = 0x38;
        timer2_count++;
    }

    if(timer2_count >= 10000)
    {
        timer2_count = 0;
        seconds++;
    }
}

ISR(INT3_vect)
{
    if(stop_interrupt == 1)
    {
        timer2_count = timer_save;
        stop_interrupt = 0;
    }
    else if(stop_interrupt == 0)
    {
        timer_save = timer2_count;
        stop_interrupt = 1;
        PORTE &= 240;
        PORTE |= (8 << PE0);
    }
}

void TimerMoter(int alarm_time, int motor_direction)
{
    seconds = 0;
    timer2_count = 0;

    while(is_enabled)
    {
        if(stop_interrupt == 0)
        {
            TIMSK |= 0x41;
            PORTB = ((motor_direction && 1) << PB0) | (!(motor_direction && 1) << PB1);
        }
        else
        {
            stop();
        }

        if(seconds >= alarm_time)
        {
            break;
        }
    }

    PORTB = (0<<PB0) | (0<<PB1);
}

void stopsign()
{
    PORTA = 0x6D;
    PORTC = ~0x01;
    _delay_ms(1);

    PORTA = 0b01111000;
    PORTC = ~0x02;
    _delay_ms(1);

    PORTA = 0x3F;
    PORTC = ~0x04;
    _delay_ms(1);

    PORTA = 0b01110011;
    PORTC = ~0x08;
    _delay_ms(1);
}

int main(void)
{
    cli();

    DDRB = 0x03;
    DDRC = 0b00001111;
    DDRE = 0xFF;
    DDRA = 0xFF;

    PORTE = 0b10000000;

    Init_Timer0();
    Init_Timer2();

    timer0_count = 0;
    timer2_count = 0;
    stop_interrupt = 0;
    seconds = 0;

    PORTE = 0b00001000;
    PORTD = 0;

    EICRA = 0b11000000;
    EIMSK = 0x08;

    DDRF &= 0b10111111;
    ADMUX = 0x06;
    ADCSRA = 0xA7;
    ADCSRA |= 0x40;

    sei();

    while(1)
    {
        if(adc_result < adc_threshold)
        {
            is_enabled = 0;
            stop();
            stopsign();
        }
        else
        {
            is_enabled = 1;
            DDRD |= 0b00000100;

            if(PIND & 0b00000001)
            {
                PORTE &= 240;
                PORTE |= (1 << PE0);
            }
            else if(PIND & 0b00000010)
            {
                PORTE &= 240;
                PORTE |= (2 << PE0);
            }
            else if(PIND & 0b00000100)
            {
                PORTE &= 240;
                PORTE |= (4 << PE0);
            }

            if(PINE & 0b00000001)
            {
                TIMSK = 0x41;
                PORTC = ~0x08;
                PORTA = 0x06;
                mode1();
            }
            else if(PINE & 0b00000010)
            {
                TIMSK = 0x41;
                PORTC = ~0x08;
                PORTA = 0x5B;
                mode2();
            }
            else if(PINE & 0b00000100)
            {
                TIMSK = 0x41;
                PORTC = ~0x08;
                PORTA = 0x4F;
                mode3();
            }

            if(PINE & 0b00001000)
            {
                stop();
                stopsign();
            }
        }
    }

    return 0;
}
