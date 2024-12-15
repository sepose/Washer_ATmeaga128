#include <avr/io.h>
#include <avr/interrupt.h>    
#define F_CPU 8000000UL    
#include <util/delay.h>   


char led = 0b10000000;

volatile unsigned int howmuch,StopInterrupt,howmuch1,TimerSave,Seconds;

int Close = 0;

int loadswitch = 0;

int loadsave = 0b11101000;

unsigned int result = 0xFFFF, th = 0x0100;

void mode1(void) {

TimerMoter(2,1);

TimerMoter(1,0);

TimerMoter(2,1);   

TimerMoter(1,0);

stop();
}

void mode2(void) {TimerMoter(10,1); stop();}

void mode3(void) {TimerMoter(10,0); stop();}

void stop(void) { PORTB = (0<<PB0) | (0<<PB1);  TIMSK &= 0xFE; PORTE = 0b11101000; }


void loadingled(void) {

if(!loadswitch)
{

PORTE = ((PORTE & 0b00001111) | led);
led >>= 1;

   if(led == 0b00010000) loadswitch = 1;}

else{

PORTE = ((PORTE & 0b00001111) | led);
led <<= 1;

   if(led == 0b10000000) loadswitch = 0;

}
}

void Init_Timer0(void)
{

TIMSK|=0x01;
TCCR0 = 0x02; 
TCNT0 = 0x38;
}

void Init_Timer2(void)
{

TIMSK|=0x40;
TCCR2 = 0x02; 
TCNT2 = 0x38;

}



ISR(TIMER0_OVF_vect)
{
   TCNT0 = 0x38; 
   howmuch++;
   
   if(howmuch >= 5000)
   {
      howmuch = 0;
      loadingled();
   }
}

ISR(TIMER2_OVF_vect)
{
   
   result = ADC;
   if(result < th) {Close = 0; stop(); stopsign();}
     
if(StopInterrupt == 0){

   TCNT2 = 0x38; 
   howmuch1++;
   }
   if(howmuch1 >= 10000)
   {
      howmuch1 = 0;
      Seconds++;
   }
}
ISR(INT3_vect)
{
   if(StopInterrupt == 1) 
   {
   howmuch1 = TimerSave;
   StopInterrupt=0;
   }
   else if(StopInterrupt == 0)
   {
   TimerSave = howmuch1;
   StopInterrupt= 1;
   PORTE &= 240; PORTE |= (8 << PE0);
   
   }
}

void TimerMoter(int Alarm,int MoterDirection)
{
   Seconds = 0;
   howmuch1= 0;
   

   while(Close)
   {

   if(StopInterrupt == 0) {TIMSK |= 0x41; PORTB = ((MoterDirection&&1)<<PB0) | (!(MoterDirection&&1)<<PB1);}

   else {stop();}

   if(Seconds >= Alarm) break;

   }

   PORTB = ((0<<PB0) | (0<<PB1));
   
}

void stopsign(){ 

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

cli(); //인터럽트 비활성화

DDRB=0x03;    // 포트B의 8비트 전체를 출력으로 설정합니다.
DDRC = 0b00001111;   // 원래 버튼 포트 C
DDRE = 0xFF;

DDRA = 0xFF;

PORTE = 0b10000000;
Init_Timer0();
Init_Timer2();

howmuch = 0;
howmuch1 = 0;

StopInterrupt = 0;
Seconds = 0;

PORTE = 0b00001000;
PORTD = 0;
EICRA = 0b11000000; //int3 request in rising edge
EIMSK = 0x08;         // int3 인터럽트 허용 

DDRF &= 0b10111111; 
ADMUX = 0x06;
ADCSRA = 0xA7;

ADCSRA |= 0x40;

sei(); // 인터럽트 활성화

while (1){ // 아래 구문을 무한 반복합니다.

      
     
   if(result < th) {Close = 0; stop(); stopsign();}

    else{

     Close = 1;

     DDRD |= 0b00000100;

     if (PIND & 0b00000001) 

	 {PORTE &= 240; PORTE |= (1 << PE0);}

     else if (PIND & 0b00000010) 
	 
	 {PORTE &= 240; PORTE |= (2 << PE0);}

     else if (PIND & 0b00000100) 
	 
	 {PORTE &= 240; PORTE |= (4 << PE0);}

     if (PINE & 0b00000001) 
	 
	 {TIMSK=0x41; PORTC= ~0x08; PORTA = 0x06; mode1();}

     else if (PINE & 0b00000010) 
	 
	 {TIMSK=0x41; PORTC= ~0x08; PORTA = 0x5B; mode2();}

     else if (PINE & 0b00000100) 
	 
	 {TIMSK=0x41; PORTC= ~0x08; PORTA = 0x4F; mode3();}

     if (PINE & 0b00001000) {stop(); stopsign();}

      }
                                                                  
      }


return 0;    // main함수에 0을 리턴합니다.

}
