#include <avr/interrupt.h>   //add interrupts
#include <avr/io.h>          //no idea
#include <avr/pgmspace.h>    //to store values in progmem
#include "Arduino.h"
#include "SquareWave.h"
#include "TableWave.h"
#include "MidiLib.h"
#include "ADSR.h"
#include "LFO.h"
#include "MoogFilter.h"
#include "LPF.h"


//TODO: Make a more integrated midi to control library. Use midi output to control audio output directly (put the miditoFreq in a class).

const uint32_t midiToFreq[128] PROGMEM = {
1123673,1190490,1261281,1336280,1415740,1499924,1589114,1683608,1783720,1889786,2002158,2121213,2247346,2380981,2522561,2672560,2831479,2999848,3178228,3367215,3567440,3779571,4004316,4242425,4494693,4761961,5045122,5345121,5662958,5999695,6356456,6734430,7134880,7559142,8008632,8484851,8989386,9523923,10090245,10690242,11325917,11999391,12712912,13468861,14269761,15118285,16017265,16969701,17978772,19047845,20180489,21380484,22651833,23998781,25425823,26937721,28539522,30236570,32034530,33939402,35957544,38095691,40360978,42760967,45303666,47997563,50851646,53875442,57079043,60473140,64069060,67878804,71915088,76191381,80721957,85521934,90607333,95995125,101703292,107750885,114158086,120946279,128138119,135757608,143830176,152382763,161443913,171043868,181214666,191990251,203406585,215501770,228316172,241892558,256276238,271515216,287660351,304765526,322887827,342087736,362429332,383980501,406813170,431003540,456632344,483785116,512552476,543030432,575320702,609531052,645775654,684175473,724858663,767961002,813626340,862007080,913264688,967570232,1025104952,1086060865,1150641405,1219062103,1291551308,1368350945,1449717327,1535922005,1627252680,1724014160
};

MidiLib theMidi;
byte currentnote; //to make sure that only on the current on note the off trigger has effect.
SquareWave SquareOsc;
ADSR ADSREnvelope;
LFO lfo;
MoogFilter filter((int32_t)31250);
LPF lpf;
volatile byte ADSRValue;
volatile byte LFOVal;
volatile byte VolVal;
volatile byte oscVal;


//FILTER TRY OUT VARIABLES
uint32_t fmultiplier = 0;
uint32_t fdiv = 0;
volatile uint32_t prevVal = 0;

/*define switches*/
//LFO to SQ
boolean LFOPWMSq = false;
boolean LFOVolSq = false;
boolean LFOPitchSq = false;
//ADSR to SQ
boolean ADSRPWMsq = false;
boolean ADSRVolSq = false;  //<-perhaps make this always true?
boolean ADSRPitchSq = false;

uint64_t something = 50;

void setup(){
  Serial.begin(115200);
  ////////experimenter section\\\\\\\\\\
    ADSREnvelope.Set(0,10,100,50);
    pinMode(13,OUTPUT);
    digitalWrite(13,LOW);
    pinMode(A0,INPUT);
    //setting LFO
    //TODO:CHECK RATE
    lfo.setRate(100);
    lfo.setWave(3);
    const uint32_t delta = 30236569;
    SquareOsc.Begin(delta);
    //TODO:Check this one
    SquareOsc.SetDetuneMod(127,128);
    SquareOsc.SetGlide(100);
    
    filter.setupFilter(0, 0);
    
    //filterSetup(200,0);
    lpf.filterSetup(200,0);
    byte filterres = 0;
    long time1=micros();
    for(int i = 0 ; i<1000; i++){
      filterres = filter.processFilter(255);
    }
    long time2 = micros();

    Serial.print("it took; ");
    Serial.print(time2-time1);
    Serial.println("us");
    delay(200);
  ///////end of experimenter section\\\\\\\\
  noInterrupts();
  setuppins();
  setupinterrupts();
  setupMidi();
  interrupts();
  digitalWrite(13,HIGH);
  SquareOsc.SetGlide(1);
  noteon(20,127,0);
}


