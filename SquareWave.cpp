#include "Arduino.h"
#include "SquareWave.h"

SquareWave::SquareWave(){
  //setting up the PWM byte registers. These are read to result in a pulse with a specific duty cycle
  /*PWMReg[0] = B00000001; //12.5%
  PWMReg[1] = B00000011; //25%
  PWMReg[2] = B00000111; //37.5%
  PWMReg[3] = B00001111; //50%
  PWMReg[4] = B00011111; //62.5%%
  PWMReg[5] = B00111111; //75%
  PWMReg[6] = B01111111; //87.5%
  PWMReg[7] = B00000000; //0%, this one is quite useless as it is a steady DC wave, but is nevertheless included, might be usefull when turning a note off,
  */
  PulseWidth = 127;
  Accumulator = 0;
  FreqDelta = 0;
  BaseDelta = 0;
  detuneDelta = 0;
  ModDelta = 0;
  finalDelta = 0;
  Detune = 127;
  ADSRDetune = 127;
  LFODetune = 127;

  //glide
  glideOn = false;
  glide = 0;
  posGlide = true;
  glideStatus = false;
}

void SquareWave::Begin(uint32_t delta){
  //set base delta and delta to the same, this makes sure it is a lean begin thing
  //FreqDelta = delta;
  Accumulator = 0;
  BaseDelta = delta;
  SetDetune(Detune);
  
}

uint32_t SquareWave::returnDelta(){
  return FreqDelta;
}

uint32_t SquareWave::returnDetuneDelta(){
  return detuneDelta;
}

uint8_t SquareWave::Update(){

  //static uint32_t accum = 0;
  static uint32_t delta = 30236569; //freq of 220Hz
  uint8_t out = Accumulator>>24;
  uint8_t result = 0;
  if(out >= PulseWidth){
    result = 255;
  }

  
  //TODO: Check this, makes a weird oscillation sound.
  //result = (Accumulator>>31)*255;
  Accumulator += FreqDelta;

  return result;
}

void SquareWave::End(){
  Accumulator=0;
  FreqDelta = 0;
}

void SquareWave::setPWM(byte amount){
  PulseWidth = amount; //max 3-bit input
}

void SquareWave::SetDetune(byte detuneAmount){
  //calculate the added delta of the original, meant for midi input
  //easiest is to calculate the different tuning words at once through fixed math multiplication
  //basically calculation should be Delta = baseDelta * detune * ADSRmod * LFOMod
  //perhaps cap midi note to note 108 (max range of piano)
  //max note Delta = 575320702 for note 108 (can be contained in a 30 bit number), meaning that overflow should be dealt with
  //Max modulation is a semitone -1 to +1 f*a^x a = constant, a^-1
  Detune = detuneAmount;
  //detuneDelta = FixedMathMultiply32(14, BaseDelta, pgm_read_word(&(detuneArray[Detune])));

  finalDelta = FixedMathMultiply32(14, BaseDelta, pgm_read_word(&(detuneArray[Detune])));
  if(!glide || (finalDelta == detuneDelta)){
    //if glide is off, or the same note is repeated, the current delta is the finaldelta
    detuneDelta = finalDelta;
   }else{
    //else calculate negative or positive value
    glideStatus = true;
    if(finalDelta>detuneDelta){
      posGlide = true;  //determines to glide positively or negatively
    }else{
      posGlide = false;
    }
   }

  
  FreqDelta = FixedMathMultiply32(14, detuneDelta, pgm_read_word(&(detuneArray[ADSRDetune])));
  FreqDelta = FixedMathMultiply32(14, FreqDelta, pgm_read_word(&(detuneArray[LFODetune])));

  
}

void SquareWave::SetDetuneMod(byte ADSR, byte LFO){
 //function for modulation detuning, adds to other modulation
 //takes detuning from -127 tot 127
 //TODO: Add mod wheel support.
 //set new detune constants
 //digitalWrite(13,HIGH);
 ADSRDetune = ADSR;
 LFODetune = LFO;
 //SetDetune(Detune);
 //FreqDelta +=2000;

 //calculate detune amount
 FreqDelta = FixedMathMultiply32(14, detuneDelta, pgm_read_word(&(detuneArray[ADSRDetune])));
 FreqDelta = FixedMathMultiply32(14, FreqDelta, pgm_read_word(&(detuneArray[LFODetune])));
 //digitalWrite(13,LOW);
}

void SquareWave::SetGlide(byte glideval){
  glide = glideval;
  glideOn=true;
  
  if(glide == 0){
    glideOn = false;
  }
}

void SquareWave::updateGlide(){
  //function managing glide effect
  uint32_t tempD = 0;
  if(posGlide && glideStatus){
    tempD = FixedMathMultiply32(14, detuneDelta, pgm_read_word(&(detuneArrayCents[glide+127])));
    if(tempD>finalDelta){
      detuneDelta = finalDelta;
      glideStatus = false;
    }else{
      detuneDelta = tempD;
    }
  }else{
    tempD = FixedMathMultiply32(14, detuneDelta, pgm_read_word(&(detuneArrayCents[127-glide])));
    if(tempD<finalDelta){
      detuneDelta = finalDelta;
      glideStatus = false;
    }else{
      detuneDelta = tempD;
    }
  }
  /*
  Serial.print(glide);
  Serial.print("\t");
  Serial.print(glideOn);
  Serial.print("fd: ");
  Serial.print(finalDelta);
  Serial.print("\t detD: ");
  Serial.println(detuneDelta);*/

  //TODO: Rewrite for less overhead
   FreqDelta = FixedMathMultiply32(14, detuneDelta, pgm_read_word(&(detuneArray[ADSRDetune])));
   FreqDelta = FixedMathMultiply32(14, FreqDelta, pgm_read_word(&(detuneArray[LFODetune])));
}

uint32_t SquareWave::FixedMathMultiply32(byte sign, uint32_t x, uint32_t y){
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
