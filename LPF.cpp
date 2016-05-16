#include "Arduino.h"
#include "LPF.h"


LPF::LPF(){
}

byte LPF::filterOut(byte in){
  int16_t x = in;
  x -= 127;
  x= x<<7;
  int16_t result;  
  result = onepoleNext3(x);   
  result = result>>7;
  result += 127;  
  byte out = result;
  return out;
}

void LPF::filterSetup(int16_t cutoff, int16_t res){
   f = cutoff;
   fb = res + (res*(255-res)>>8);
}

int16_t LPF::onepoleNext(int16_t in){
  //oscillates around 1n15;
  static int16_t buf0 = 0;
  static int16_t buf1 = 0;
  int16_t pre = (in - buf0);
  int16_t pre2 = multQ8n8(fb, (buf0-buf1));
  buf0 += multQ8n8(pre + pre2, f);
  buf1 += multQ8n8(buf0-buf1, f);
  return buf1;
}

int16_t LPF::onepoleNext3(int16_t in){
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

int16_t LPF::multQ8n8(int16_t x, int16_t y){
  return ((int32_t)x*y)>>8;
}