ISR(TIMER1_COMPA_vect){
   /*Handle modulation in here, operates at 3125HZ*/
   ADSRValue = ADSREnvelope.Update();
  LFOVal = lfo.Update();
  //glide
  SquareOsc.updateGlide();
  
  /*
    uint32_t tempval = (uint32_t) LFOVal<<16;
    
  //do some maths here
  //use 16 bit for precision
  //first calculate the additive
  uint32_t add = FixedMathMultiply32(16, prevVal, fmultiplier);

  //add the additive to the current result
  tempval += add;
  Serial.println(tempval>>16);
  
  //now multiply it by the divider
  uint32_t fres = FixedMathMultiply32(16,tempval, fdiv);
  if(fres > 16777215){
    fres = 16777215;
  }
  //Serial.println(fres);
  prevVal = fres;
  fres= fres>>16;*/
  
  //LFOVal = fres>>16;
  
  
  //TESTED
  //Vol mod
  VolVal = ADSRValue;
  if(LFOVolSq){
    uint16_t tempv = VolVal * LFOVal;
    tempv = tempv>>8;
    VolVal = tempv;//tempv>>1;  //divide by two
  }

  
    //PWM Mod
    if(ADSRPWMsq && LFOPWMSq){
      SquareOsc.setPWM((ADSRValue+LFOPWMSq)>>1);
    }else{
      if(ADSRPWMsq){
        SquareOsc.setPWM(ADSRValue);
      }
      if(LFOPWMSq){
        SquareOsc.setPWM(LFOVal);
      }
    }
    
   if(LFOPitchSq && ADSRPitchSq){
    //do both, LFO oscillates pitch, ADSR increments pitch
    SquareOsc.SetDetuneMod((ADSRValue>>1) + 127, LFOVal);
  }
  if(LFOPitchSq && !ADSRPitchSq){
    SquareOsc.SetDetuneMod(127,LFOVal);
  }
  if(!LFOPitchSq && ADSRPitchSq){
    SquareOsc.SetDetuneMod((ADSRValue>>1) + 127,127);
  }
  
}

 
ISR(TIMER2_OVF_vect){
  //fires 31250 times per second.
  uint16_t value= SquareOsc.Update();
  value = lpf.filterOut(value);
  value *= VolVal;
  value = value>>8;
  byte finalvalue = value;

  
  OCR2A = LFOVal ;
  OCR2B = finalvalue;
  
  
}


void loop(){
  theMidi.readMidi();
  static long prevT = millis();
  static boolean state = true;
  static boolean count = true;

  /*
  if(millis() - prevT > 2000){
    if(count){
        if(state){
        noteon(20,127,0);
        state = false;
      }else{
        noteoff(32,127,0);
        state = true;
        count = !count;
      }
      
    }else{
      if(state){
      noteon(32,127,0);
      state = false;
      }else{
        noteoff(44,127,0);
        state = true;
        count = !count;
      }
    }
    
    prevT = millis();   

  
    
  }*/
  lpf.filterSetup(analogRead(A0)>>2, 0);
  
  
}


void setuppins(){
  //setting the output to the PWM pins
  pinMode(9,OUTPUT);
  pinMode(10,OUTPUT);
}

void setupinterrupts(){
  //function for setting up the basic interrupt triggers for waveform gen.
  pinMode(3,OUTPUT); //timer2
  pinMode(5,OUTPUT);
  pinMode(6,OUTPUT);
  pinMode(8,OUTPUT);
  pinMode(11,OUTPUT); //timer2
  
  //OCRA+B
  pinMode(9,OUTPUT); 
  pinMode(10,OUTPUT);
  
  
  //set the 8-bit pwm's to phase correct pwm generating a frequency of 31250
  TCCR2A = _BV(COM2A1) | _BV(COM2B1)| _BV(WGM20);  //setting up the correct compare match in phase correct mode
  TCCR2B = _BV(CS20); //set the prescaler to 1 (no prescaling)
  TIMSK2 = 0x01;  //enable Timer 2 overflow interrupt.
  OCR2A = 180;  //set to random values, pin 11
  OCR2B = 50;    //set to random values, pin3

  
  //setup timer1 for modulation, CTC mode (no pwm), at 3125 HZ (set counter at 5120-1) (16.000.000/ 5120 = 3125)
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;

  OCR1A = 15999; //5199 for 3125hz, 15999 for 1000hz
  
  TCCR1B |= B00001001; //set TCCR1B with WGM12 && CS10
  //TCCR1B = _BV(WGM12) | _BV(CS10);  //set to CTC mode with pins disconnected and OCR1A as top and set prescaler to 1
  TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt
  /*OCR1AH = B00010011; //31999 for trigger at 500, 5119 to trigger at 3125
  OCR1AL = B11111111;*/
  //OCR1A = 5199;
  //OCR1A = 65500;
  //OCR1A
  OCR1B = 5; //random value
}

