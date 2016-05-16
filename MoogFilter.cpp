#include "Arduino.h"
#include "MoogFilter.h"

MoogFilter::MoogFilter(int32_t samplefreq){
  fs = samplefreq;
}

void MoogFilter::setupFilter(int32_t Q, int32_t F){
  //TODO: Fix the F should provide a range from 20 tot 10Khz
  //gonna mix some bases
  int32_t fQ8n24 = division8Q24(F<<1, fs); 
  int32_t fsq8n24 = multiply8Q24(fQ8n24,fQ8n24);
  int32_t fq = Q<<16; //Q is max 256, converts to base 24
  
  //now setup p&k
  /*  k=3.6*f-1.6*f*f -1;
   *  3.6Q24 = 60397978
   *  1.6Q24 = 26843546
   *  -1 = -16777216
   */
  int32_t kQ8n24 = multiply8Q24((int32_t)60397978,fQ8n24);
  kQ8n24 -= multiply8Q24((int32_t)60397978,fsq8n24);
  int32_t pQ8n24 = (int32_t) kQ8n24>>1; //p = (k+1)*0.5 (have not removed the one yet in k)
  kQ8n24 -= 16777216; //-1
  
  //t = (1.0-p)*1.386249;
  // 1.386249Q24 = 23257399 
  int32_t t = multiply8Q24(((int32_t) 16777216 -pQ8n24), (int32_t)23257399); //TODO: perhaps Q8n8
  int32_t tsq = multiply8Q24(t,t);  ////TODO: perhaps Q8n8
  //t2 = 12.0+t*t;
  //12Q24 = 201326592
  int32_t t2 = tsq + (int32_t)201326592;  //TODO: perhaps Q8n8
  
  //r = res*(t2+6.0*t)/(t2-6.0*t);
  //6Q24 = 100663296
  int32_t t6 = multiply8Q24((int32_t)100663296,t);  //TODO: perhaps Q8n8
  int32_t r1 = t2 + t6;
  int32_t r2 = t2 - t6;
  //r1 and r2 are a factor 2^3 to large for the Q24 division
  int32_t r3 =  division8Q24(r1>>3, r2>>3);
  int32_t rQ8n24 = multiply8Q24(fq,r3);
  
  //Now translate all the variables to Q8N8
  k = kQ8n24>>16;
  p = pQ8n24>>16;
  r = rQ8n24>>16;
  int32_t pQ16 = pQ8n24>>8;
}

byte MoogFilter::processFilter(byte input){

   //setup variables
  static int32_t x = 0;
  static int32_t y1 = 0;
  static int32_t y2 = 0;
  static int32_t y3 = 0;
  static int32_t y4 = 0;
  static int32_t oldx = 0;
  static int32_t oldy1 = 0;
  static int32_t oldy2 = 0;
  static int32_t oldy3 = 0;

  //setup input
  int32_t in = (int32_t)input<<8; //convert to Q8N8 base, takes 8us
  in -= 256;  //convert to something between 1 and -1
  x = in - multiply8Q8(r, y4); 

  
  //takes about 12 us total
  //this took about 112 us
  y1 = onepole(x,oldx,y1);
  y2 = onepole(y1,oldy1,y2);
  y3 = onepole(y2,oldy2,y3);
  y4 = onepole(y3,oldy3,y4);
  
  //Clipper band limited sigmoid
  // y4-=(y4*y4*y4)/6.0;
  //1/6 in Q8 = 43
  /*
  int32_t ya = multiply8Q8(y4,(int32_t)43);  //TODO see if this is neccessary not doing this is 12us faster
  ya = multiply8Q8(y4,ya);
  ya = multiply8Q8(y4,ya);
  y4 -= ya;*/
  
  oldx= x;
  oldy1 = y1;
  oldy2 = y2;
  oldy3 = y3;
  uint32_t res = y4 +(int32_t)(1<<8);
  byte fres = (uint32_t)res>>8;
  return fres;
}

int32_t MoogFilter::onepole(int32_t x, int32_t x1, int32_t y){
  
  //x*p + x1*p - k*y
  int32_t result = multiply8Q8(x,p) + multiply8Q8(x1,p) - multiply8Q8(k,y);
  return result;
}

int32_t MoogFilter::division8Q24(int32_t num, int32_t divs){
  //now massage signage into it
  uint32_t sign = (int32_t) (num>>16*divs>>16)>>31;
  //Serial.print("sign: ");
  //Serial.println(sign, BIN);
  uint32_t xu = abs(num);
  uint32_t yu = abs(divs);
  uint32_t mainpart = (xu/yu<<24);
  uint32_t remainder = (xu%yu)<<6;//number will not be larger then 3 or -3
  uint32_t correction1 = remainder/yu;
  correction1 = correction1<<18;
  uint32_t remainder2 = remainder%yu<<6;
  uint32_t correction2 = (remainder2/yu)<<12;
  uint32_t remainder3 = (remainder2%yu<<6);
  uint32_t correction3 = (remainder3/yu)<<6;
  uint32_t remainder4 = (remainder3%yu<<6);
  uint32_t correction4 = (remainder4/yu);
  int32_t result = mainpart + correction1 + correction2 + correction3 + correction4;
  result ^=sign;
  return result;
}

int32_t MoogFilter::multiply8Q8(int32_t x, int32_t y){
  int32_t res = x*y;
  res = res>>8;
  return res;
}

int32_t MoogFilter::multiply8Q24(int32_t x, int32_t y){
  int32_t x8Q = x>>24;
  x8Q*= y;  //multiplied already. Now do the fractional part
  uint32_t sign = (int32_t) x8Q>31;
  uint32_t y8Q = y>>24;
  uint32_t xQ24 = (uint32_t) x<<8>>8;
  uint32_t resQ24 = xQ24 * y8Q;
  //this part does 1.56 * 2;
  //now I have to do the rest. 0,56 * 0,3
  //lets split up x for doing so
  uint32_t xQ24m8 = (uint32_t)xQ24>>16;
  uint32_t xQ24m16 = (uint32_t)(xQ24<<16)>>24;
  uint32_t xQ24m24 = (uint32_t)(xQ24<<24)>>24;
  uint32_t yQ24 = (uint32_t)(y<<8)>>8;
  //now multiply the fractional parts
  uint32_t xQ24m8res = (xQ24m8 * yQ24)>>8;
  uint32_t xQ24m16res = (xQ24m16 * yQ24)>>16;
  uint32_t xQ24m24res = (xQ24m24 * yQ24)>>24;
  //total it
  int32_t xQ24res = xQ24m8res + xQ24m16res + xQ24m24res;
  xQ24res ^= sign;
  int32_t result = x8Q + resQ24 + xQ24res;
  return result;
}
