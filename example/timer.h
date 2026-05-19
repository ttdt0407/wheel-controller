
void Timer0(){
  cli();
  TCCR0A = 0;
  TCCR0B = 0;
  TCNT0=0;
  TIMSK0 = 0;

  // TCCR0B |= (1 << WGM02) |(1 << CS01 ) | (1 << CS00);   PSC 16
  TCCR0B |= (1 << WGM02) |(1 << CS01 ); // PSC 8
  TCCR0A |= (1 << WGM01) |(1 << WGM00) | (1 << COM0A1);
  OCR0A =  200;
  TIMSK0 |= (1 << OCIE0A) ;
  sei();
}
void Timer1(){
  cli(); 
   TCCR1A = 0;
   TCCR1B = 0;
   TIMSK1 = 0;
   OCR1A = 0;
   OCR1B = 0;
   TCNT1 = 24;
   ICR1 = 0;
   TCCR1A |=  (1 << COM1A1)| (1 << COM1B1)| (1 << WGM11) | (1 << WGM10);
   TCCR1B |= (1 << ICES1)  | (1 << CS10)| (1 << WGM12);    // prescale = 1
   TIMSK1 = (1 << ICIE1)  |(1 << TOIE1);
   sei(); 
} 
void Timer3(){
  cli();
  TCCR3A = 0;
  TCCR3B = 0;
  TIMSK3 = 0;
  TCCR3B |= (1 << WGM33) |(1 << WGM32) | (1 << CS31);
  TCCR3A |= (1 << WGM31) |  (1 << COM3A1)| (1 << COM3B1);
  ICR3 = 39999;
  OCR3A = 0;
  OCR3B = 0;
  TIMSK3 = (1 << TOIE3);
  sei();
}
