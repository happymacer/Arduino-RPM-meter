#include <Arduino.h>
#include <U8x8lib.h>
#include <SPI.h>
#include <Wire.h>

U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);

volatile unsigned int newCount; // will be updated in the pulse interrupt routine
float RPM; // will be updated in the timer interrupt routine
bool dataReady;


void setup() {
  dataReady = false;
  //setup display
  u8x8.begin();
  u8x8.setFont(u8x8_font_7x14_1x2_r);
  u8x8.setFlipMode(1);
  u8x8.clear();

  //setup the built in LED on pin PB5 (PORTB5 is the 5th pin of the port)
  DDRB |= (1<<PORTB5); //1 means output
  PORTB &= ~(1<<PORTB5); // set the output low and the LED off

  PORTD = (1 << PORTD2);
  EICRA = 0b00000011;  //The rising edge of INT0 generates an interrupt request.
  EIMSK = 0x0;

  // set up the built in LED on pin 13
  //pinMode(LED_BUILTIN, OUTPUT);
  
  // set up the timer to interrupt ever 0.1s with an interrupt on overflow
  // these are redundant instrutions due to them being defined as 0 at start anyway.
  TCCR1A = 0;  // table 15-2 page 108 and table 15-5 page 109
  TCCR1B = 0;  // page 110 - also stops the timer 1
  TCCR1C = 0;  // page 111

  // the timer 1 count range is 0 to 2^16 (ie 65535) so counts must be < 65535
  
  // crystal is 16MHz, and CLKio = crystal/1 set in low fuse, so timer freq=16MHz, so period = 1/(16e6) = 62.5nS
  // so if prescaler = 1, => period is 1*62.5nS = 62.5nS 
  // we want 1S so counts = 1/62.5e-9 = 16million

  // crystal is 16MHz, and CLKio = crystal/1 set in low fuse, so timer freq=16MHz, so period = 1/(16e6) = 62.5nS
  // so if prescaler = 8, => period is 8*62.5nS = 500nS 
  // we want 1S so counts = 1/500e-9 = 2000 000

  // crystal is 16MHz, and CLKio = crystal/1 set in low fuse, so timer freq=16MHz, so period = 1/(16e6) = 62.5nS
  // so if prescaler = 64, => period is 64*62.5nS = 4uS 
  // we want 1S so counts = 1/4e-6 = 250000  

  // crystal is 16MHz, and CLKio = crystal/1 set in low fuse, so timer freq=16MHz, so period = 1/(16e6) = 62.5nS
  // so if prescaler = 256, => period is 256*62.5nS = 16uS 
  // we want 1S so counts = 1/16e-6 = 62500 - selected

  // crystal is 16MHz, and CLKio = crystal/1 set in low fuse, so timer freq=16MHz, so period = 1/(16e6) = 62.5nS
  // so if prescaler = 1024, => period is 1024*62.5nS = 64uS 
  // we want 1S so counts = 1/64e-6 = 15625
  
  // start the timer by 
  //TCCR1B |= (1 << CS12); //page 110 
  // or TCCR1B |= 0b00000100;
  
  // if we start counting at 65535-62500 = 3035 = 0xBDB in hex (set TCNT1 - normal mode )
  // using normal mode as set in TCCR1A and B - 
  TCNT1 = 0xBDB;
  TIMSK1 |= (1 << TOIE1); //enable timer overflow page 112
  RPM = 0;
  newCount = 0; 
}

ISR(TIMER1_OVF_vect) { 
    EIMSK = 0x0; //stop counting
    TCCR1B = 0x0; //stop the timer from restarting
    PORTB &= ~(1<<PORTB5); // turn the LED off
    //digitalWrite(LED_BUILTIN, LOW);
    dataReady = true;
}

ISR(INT0_vect) {
    ++newCount;
}

void displayData () {
     // N = 36 holes, so 1 rpm = 36 holes in 1 minute or 36/60 holes in 1 second, or 36/600 holes in 0.1 second.  
     // If you use the 0.1s count period then be weary of the error magnification that occurs when you convert a 
     // count in 0.1s to 1 minute - ie any error gets magnified by 600, so 1 extra count in 0.1s period is a 
     // 600 more counts had it you counted over 1s instead 
     RPM = (newCount*60/36);   
     //u8x8.clear();
     u8x8.setCursor(0,0);
     u8x8.print("RPM : ");
     u8x8.setCursor(6,0);
     u8x8.print(RPM);
}

void loop() {

  if ( (TCCR1B == 0)) {
    PORTB |= (1<<PORTB5); //turn on the LED
    //digitalWrite(LED_BUILTIN, HIGH);
    // reset the global variables
    dataReady = false;
    newCount = 0;
    // turn on the timer      
    TCCR1B = (1 << CS12); //set prescaler to 256
    // turn on the counter
    EIMSK = (1 << INT0);     
  } 
  if (dataReady) {
    displayData();     
  }
  // now go do something else
       
}  


 
