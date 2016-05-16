#ifndef MoogFilter_h
#define MoogFilter_h

#include "Arduino.h"

/*Class using a Moog type Filter approximation
*/


class MoogFilter{

  public:
     MoogFilter(int32_t samplefreq);
     void setupFilter(int32_t Q, int32_t F);
     byte processFilter(byte input);
  private:
    int32_t fs;
    int32_t fc;
    int32_t p;
    int32_t k;
    int32_t r;

    //onepole filter function
    int32_t onepole(int32_t x, int32_t x1, int32_t y);
    //math functions
    int32_t division8Q24(int32_t num, int32_t divs);
    int32_t multiply8Q8(int32_t x, int32_t y);
    int32_t multiply8Q24(int32_t x, int32_t y);
};

#endif
