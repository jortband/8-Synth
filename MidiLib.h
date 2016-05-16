#ifndef MidiLib_h
#define MidiLib_h
#include "Arduino.h"
#include "stdlib.h"

typedef void (*CallbackFunction) (byte arg1, byte arg2, byte arg3);
const byte noteonMessage = B1001;
const byte noteoffMessage =  B1000;
const byte aftertouchMessage = B1010;
const byte controlchangeMessage = B1011;
const byte pitchbendMessage = B1110;
   
   
class MidiLib{
 public:
    MidiLib();
    void readMidi();
    void setnoteOnHandle(CallbackFunction thecallbackfunction);
    void setnoteOffHandle(CallbackFunction thecallbackfunction);
   
 private: 
    CallbackFunction noteOnFunction;
    CallbackFunction noteOffFunction;
    byte incomingmidicommandbyte;
    byte incomingmidinote;
    byte incomingmidivelocity;
};


#endif
