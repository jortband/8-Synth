#ifndef SquareWave_h
#define SquareWave_h

#include "Arduino.h"

/*Main class for generating a square wave.
Includes Frequency Control
PWM (8, selectable duty cycles)
Reset
Output/Update function
begin/setup function
TODO:Add a check reset function, perhaps integrate within Update? Return a Boolean to see if a reset is required
*/

//array containing the values needed for detuning.
/*
const uint16_t detuneArray[256] PROGMEM = {15464,15471,15479,15486,15493,15500,15507,15514,15521,15528,15535,15542,15549,15556,15563,15570,15577,15584,15592,15599,15606,15613,15620,15627,15634,15641,15648,15656,15663,15670,15677,15684,15691,15698,15705,15713,15720,15727,15734,15741,15748,15756,15763,15770,15777,15784,15791,15799,15806,15813,15820,15827,15835,15842,15849,15856,15863,15871,15878,15885,15892,15899,15907,15914,15921,15928,15936,15943,15950,15957,15965,15972,15979,15987,15994,16001,16008,16016,16023,16030,16037,16045,16052,16059,16067,16074,16081,16089,16096,16103,16111,16118,16125,16133,16140,16147,16155,16162,16169,16177,16184,16191,16199,16206,16214,16221,16228,16236,16243,16250,16258,16265,16273,16280,16287,16295,16302,16310,16317,16324,16332,16339,16347,16354,16362,16369,16377,16384,16391,16399,16406,16414,16421,16429,16436,16444,16451,16459,16466,16474,16481,16489,16496,16504,16511,16519,16526,16534,16541,16549,16556,16564,16571,16579,16586,16594,16602,16609,16617,16624,16632,16639,16647,16654,16662,16670,16677,16685,16692,16700,16708,16715,16723,16730,16738,16746,16753,16761,16768,16776,16784,16791,16799,16807,16814,16822,16830,16837,16845,16853,16860,16868,16876,16883,16891,16899,16906,16914,16922,16929,16937,16945,16953,16960,16968,16976,16983,16991,16999,17007,17014,17022,17030,17038,17045,17053,17061,17069,17076,17084,17092,17100,17107,17115,17123,17131,17139,17146,17154,17162,17170,17178,17185,17193,17201,17209,17217,17225,17232,17240,17248,17256,17264,17272,17279,17287,17295,17303,17311,17319,17327,17335,17342,17350,17358,17366
};*/


//-1 semitone to +1 semitones
const uint16_t detuneArrayCents[256] PROGMEM = 
{
  14596,14610,14623,14636,14650,14663,14676,14690,14703,14716,14730,14743,14757,14770,14784,14797,14810,14824,14837,14851,14864,14878,14892,14905,14919,14932,14946,14959,14973,14987,15000,15014,15028,15041,15055,15069,15082,15096,15110,15124,15137,15151,15165,15179,15193,15206,15220,15234,15248,15262,15276,15290,15304,15317,15331,15345,15359,15373,15387,15401,15415,15429,15443,15457,15471,15486,15500,15514,15528,15542,15556,15570,15584,15599,15613,15627,15641,15656,15670,15684,15698,15713,15727,15741,15756,15770,15784,15799,15813,15827,15842,15856,15871,15885,15899,15914,15928,15943,15957,15972,15987,16001,16016,16030,16045,16059,16074,16089,16103,16118,16133,16147,16162,16177,16191,16206,16221,16236,16250,16265,16280,16295,16310,16324,16339,16354,16369,16384,16399,16414,16429,16444,16459,16474,16489,16504,16519,16534,16549,16564,16579,16594,16609,16624,16639,16654,16670,16685,16700,16715,16730,16746,16761,16776,16791,16807,16822,16837,16853,16868,16883,16899,16914,16929,16945,16960,16976,16991,17007,17022,17038,17053,17069,17084,17100,17115,17131,17146,17162,17178,17193,17209,17225,17240,17256,17272,17287,17303,17319,17335,17350,17366,17382,17398,17414,17429,17445,17461,17477,17493,17509,17525,17541,17557,17573,17589,17605,17621,17637,17653,17669,17685,17701,17717,17733,17749,17766,17782,17798,17814,17830,17847,17863,17879,17895,17912,17928,17944,17961,17977,17993,18010,18026,18042,18059,18075,18092,18108,18125,18141,18158,18174,18191,18207,18224,18240,18257,18274,18290,18307,18324,18340,18357,18374,18390,18407
};


//-12 semitones to +12 semitones
const uint16_t detuneArray[256] PROGMEM ={
  8192,8237,8282,8327,8373,8419,8465,8511,8558,8604,8652,8699,8746,8794,8842,8891,8940,8988,9038,9087,9137,9187,9237,9288,9339,9390,9441,9493,9545,9597,9649,9702,9755,9809,9862,9916,9971,10025,10080,10135,10191,10246,10303,10359,10416,10473,10530,10588,10645,10704,10762,10821,10880,10940,11000,11060,11121,11181,11243,11304,11366,11428,11491,11554,11617,11680,11744,11809,11873,11938,12004,12069,12135,12202,12269,12336,12403,12471,12539,12608,12677,12746,12816,12886,12957,13028,13099,13171,13243,13315,13388,13461,13535,13609,13684,13758,13834,13909,13986,14062,14139,14216,14294,14373,14451,14530,14610,14690,14770,14851,14932,15014,15096,15179,15262,15345,15429,15514,15599,15684,15770,15856,15943,16030,16118,16206,16295,16384,16474,16564,16654,16746,16837,16929,17022,17115,17209,17303,17398,17493,17589,17685,17782,17879,17977,18075,18174,18274,18374,18474,18575,18677,18779,18882,18985,19089,19194,19299,19404,19511,19617,19725,19833,19941,20050,20160,20270,20381,20493,20605,20718,20831,20945,21060,21175,21291,21407,21525,21642,21761,21880,22000,22120,22241,22363,22485,22608,22732,22856,22982,23107,23234,23361,23489,23617,23747,23877,24007,24139,24271,24404,24537,24671,24806,24942,25079,25216,25354,25493,25632,25772,25914,26055,26198,26341,26485,26630,26776,26923,27070,27218,27367,27517,27668,27819,27971,28124,28278,28433,28589,28745,28902,29060,29220,29379,29540,29702,29864,30028,30192,30357,30524,30691,30859,31028,31197,31368,31540,31712,31886,32060,32236,32412,32590,32768,32947
};

class SquareWave{

  public:
     SquareWave();
     void Begin(uint32_t delta);
     byte Update();
     void Reset();
     void End();
     void SetDetune(byte detuneAmount);
     void SetDetuneMod(byte ADSR, byte LFO);
     void setPWM(byte Amount);
     void SetGlide(byte glideval);
     void updateGlide();
     uint32_t returnDelta();
     uint32_t returnDetuneDelta();
     uint32_t FixedMathMultiply32(byte sign, uint32_t x, uint32_t y);
  private:
     //uint32_t FixedMathMultiply32(byte sign, uint32_t x, uint32_t y);
     volatile uint8_t PulseWidth;
     volatile uint32_t Accumulator;
     volatile uint32_t FreqDelta;
     byte PWMReg[8];
     byte Detune;
     uint32_t detuneDelta;
     uint32_t BaseDelta;
     uint32_t ModDelta;
     uint32_t finalDelta;
     boolean glideOn;
     byte glide;
     boolean posGlide;
     boolean glideStatus;
     byte ADSRDetune;
     byte LFODetune;     
};

#endif
