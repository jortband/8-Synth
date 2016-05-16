#include "Arduino.h"
#include "ADSR.h"

ADSR::ADSR(){
  AttackDelta = 0;
  ReleaseDelta = 0;
  DecayDelta = 0;
  Sustain = 0;  //watch out sustain should be 255 when 127 is inputted
  Accumulator = 0;
  ADSRstatus = 0;
}

byte ADSR::Update(){
  //updates the ADSR
  if(ADSRstatus == 0){
    return 0; 
  }
  byte level;
  //first check whichever status it has
  switch(ADSRstatus){
    case 1:
      //update the attacktime
      level = updateAttack();
      break;
    case 2:
     //updatedecay
     level = updateDecay();
     break;
    case 3:
     //basically take a rest (sustain)
     level = Sustain;
     break;
    case 4:
     //updaterelease
     level = updateRelease();
     break;
    default:
     //in case I fuck up with programming
     level = 0;
     break;
  }
  return level;
}


void ADSR::Set(byte attackTime, byte decayTime, byte sustainValue, byte releaseTime){
  Sustain = (sustainValue<<1) + 1;
  if(sustainValue == 0){
    Sustain = 0;
  }
  AttackDelta = calculateAttackTime(attackTime);
  DecayDelta = calculateDecayTime(decayTime);
  ReleaseDelta = calculateReleaseTime(releaseTime);
  
}

void ADSR::Attack(){
  ADSRstatus = 1;
}

void ADSR::Release(){
  ADSRstatus = 4;
}

void ADSR::Reset(){
  ADSRstatus = 0;
  Accumulator = 0;
}

byte ADSR::updateAttack(){
  //function used for incrementing the Accumulator when in attackphase
  uint32_t previousaccumulator = Accumulator;
  Accumulator += AttackDelta;
  if(Accumulator<previousaccumulator){
   Accumulator = uint32Max; //max value for a 32-bit unsigned integer
   ADSRstatus =2; //enter release phase
  }
  byte ADSRvalue = Accumulator>>24;  //returns an 8-bit value;
  return ADSRvalue;
}

byte ADSR::updateDecay(){
  //function used for decrementing the Accumulator when in decay phase
  uint32_t previousaccumulator = Accumulator;
  Accumulator -= DecayDelta;
  byte currentlevel = Accumulator>>24;
  if(Accumulator>previousaccumulator || currentlevel<=Sustain){
    uint32_t thesustain = Sustain;
    Accumulator = thesustain<<24;
    ADSRstatus = 3; //enter sustain phase
  }
  byte ADSRvalue = Accumulator>>24;
  
  return ADSRvalue;
  
}

byte ADSR::updateRelease(){
  //function for decrementing the Accumulator when in decay phase
  uint32_t previousaccumulator = Accumulator;
  Accumulator -= ReleaseDelta;
  if(Accumulator>previousaccumulator){
    //when the accumulator has overflown (gone below zero)
    Accumulator = 0;
    ADSRstatus = 0;  //set ADSR to off
  }
  byte ADSRValue = Accumulator>>24;
  return ADSRValue;
}

uint32_t ADSR::calculateAttackTime(byte attackTime){
  //function for calculating the decay time delta
  //requirement should go from 0 to 10000ms
  //attackTime input ranges from 0 to 127 (midi support)
  if(attackTime == 0){
    //if attackTime is 0 just max out the delta
     uint32_t theoutput = uint32Max -100;
     return theoutput;
  }
  
  uint32_t theDelta = uint32Max/pgm_read_dword(&(StepsConversionArray[attackTime]));
  return theDelta;
}

uint32_t ADSR::calculateDecayTime(byte decayTime){
  //function for calculating the decay time delta
  if(decayTime == 0){
    //just fill the thing, doesnt matter how much
    uint32_t theoutput = uint32Max -100;
    return theoutput;
  }
  uint32_t thesustain = Sustain;
  thesustain = thesustain<<24;
  uint32_t theDelta = (uint32Max - thesustain)/pgm_read_dword(&(StepsConversionArray[decayTime]));
  return theDelta;
}

uint32_t ADSR::calculateReleaseTime(byte releaseTime){
  //function for calculating the Release time delta
  if(releaseTime == 0){
    //just fill the thing, doesnt matter how much
    uint32_t theoutput = uint32Max -100;
    return theoutput;
  }
  
  uint32_t thesustain = Sustain;
  thesustain = thesustain<<24;
  uint32_t theDelta = thesustain/pgm_read_dword(&(StepsConversionArray[releaseTime]));
  return theDelta;
}





