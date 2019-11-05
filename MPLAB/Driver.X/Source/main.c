//==============================================================================
// Copyright (C) John-Philip Taylor
// jpt13653903@gmail.com
//
// This file is part of LED Light
//
// This file is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>
//==============================================================================

#include "config.h"
#include "global.h"
//------------------------------------------------------------------------------

byte Address = 0;
byte Buttons = 0;

struct CONTROL{
  unsigned isOn     : 1;
  unsigned FadeIn0  : 1;
  unsigned FadeIn1  : 1;
  unsigned FadeIn2  : 1;
  unsigned FadeOut0 : 1;
  unsigned FadeOut1 : 1;
  unsigned FadeOut2 : 1;
} Control = {0};

byte DutyCycle0 = 64;
byte DutyCycle1 = 64;
byte DutyCycle2 = 64;
//------------------------------------------------------------------------------

dword RawData = 0;
//------------------------------------------------------------------------------

// Interrupt-on-change, port A
interrupt void OnInterrupt(){
  byte Bit     = Rx; // Sample to clear the interrupt on change
  word Decoded = 0;

  T1CONbits.TMR1ON  = 0;
    word Time = TMR1; // Time is in μs
    if(PIR1bits.TMR1IF) Time = 0xFFFF;
    TMR1            = 0;
    PIR1bits.TMR1IF = 0;
  T1CONbits.TMR1ON  = 1;
  
  if(Time > 5000){
    RawData = 0;

  }else if(Time > 2500){
    // Decode -- don't pull into a function: not enough RAM
    RawData = (RawData << 1) ^ RawData;
    if((RawData & 0x55555554) == 0x55555554){
      RawData >>= 1;
      for(byte n = 0; n < 16; n++){
        Decoded >>= 1;
        if(RawData & 1) Decoded |= 0x8000;
        RawData >>= 2;
      }
      byte Parity = 0;
      Parity = (Decoded & 0xFF) ^ (Decoded >> 8);
      Parity = (Parity  & 0x0F) ^ (Parity  >> 4);
      Parity = (Parity  & 0x03) ^ (Parity  >> 2);
      Parity = (Parity  & 0x01) ^ (Parity  >> 1);
      if(!Parity) Decoded = 0;
    }
    if(Decoded){
      Address = Decoded >> 9;
      Buttons = Decoded >> 1;
    }
    RawData = 0;

  }else if(Time > 1500){
    RawData <<= 1; RawData |= Bit;
    RawData <<= 1; RawData |= Bit;

  }else if(Time > 500){
    RawData <<= 1; RawData |= Bit;

  }else{
    RawData = 0;
  }

  INTCONbits.RAIF = 0;
}
//------------------------------------------------------------------------------

void HandleButtons(byte Buttons){
  static byte PrevButtons = 0;

  if(Buttons != PrevButtons){
    byte ButtonDown = (PrevButtons ^ Buttons) & ( Buttons);
    byte ButtonUp   = (PrevButtons ^ Buttons) & (~Buttons);

    PrevButtons = Buttons;

    if(ButtonDown & 0x01){ // Cool white brighter
      if(!Control.isOn) Control.isOn = true;
      Control.FadeIn0 = true;
    }
    if(ButtonDown & 0x02){ // Cool white brighter
      if(!Control.isOn) Control.isOn = true;
      Control.FadeIn1 = true;
    }
    if(ButtonDown & 0x04){ // Cool white brighter
      if(!Control.isOn) Control.isOn = true;
      Control.FadeIn2 = true;
    }
    if(ButtonDown & 0x40){ // Cool white brighter
      Control.FadeOut0 = true;
    }
    if(ButtonDown & 0x20){ // Cool white brighter
      Control.FadeOut1 = true;
    }
    if(ButtonDown & 0x10){ // Cool white brighter
      Control.FadeOut2 = true;
    }

    if(ButtonUp & 0x01){ // Cool white brighter
      Control.FadeIn0 = false;
    }
    if(ButtonUp & 0x02){ // Cool white brighter
      Control.FadeIn1 = false;
    }
    if(ButtonUp & 0x04){ // Cool white brighter
      Control.FadeIn2 = false;
    }
    if(ButtonUp & 0x40){ // Cool white brighter
      Control.FadeOut0 = false;
    }
    if(ButtonUp & 0x20){ // Cool white brighter
      Control.FadeOut1 = false;
    }
    if(ButtonUp & 0x10){ // Cool white brighter
      Control.FadeOut2 = false;
    }

    if(ButtonDown & 0x08){ // On / Off
      Control.isOn = !Control.isOn;
    }
    if(ButtonDown & 0x80){ // Mode
    }
  }
}
//------------------------------------------------------------------------------

void DoFading(){
  static byte PrevTime  = 0;
  static byte TimeCount = 0;

  byte Time = TMR0;

  if(Time < PrevTime){
    if(TimeCount = 3){
      TimeCount = 0;

      if(Control.FadeIn0  && DutyCycle0 < 0xFF) DutyCycle0++;
      if(Control.FadeIn1  && DutyCycle1 < 0xFF) DutyCycle1++;
      if(Control.FadeIn2  && DutyCycle2 < 0xFF) DutyCycle2++;

      if(Control.FadeOut0 && DutyCycle0 > 0x00) DutyCycle0--;
      if(Control.FadeOut1 && DutyCycle1 > 0x00) DutyCycle1--;
      if(Control.FadeOut2 && DutyCycle2 > 0x00) DutyCycle2--;
    }else{
      TimeCount++;
    }
  }
  PrevTime = Time;
}
//------------------------------------------------------------------------------

void AssignLEDs(){
  if(Control.isOn){
    byte Temp = 0;
    if(DutyCycle0 > TMR0) Temp |= 0x09;
    if(DutyCycle1 > TMR0) Temp |= 0x12;
    if(DutyCycle2 > TMR0) Temp |= 0x24;
    PORTC = Temp;
  }else{
    PORTC = 0;
  }
}
//------------------------------------------------------------------------------

void main(){
  OPTION_REGbits.nRAPU  = 1; // Disable weak pull-ups
  OPTION_REGbits.T0CS   = 0; // Timer 0 uses internal clock
  OPTION_REGbits.PSA    = 0; // Prescaler assigned to Timer 0
  OPTION_REGbits.PS     = 3; // Timer 0 rate => 16 μs clock / 4.096 ms interrupt

  T1CONbits.TMR1CS  = 0;
  T1CONbits.T1CKPS  = 0; // 1 μs clock / 65.536 ms overflow
  T1CONbits.nT1SYNC = 1;
  T1CONbits.TMR1GE  = 0;
  PIR1bits.TMR1IF   = 1;
  T1CONbits.TMR1ON  = 1;

  CMCONbits .CM   = 7; // Switch off the comparators
  ADCON0bits.ADON = 0; // Switch off the ADC

  ANSEL = 0; // Disable analogue functionality
  WPUA  = 0; // Disable weak pull-up on port A

  IOCA            = 0x20; // Interrupt-on-change on A5
  INTCONbits.RAIE = 1;    // Enable port A interrupt-on-change
  INTCONbits.GIE  = 1;

  PORTA = 0x00;
  PORTC = 0x00;

  TRISA = 0xEF;
  TRISC = 0xC0;

  while(1){
    if(PIR1bits.TMR1IF) Buttons = 0;
    HandleButtons(Buttons);
    DoFading();
    AssignLEDs();
  }
}
//------------------------------------------------------------------------------
