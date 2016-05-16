#ifndef LPF_h
#define LPF_h

#include "Arduino.h"

/*Class using a Moog type Filter approximation
*/


class LPF{

  public:
     LPF();
     void filterSetup(int16_t cutoff, int16_t res);
     byte filterOut(byte input);
  private:
    int16_t f;
    int16_t fb;
    int16_t onepoleNext(int16_t in);
    int16_t onepoleNext3(int16_t in);
    int16_t multQ8n8(int16_t x, int16_t y);
};

#endif
