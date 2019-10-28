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

bool isOn = false;

byte DutyCycle0 = 64;
byte DutyCycle1 = 64;
byte DutyCycle2 = 64;
//------------------------------------------------------------------------------

dword RawData = 0;
//------------------------------------------------------------------------------

// Interrupt-on-change, port A
interrupt void OnInterrupt(){
  T1CONbits.TMR1ON  = 0;
    word Time = TMR1; // Time is in μs
    if(PIR1bits.TMR1IF) Time = 0xFFFF;
    TMR1            = 0;
    PIR1bits.TMR1IF = 0;
  T1CONbits.TMR1ON  = 1;
  
  byte Bit = Rx; // Sample to clear the interrupt on change

  if(Time > 10000){
    RawData = 0;

  }else if(Time > 5000){
    // Decode -- don't pull into a function: not enough RAM
    word Decoded = 0;

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

  }else if(Time > 3000){
    RawData <<= 1; RawData |= Bit;
    RawData <<= 1; RawData |= Bit;

  }else if(Time > 1000){
    RawData <<= 1; RawData |= Bit;

  }else{
    RawData = 0;
  }

  INTCONbits.RAIF = 0;
}
//------------------------------------------------------------------------------

void AssignLEDs(){
  if(isOn){
    PORTC = (((DutyCycle2 > TMR0) & 1) << 2) |
            (((DutyCycle1 > TMR0) & 1) << 1) |
            (((DutyCycle0 > TMR0) & 1)     ) ;
  }else{
    PORTC = 0;
  }
}
//------------------------------------------------------------------------------

void HandleButtons(byte Buttons){
  static byte PrevButtons = 0;

  if(Buttons != PrevButtons){
    byte ButtonDown = (PrevButtons ^ Buttons) & ( Buttons);
    // byte ButtonUp   = (PrevButtons ^ Buttons) & (~Buttons);

    PrevButtons = Buttons;

    if(ButtonDown & 0x01){ // Cool white brighter
      if(!isOn) isOn = true;
      if     (DutyCycle0 <  0x01) DutyCycle0 = 0x01;
      else if(DutyCycle0 >= 0x80) DutyCycle0 = 0xFF;
      else                        DutyCycle0 <<= 1;
    }
    if(ButtonDown & 0x02){ // Natural white brighter
      if(!isOn) isOn = true;
      if     (DutyCycle1 <  0x01) DutyCycle1 = 0x01;
      else if(DutyCycle1 >= 0x80) DutyCycle1 = 0xFF;
      else                        DutyCycle1 <<= 1;
    }
    if(ButtonDown & 0x04){ // Warm white brighter
      if(!isOn) isOn = true;
      if     (DutyCycle2 <  0x01) DutyCycle2 = 0x01;
      else if(DutyCycle2 >= 0x80) DutyCycle2 = 0xFF;
      else                        DutyCycle2 <<= 1;
    }

    if(ButtonDown & 0x40){ // Cool white dimmer
      if(DutyCycle0 > 0x80) DutyCycle0 = 0x80;
      else                  DutyCycle0 >>= 1;
    }
    if(ButtonDown & 0x20){ // Natural white dimmer
      if(DutyCycle1 > 0x80) DutyCycle1 = 0x80;
      else                  DutyCycle1 >>= 1;
    }
    if(ButtonDown & 0x10){ // Warm white dimmer
      if(DutyCycle2 > 0x80) DutyCycle2 = 0x80;
      else                  DutyCycle2 >>= 1;
    }

    if(ButtonDown & 0x08){ // On / Off
      isOn = !isOn;
    }
    if(ButtonDown & 0x80){ // Mode
    }
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
    AssignLEDs();

    if(PIR1bits.TMR1IF) Buttons = 0;
    HandleButtons(Buttons);
  }
}
//------------------------------------------------------------------------------
