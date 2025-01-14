/******************************************************************/
/* This is just an example of code structure! OF COURSE this      */
/* code can be greatly optimized, and it is not clean at all but  */
/* the idea is to make it so simple and free of libraries that    */
/* it becomes easy to make changes and we can observe the results */
/******************************************************************/
//set your clock speed
//#define F_CPU 16000000UL
//these are the include files. They are outside the project folder
#include <avr/io.h>
//#include <iom1284p.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define diodeLED 13 // Only to debug
#define VFD_data 8  // 
#define VFD_clk 9 // 
#define VFD_stb 10 // Must be pulsed to LCD fetch data of bus

/*Global Variables Declarations*/
bool flagSet = false;
bool flagReached = false;
bool clockWakeUp = false;
bool clockAlarm = false;
bool setAlarm = false;
bool resetAlarm = false;
uint8_t myByte= 0x00;   // this variable is only related with swapLed1.

unsigned char hours = 0;
unsigned char minutes = 0;
unsigned char seconds = 0;
unsigned char milisec = 0;

unsigned char secs;

unsigned char number;
unsigned char numberAclock;
unsigned char numberBclock;
unsigned char numberCclock;
unsigned char numberDclock;
unsigned char numberEclock;
unsigned char numberFclock;

unsigned char numberAalarm;
unsigned char numberBalarm;
unsigned char numberCalarm;
unsigned char numberDalarm;
unsigned char numberEalarm;
unsigned char numberFalarm;

unsigned char wordAalarm = 0;
unsigned char wordBalarm = 0;
unsigned char wordCalarm = 0;
unsigned char wordDalarm = 0;
unsigned char wordEalarm = 0;
unsigned char wordFalarm = 0;

unsigned char wakeSeconds = 0;
unsigned char wakeMinutes = 0;
unsigned char wakeHours = 0;

unsigned char wordAclock = 0;
unsigned char wordBclock = 0;
unsigned char wordCclock = 0;
unsigned char wordDclock = 0;
unsigned char wordEclock = 0;
unsigned char wordFclock = 0;

unsigned int segments[] ={
  //This not respect the normal table for 7segm like "abcdefgh"  // 
      0b01110111, //0  // 
      0b00010010, //1  // 
      0b01101011, //2  // 
      0b01011011, //3  // 
      0b00011110, //4  // 
      0b01011101, //5  // 
      0b01111101, //6  // 
      0b00010011, //7  // 
      0b01111111, //8  // 
      0b00011111, //9  // 
      0b00000000, //10 // empty display
      0b11111111  //11 // Filled display
  };
