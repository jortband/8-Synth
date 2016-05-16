#include "Arduino.h"
#include "MidiLib.h"

MidiLib::MidiLib(){
  incomingmidicommandbyte = 0;
  incomingmidinote = 0;
  incomingmidivelocity = 0;
  noteOnFunction = NULL;
  noteOffFunction = NULL;
}

void MidiLib::readMidi(){
  //TODO: add CC, pitchbend and PC support.
  //TODO: Make it independent from the Serial Library (access UART directly), so it will not intefere with timers.
  //only on program change a 2byte value is send.
  //check how large the incoming midi message is.
  while(Serial.available() >=1){
   //if more than one byte is available. 
   byte incomingbyte = Serial.read();
   byte midicommand = incomingbyte>>4;
   
   if(midicommand == noteonMessage|| midicommand == noteoffMessage ){
      //if the incoming byte is a commandbyte (note on or off)
      //wait for the other bytes to come in.
      incomingmidicommandbyte = incomingbyte;
      while(Serial.available() <=1){
        //do nothing (wait for the others to come in)
       }
       
       incomingmidinote= Serial.read();
       incomingmidivelocity = Serial.read();
       byte channel = incomingmidicommandbyte & B00001111;
       
       if(midicommand == noteoffMessage || incomingmidivelocity == 0){
         noteOffFunction(incomingmidinote,incomingmidivelocity, channel);
         //digitalWrite(13,LOW);
       }else if(midicommand == noteonMessage){
          noteOnFunction(incomingmidinote,incomingmidivelocity, channel);
          //digitalWrite(13,HIGH); 
       }
    }
  }
}

void MidiLib::setnoteOnHandle(CallbackFunction thecallbackfunction){
  noteOnFunction = thecallbackfunction;
}

void MidiLib::setnoteOffHandle(CallbackFunction thecallbackfunction){
  noteOffFunction = thecallbackfunction;
}