void setupMidi(){
  theMidi.setnoteOnHandle(noteon);
  theMidi.setnoteOffHandle(noteoff);
  currentnote = 0;
}

void noteon(byte note, byte velocity, byte channel){
  digitalWrite(13,HIGH);
  SquareOsc.Begin(pgm_read_dword(&(midiToFreq[note])));
  currentnote = note;
  ADSREnvelope.Attack();
  
}

void noteoff(byte note, byte velocity, byte channel){
  if(currentnote == note){
    digitalWrite(13,LOW);
    //SquareOsc.End();
    ADSREnvelope.Release();
  }
}






//placed here for filter calculation
uint32_t FixedMathMultiply32(byte sign, uint32_t x, uint32_t y){
  //multiply a 32 bit number with another 32-bit number, without causing overflow
  //first break x into two different 32-bit numbers
  uint32_t x_LSB = (x<<16)>>16;
  uint32_t x_MSB = (x>>16);
  //multiply the numbers by each other
  x_LSB *= y;
  x_MSB *= y;
  
  //scale back by the significance of the LSB
  x_LSB = x_LSB>>sign;
  
  //now transport the least significant part of x_MSB to the x_LSB
  uint32_t x_MSBL = x_MSB<<16;
  x_MSBL = x_MSBL>>sign;
  x_LSB += x_MSBL;

  //x_MSB most significant bit to 
  x_MSB = (x_MSB&4294901760)>>(sign);
  x_MSB = x_MSB<<16;
  
  //combine x_LSB & x_MSB
  uint32_t theoutput = x_LSB + x_MSB;
  
  return theoutput;
}

















/////////////FILTER TRYOUT\\\\\\\\\\\\\\
/*
 * Mozzi inspired filter, should see if I can cascade filters
 */
int16_t f;
int16_t fb;
  

byte filterOut(byte in){
  int16_t x = in;
  x -= 127;
  x= x<<7;
  int16_t result;
  result = onepoleNext3(x);
  //result = onepoleNext(result);
  result = result>>7;
  result += 127;
  
  byte out = result;
  return out;
}

void filterSetup(int16_t cutoff, int16_t res){
   f = cutoff;
   fb = res + (res*(255-res)>>8);
}



int16_t onepoleNext(int16_t in){
  //oscillates around 1n15;
  static int16_t buf0 = 0;
  static int16_t buf1 = 0;
  int32_t pre = (in - buf0);
  int32_t pre2 = multQ8n8(fb, (buf0-buf1));
  buf0 += multQ8n8(pre + pre2, f);
  buf1 += multQ8n8(buf0-buf1, f);
  return buf1;
}

int32_t multQ8n8(int16_t x, int16_t y){
  return ((int32_t)x*y)>>8;
}

int16_t onepoleNext4(int16_t in){
  //oscillates around 1n15;
  static int16_t buf0 = 0;
  static int16_t buf1 = 0;
  static int32_t buf2 = 0;
  static int32_t buf3 = 0;
  int16_t pre = (in - buf0);
  int16_t pre2 = multQ8n8(fb, (buf0-buf1));
  buf0 += multQ8n8(pre + pre2, f);
  buf1 += multQ8n8(buf0-buf1, f);
  buf2 += multQ8n8(buf1 - buf2, f);
  buf3 += multQ8n8(buf2 - buf3, f);
  return buf3;
}

int16_t onepoleNext3(int16_t in){
  //oscillates around 1n15;
  static int16_t buf0 = 0;
  static int16_t buf1 = 0;
  static int32_t buf2 = 0;
  int16_t pre = (in - buf0);
  int16_t pre2 = multQ8n8(fb, (buf0-buf1));
  buf0 += multQ8n8(pre + pre2, f);
  buf1 += multQ8n8(buf0-buf1, f);
  buf2 += multQ8n8(buf1 - buf2, f);
  return buf2;
}