/*****************************************************************/
void pt6311_init(void){
  delay(200); //power_up delay
  // Note: Allways the first byte in the input data after the STB go to LOW is interpret as command!!!

  // Configure VFD display (grids)
  cmd_with_stb(0b00000000);//  cmd1 8 grids 20 segm, this is the minimum value of digits at PT6311
  delayMicroseconds(1);
  // turn vfd on, stop key scannig
   cmd_with_stb(0b10001000);//(BIN(01100110)); 
  delayMicroseconds(1);
  // Write to memory display, increment address, normal operation
  cmd_with_stb(0b01000000);//(BIN(01000000));
  delayMicroseconds(1);
  // Address 00H - 15H ( total of 11*2Bytes=176 Bits)
  cmd_with_stb(0b11000000);//(BIN(01100110)); 
  delayMicroseconds(1);
  // set DIMM/PWM to value
  cmd_with_stb((0b10001000) | 7);//0 min - 7 max  )(0b01010000)
  delayMicroseconds(1);
}
void cmd_4bitsWithout_stb(unsigned char a){
  // send without stb
  unsigned char data = 170; //value to transmit, binary 10101010
  unsigned char mask = 1; //our bitmask
  
  data=a;
  //This don't send the strobe signal, to be used in burst data send
   for (mask = 00000001; mask>0; mask <<= 1) { //iterate through bit mask
     digitalWrite(VFD_clk, LOW);
     if (data & mask){ // if bitwise AND resolves to true
        digitalWrite(VFD_data, HIGH);
     }
     else{ //if bitwise and resolves to false
       digitalWrite(VFD_data, LOW);
     }
    delayMicroseconds(5);
    digitalWrite(VFD_clk, HIGH);
    delayMicroseconds(5);
   }
}
void cmd_without_stb(unsigned char a){
  // send without stb
  unsigned char data = 170; //value to transmit, binary 10101010
  unsigned char mask = 1; //our bitmask
  
  data=a;
  //This don't send the strobe signal, to be used in burst data send
   for (mask = 00000001; mask>0; mask <<= 1) { //iterate through bit mask
     digitalWrite(VFD_clk, LOW);
     if (data & mask){ // if bitwise AND resolves to true
        digitalWrite(VFD_data, HIGH);
     }
     else{ //if bitwise and resolves to false
       digitalWrite(VFD_data, LOW);
     }
    delayMicroseconds(5);
    digitalWrite(VFD_clk, HIGH);
    delayMicroseconds(5);
   }
   //digitalWrite(VFD_clk, LOW);
}
void cmd_4bitsWith_stb(unsigned char a){
  // send with stb
  unsigned char data = 170; //value to transmit, binary 10101010
  unsigned char mask = 1; //our bitmask
  
  data=a;
  
  //This send the strobe signal
  //Note: The first byte input at in after the STB go LOW is interpreted as a command!!!
  digitalWrite(VFD_stb, LOW);
  delayMicroseconds(1);
   for (mask = 00000001; mask>0; mask <<= 1) { //iterate through bit mask
     digitalWrite(VFD_clk, LOW);
     delayMicroseconds(1);
     if (data & mask){ // if bitwise AND resolves to true
        digitalWrite(VFD_data, HIGH);
     }
     else{ //if bitwise and resolves to false
       digitalWrite(VFD_data, LOW);
     }
    digitalWrite(VFD_clk, HIGH);
    delayMicroseconds(1);
   }
   digitalWrite(VFD_stb, HIGH);
   delayMicroseconds(1);
}
void cmd_with_stb(unsigned char a){
  // send with stb
  unsigned char data = 170; //value to transmit, binary 10101010
  unsigned char mask = 1; //our bitmask
  
  data=a;
  
  //This send the strobe signal
  //Note: The first byte input at in after the STB go LOW is interpreted as a command!!!
  digitalWrite(VFD_stb, LOW);
  delayMicroseconds(1);
   for (mask = 00000001; mask>0; mask <<= 1) { //iterate through bit mask
     digitalWrite(VFD_clk, LOW);
     delayMicroseconds(1);
     if (data & mask){ // if bitwise AND resolves to true
        digitalWrite(VFD_data, HIGH);
     }
     else{ //if bitwise and resolves to false
       digitalWrite(VFD_data, LOW);
     }
    digitalWrite(VFD_clk, HIGH);
    delayMicroseconds(1);
   }
   digitalWrite(VFD_stb, HIGH);
   delayMicroseconds(1);
}
void test_VFD(void){
    clear_VFD();
    digitalWrite(VFD_stb, LOW);
    delayMicroseconds(1);
    cmd_with_stb(0b00001000); // cmd 1 // 5 Grids & 16 Segments
    cmd_with_stb(0b01000000); // cmd 2 //Write VFD, Normal operation; Set pulse as 1/16, Auto increment
    cmd_with_stb(0b10001000 | 0x07); // cmd 2 //set on, dimmer to max
    digitalWrite(VFD_stb, LOW);
    delayMicroseconds(1);
    cmd_without_stb((0b11000000)); //cmd 3 wich define the start address (00H to 15H)
    // Only here must change the bit to test, first 2 bytes and 1/2 byte of third.
         for (int i = 0; i < 8 ; i++){ // test base to 20 segm and 8 grids
          // Zone of test, if write 1 on any position of 3 bytes below position, will bright segment corresponding it.
         cmd_without_stb(0b11111111); // Data to fill first posição of memory belongs to the digit.
         cmd_without_stb(0b11111111); // Data to fill second posição of memory belongs to the digit.
         cmd_4bitsWithout_stb(0b00001111); // Data to fill third posição of memory belongs to the digit.
         }
      //cmd_without_stb(0b00000001); // cmd1 Here I define the 5 grids and 16 Segments
      //cmd_with_stb((0b10001000) | 7); //cmd 4
      digitalWrite(VFD_stb, HIGH);
      delay(1);
      delay(10);  
}
void clear_VFD(void){
      for (uint8_t n=0; n < 12; n=n+3){  //
      //Note we here implement a skip by 3, because the address RAM of each grid is:
      //0, 3, 6, 9, C wich correspond to grids: 0 until 4 (total of 5 grids)
      //by this reason each cycle for will fill the 3 positions of RAM address.
        cmd_with_stb(0b00000000); //       cmd 1 // 8 Grids & 20 Segments is minimum value of digits to the PT6311
        cmd_with_stb(0b01000000); //       cmd 2 //Normal operation; Set pulse as 1/16
        digitalWrite(VFD_stb, LOW);
        delayMicroseconds(1);
            cmd_without_stb((0b11000000) | n); // cmd 3 //wich define the start address (00H to 15H)
            cmd_without_stb(0b00000000); // Data to fill first posição of memory belongs to the digit.
            cmd_without_stb(0b00000000); // Data to fill second posição of memory belongs to the digit.
            cmd_4bitsWithout_stb(0b00000000); // Data to fill third posição of memory belongs to the digit.
            //
            //cmd_with_stb((0b10001000) | 7); //cmd 4
            digitalWrite(VFD_stb, HIGH);
            delayMicroseconds(10);
     }
}
void animateTheWheels(){
  for(uint8_t i =0x00; i < 5; i++){
            switch (i){
              case 0: runWheels(0b00100001);break;
              case 1: runWheels(0b00100010);break;
              case 2: runWheels(0b00110000);break;
              case 3: runWheels(0b01100000);break;
              case 4: runWheels(0b00100001);break;
            }
            delay(100);
  }
}
void runWheels(uint8_t seg){
                //cmd1 Configure VFD display (grids) 
              cmd_with_stb(0b00000000);//  8 grids is the minimum value to PT6311
              delayMicroseconds(1);  // 
              
              //cmd2 Write to memory display, increment address, normal operation 
              cmd_with_stb(0b01000000);//Teste mode setting to normal, Address increment Fixed, Write data to display memory...
              
              digitalWrite(VFD_stb, LOW);
              delayMicroseconds(1);
              //cmd3 Address 15H Start 3 position of memory allocated to the 6ª grid(total of 2bytes and half).
              cmd_without_stb(0b11001100);//Increment active, then test all segments from positions 15 to 17, grid 6!
              delayMicroseconds(1);
              cmd_without_stb(0b00000000); // Data to fill first posição of memory belongs to the digit.
              cmd_without_stb(seg); // Data to fill second posição of memory belongs to the digit.
              cmd_4bitsWithout_stb(0b00000000); // Data to fill third posição of memory belongs to the digit.
              delayMicroseconds(1);
              digitalWrite(VFD_stb, HIGH);
              //cmd4 set DIMM/PWM to value
              cmd_with_stb((0b10001000) | 7);//0 min - 7 max  )(0b01010000)//0 min - 7 max  )(0b01010000)
              delay(100);
}
void creatUpdateClockInfo(void){
  if (secs >=60){
    secs =0;
    minutes++;
  }
  if (minutes >=60){
    minutes =0;
    hours++;
  }
  if (hours >=24){
    hours =0;
  }
   //*************************************************************
    numberAclock = (secs%10);
    numberBclock = (secs/10);
    sendTo7segDigitClock();
    //*************************************************************
    numberCclock = (minutes%10);
    numberDclock = (minutes/10);
    sendTo7segDigitClock();
    //**************************************************************
    numberEclock = (hours%10);
    numberFclock = (hours/10);
    sendTo7segDigitClock();
    //**************************************************************
}
void createUpdateWakeUpAlarm(void){
  if (wakeSeconds >=60){
    wakeSeconds =0;
    wakeMinutes++;
  }
  if (wakeMinutes >=60){
    wakeMinutes =0;
    wakeHours++;
  }
  if (wakeHours >=24){
    wakeHours =0;
  }
    //*************************************************************
    numberAalarm = (wakeSeconds%10);
    numberBalarm = (wakeSeconds/10);
    sendTo7segDigitAlarm();
    //*************************************************************
    numberCalarm = (wakeMinutes%10);
    numberDalarm = (wakeMinutes/10);
    sendTo7segDigitAlarm();
    //**************************************************************
    numberEalarm = (wakeHours%10);
    numberFalarm = (wakeHours/10);
    sendTo7segDigitAlarm();
    //**************************************************************
    //
}
void sendTo7segDigitClock(){
  // This block is very important, it solve the difference 
  // between segments from grid 1 and grid 2(start 8 or 9)
  digitalWrite(VFD_stb, LOW);
  delayMicroseconds(10);
      cmd_with_stb(0b00000001); // cmd 1 // 5 Grids & 16 Segments
      cmd_with_stb(0b01000000); // cmd 2 //Normal operation; Set pulse as 1/16
      
        digitalWrite(VFD_stb, LOW);
        delayMicroseconds(10);
        cmd_without_stb((0b11000000) | 0x00); //cmd 3 wich define the start address (00H to 15H)
        wordAclock=segments[numberAclock];
        wordBclock=segments[numberBclock];
        wordCclock=segments[numberCclock];
        wordDclock=segments[numberDclock];
        wordEclock=segments[numberEclock];
        wordFclock=segments[numberFclock];
            
          // Grid 0..............................................This column is Memory address!
          cmd_without_stb(wordAclock);// seconds unit //---------0
          cmd_without_stb(wordBclock);// seconds dozens  //------1
          cmd_4bitsWithout_stb(0x00);//--------------------------2
          // Grid 1
          cmd_without_stb(0x00);// //----------------------------3
          cmd_without_stb(0x00);// //----------------------------4
          cmd_4bitsWithout_stb(0x00);//--------------------------5
          // Grid 2
          cmd_without_stb(wordCclock); //   Minuts unit //-------6
          cmd_without_stb(wordDclock); //   Minuts dozens //-----7
          cmd_4bitsWithout_stb(0x00);//--------------------------8
          // Grid 3
          cmd_without_stb(wordEclock); //  Hours unit   //-------9
          cmd_without_stb(wordFclock); //  Hours dozens //-------A
          cmd_4bitsWithout_stb(0x00);// //-----------------------B
          // // Grid 4
          cmd_without_stb(0x00);//      //-----------------------C
          cmd_without_stb(0x00);//   //--------------------------D
          cmd_4bitsWithout_stb(0x00);// //-----------------------E
          //
      digitalWrite(VFD_stb, HIGH);
      delayMicroseconds(10);
      cmd_with_stb((0b10001000) | 7); //cmd 4
      delay(5);      
}
void sendTo7segDigitAlarm(){
  digitalWrite(VFD_stb, LOW);
  delayMicroseconds(10);
      cmd_with_stb(0b00000001); // cmd 1 // 8 Grids 
      cmd_with_stb(0b01000000); // cmd 2 //Normal operation; Set pulse as 1/16
      
        digitalWrite(VFD_stb, LOW);
        delayMicroseconds(10);
        cmd_without_stb((0b11000000) | 0x00); //cmd 3 wich define the start address (00H to 15H)

        wordAalarm=segments[numberAalarm];
        wordBalarm=segments[numberBalarm];
        wordCalarm=segments[numberCalarm];
        wordDalarm=segments[numberDalarm];
        wordEalarm=segments[numberEalarm];
        wordFalarm=segments[numberFalarm];
          // Grid 0..............................................This column is Memory address!
          cmd_without_stb(wordAalarm);// seconds unit //---------0
          cmd_without_stb(wordBalarm);// seconds dozens  //------1
          cmd_4bitsWithout_stb(0x00);//--------------------------2
          // Grid 1
          cmd_without_stb(0x00);// //----------------------------3
          cmd_without_stb(0x00);// //----------------------------4
          cmd_4bitsWithout_stb(0x00);//--------------------------5
          // Grid 2
          cmd_without_stb(wordCalarm); //   Minuts unit //-------6
          cmd_without_stb(wordDalarm); //   Minuts dozens //-----7
          cmd_4bitsWithout_stb(0x00);//--------------------------8
          // Grid 3
          cmd_without_stb(wordEalarm); //  Hours unit   //-------9
          cmd_without_stb(wordFalarm); //  Hours dozens //-------A
          cmd_4bitsWithout_stb(0x00);// //-----------------------B
          // Grid 4
          cmd_without_stb(0x00);//      //-----------------------C
          cmd_without_stb(0x00);//   //--------------------------D
          cmd_4bitsWithout_stb(0x00);// //-----------------------E
          //
      digitalWrite(VFD_stb, HIGH);
      delayMicroseconds(10);
      cmd_with_stb((0b10001000) | 7); //cmd 4
      delay(5); 
}
void readButtonsClock(){
  // Pay attenntion to the Pin: 8. This is the pin will be used as INPUT-PULLUP comming from Arduino
  uint8_t val = 0x00;       // variable to store the read value

  byte array[8] = {0,0,0,0,0,0,0,0};
 
  unsigned char mask = 0b00000001; //our bitmask

    array[0] = 1;
    digitalWrite(VFD_stb, LOW);
    delayMicroseconds(2);
    cmd_without_stb(0b01000010); // cmd 2 //Read Keys;Normal operation; Set pulse as 1/16
    delayMicroseconds(5);
  pinMode(8, INPUT_PULLUP);  // Important this point! Here I'm changing the direction of the pin to INPUT data.
  delayMicroseconds(5);
         for (int8_t z = 0; z < 6; z++){
                   for (int8_t h = 8; h > 0; h--) {
                      digitalWrite(VFD_clk, HIGH);  // Remember wich the read data happen when the clk go from LOW to HIGH! Reverse from write data to out.
                      delayMicroseconds(2);
                     val = digitalRead(VFD_data);
                           if (val & mask){ // if bitwise AND resolves to true
                            array[h] = 1;
                           }
                           else{ //if bitwise and resolves to false
                           array[h] = 0;
                           }
                    digitalWrite(VFD_clk, LOW);
                    delayMicroseconds(2);
                   } 
             
              Serial.print(z);
              Serial.print(" - ");
                        
                    for (int8_t bits = 7 ; bits > -1; bits--) {
                        Serial.print(array[bits]);
                    }
                     
                          if ((z==3) && (array[5] == 1)){
                                if(clockAlarm == false){
                                hours++;
                                }
                                else{
                                wakeHours++;
                                }
                          }
                          if ((z==3) && (array[4] == 1)){
                              if(clockAlarm == false){
                              hours--;
                              }
                              else{
                              wakeHours--;
                              }
                          }
                          if ((z==3) && (array[7] == 1)){
                              if(clockAlarm == false){
                              minutes++;
                              }
                              else{
                              wakeMinutes++;
                              }  
                          }
                          if ((z==3) && (array[6] == 1)){
                              if(clockAlarm == false){
                              minutes--;
                              }
                              else{
                              wakeMinutes--;
                              }  
                          }
                          if((z==2) && (array[2] == 1)){
                              clockAlarm = !clockAlarm;
                          }
                          if((z==2) && (array[3] == 1)){
                            setAlarm = !setAlarm;
                          }
                          if((z==5) && (array[6] == 1)){
                            resetAlarm = !resetAlarm;
                          }        
                          if((z==2) && (array[1] == 1)){
                              if(clockAlarm == true){
                              wakeHours = 0;
                              wakeMinutes = 0;
                              wakeSeconds=0;  // Set count of secs to zero to be more easy to adjust with other clock.
                              }
                            else{
                              hours = 0;
                              minutes = 0;
                              secs=0;  
                            }
                          }
                  Serial.println();
          }  // End of "for" of "z"
      Serial.println();

 digitalWrite(VFD_stb, HIGH);
 delayMicroseconds(2);
 cmd_with_stb((0b10001000) | 7); //cmd 4
 delayMicroseconds(2);
 pinMode(8, OUTPUT); // Important this point! Here I'm changing the direction of the pin to OUTPUT data.
 delay(2); 
}
void comparTime(){
  if (setAlarm == true){
      if((hours == wakeHours) && (minutes == wakeMinutes) && (seconds == wakeSeconds)){
        wakeSeconds = 0x00;
      swapLedAlarm(0b00011011);
      }
      else{
        swapLedAlarm(0b00011110);
      }
  }
  if(resetAlarm == true){
      swapLedAlarm(0b00011111);
  }    
}
void swapLedAlarm(unsigned char alarmLED){
  unsigned char myByte = alarmLED;
    digitalWrite(VFD_stb, LOW);
    delayMicroseconds(20);
    cmd_without_stb(0b01000001);  //Write Data to LED Port. (PT6311 have 5 LED pins: 50,49,48,47,46)
    delayMicroseconds(20);
    //myByte = (statusAlarm);  // The buzzer of this panel is LED3
    cmd_without_stb(myByte);
    delayMicroseconds(20);
    digitalWrite(VFD_stb, HIGH);
    delayMicroseconds(20);
}
void offAllLEDs(){
  //The status On of LED happen applaing the GND on pin of LED. The common is to VCC.
    digitalWrite(VFD_stb, LOW);
    delayMicroseconds(20);
    cmd_without_stb(0b01000001);  //Write Data to LED Port. (PT6311 have 5 LED pins: 50,49,48,47,46)
    delayMicroseconds(20);
    myByte =(0b00011111);  // Deactivate the 5 pins of LED's. They are active when are at level GND.
    cmd_without_stb(myByte);
    delayMicroseconds(20);
    digitalWrite(VFD_stb, HIGH);
    delayMicroseconds(20);
}
void onAllLEDs(){
  //The status On of LED happen applaing the GND on pin of LED. The common is to VCC.
    digitalWrite(VFD_stb, LOW);
    delayMicroseconds(20);
    cmd_without_stb(0b01000001);  //Write Data to LED Port. (PT6311 have 5 LED pins: 50,49,48,47,46)
    delayMicroseconds(20);
    myByte =(0b00000000);  // Active the 5 pins of LED's. They are active when are at level GND.
    cmd_without_stb(myByte);
    delayMicroseconds(20);
    digitalWrite(VFD_stb, HIGH);
    delayMicroseconds(20);
}
void setup() {
  // put your setup code here, to run once:
  pinMode(diodeLED, OUTPUT);
  //pinMode(diodeLED, OUTPUT);
  
  Serial.begin(115200);
  seconds = 0x00;
  minutes =0x00;
  hours = 0x00;

  /*CS12  CS11 CS10 DESCRIPTION
  0        0     0  Timer/Counter1 Disabled 
  0        0     1  No Prescaling
  0        1     0  Clock / 8
  0        1     1  Clock / 64
  1        0     0  Clock / 256
  1        0     1  Clock / 1024
  1        1     0  External clock source on T1 pin, Clock on Falling edge
  1        1     1  External clock source on T1 pin, Clock on rising edge
 */
  // initialize timer1 
  cli();           // clear of all interrupts
  // initialize timer1 
  //noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;// This initialisations is very important, to have sure the trigger take place!!!
  TCNT1  = 0;
  // Use 62499 to generate a cycle of 1 sex 2 X 0.5 Secs (16MHz / (2*256*(1+62449) = 0.5
  OCR1A = 62499;            // compare match register 16MHz/256/2Hz
  //OCR1A = 1000; //Change to low value to accelarate the clock to debug!  // compare match register 16MHz/256/2Hz
  TCCR1B |= (1 << WGM12);   // CTC mode
  TCCR1B |= ((1 << CS12) | (0 << CS11) | (0 << CS10));    // 256 prescaler 
  TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt
 
  
  // Note: this counts is done to a Arduino 1 with Atmega 328... Is possible you need adjust
  // a little the value 62499 upper or lower if the clock have a delay or advance in the time.
    
  //  a=0x33;
  //  b=0x01;

  CLKPR=(0x80);
  //Set PORT
  DDRD = 0xFF;  // IMPORTANT: from pin 0 to 7 is port D, from pin 8 to 13 is port B
  PORTD=0x00;
  DDRB =0xFF;
  PORTB =0x00;

  pt6311_init();

  test_VFD();

  clear_VFD();

  //only here I active the enable of interrupts to allow run the test of VFD
  //interrupts();             // enable all interrupts
  sei();  //set enable interrupt.
}
void loop() {
  // Can use this cycle to teste all segments of VFD
    for(uint8_t i = 0x00; i < 6; i++){
          clear_VFD();
          test_VFD();
          delay(500);
          clear_VFD();
          delay(500);
          animateTheWheels();
    }
       for(uint8_t i=0; i < 2; i++){
        onAllLEDs(); //The buzzer on this panel is bit 2 and the LED is bit 0.
        delay(500);
        offAllLEDs(); //Clear all outputs of 5 LED's pins
        delay(500);
       }
  offAllLEDs();
  clear_VFD();
  while(1){
    if(clockAlarm == false){
      creatUpdateClockInfo();  //This function make the creation of Clock/time info to be sent to VFD grid's.
      delay(100);
    }
    else{
      createUpdateWakeUpAlarm(); //This function make the creation of Alarm/time info to be sent to VFD grid's.
      delay(100);
    } 
    readButtonsClock();  //Routine to read buttons, pay attention to the flag which define Clock or Alarm digits.
    comparTime();  //Here is done the compare of real time to the keeped values of alarm set.    
  }  
}
ISR(TIMER1_COMPA_vect)   {  //This is the interrupt service routine point!
      secs++;
} 
