#include "Arduino.h"
#include "LFO.h"

LFO::LFO(){
  accumulator = 0;
  delta =  pgm_read_dword(&(rateArray[0]));
  waveSelector = 0;
  for(int i = 0; i<256; i++){
    wave[i] = pgm_read_byte(&(waveArray[waveSelector][i]));
  }
}

byte LFO::Update(){
  byte val = wave[accumulator>>24];
  accumulator += delta;
  return val;
}

void LFO::setRate(byte rate){
  delta = pgm_read_dword(&(rateArray[rate]));
}

void LFO::setWave(byte waveSel){
  waveSelector = waveSel;
  
  for(int i = 0; i<256; i++){
    wave[i] = pgm_read_byte(&waveArray[waveSelector][i]);
  }
}

