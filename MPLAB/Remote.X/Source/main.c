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

byte Parity(word Data){
  byte Result;

  Result = (Data   & 0xFF) ^ (Data   >> 8);
  Result = (Result & 0x0F) ^ (Result >> 4);
  Result = (Result & 0x03) ^ (Result >> 2);
  Result = (Result & 0x01) ^ (Result >> 1);
  
  // Has to be odd parity to ensure that the final edge is falling
  return (~Result) & 1;
}
//------------------------------------------------------------------------------

// Interrupt on timer 0
interrupt void OnInterrupt(){
  typedef enum{
    Idle,
    SendEdge,
    SendData
  } STATE;
  
  static STATE State = Idle;
  static byte  Count = 0;
  static word  Data;

  switch(State){
    case Idle:
      Count++;
      if(Count == 3){
        Data = (PORTAbits.RA3 << 2) | (PORTAbits.RA0 << 1) | PORTAbits.RA1; // Address
        Data = (Data << 8) | ((PORTA & 0x30) << 2) | PORTC; // Buttons
        Data = (Data << 1) | Parity(Data); // Parity

        Count = 0;
        State = SendEdge;
      }
      break;
    //--------------------------------------------------------------------------

    case SendEdge:
      Tx = !Tx;

      if(Count == 16){
        Count = 0;
        State = Idle;

      }else{
        State = SendData;
      }
      break;
    //--------------------------------------------------------------------------

    case SendData:
      if(Data & 0x8000) Tx = !Tx;
      Data = Data << 1;

      Count++;
      State = SendEdge;

      break;
    //--------------------------------------------------------------------------

    default: break;
  }

  INTCONbits.T0IF = 0;
}
//------------------------------------------------------------------------------

void main(){
  OPTION_REGbits.nRAPU = 1; // Disable weak pull-ups
  OPTION_REGbits.T0CS  = 0; // Timer 0 uses internal clock
  OPTION_REGbits.PSA   = 0; // Prescaler assigned to Timer 0
  OPTION_REGbits.PS    = 1; // Timer 0 rate => 4 μs clock / 1.024 ms interrupt

  CMCONbits .CM   = 7; // Switch off the comparators
  ADCON0bits.ADON = 0; // Switch off the ADC

  ANSEL = 0; // Disable analogue functionality
  WPUA  = 0; // Disable weak pull-up on port A

  PORTA = 0;
  PORTC = 0;
  TRISA = 0xFB; // Only RA2 is an output
  TRISC = 0xFF;

  INTCONbits.T0IE = 1;
  INTCONbits.GIE  = 1;

  while(1);
}
//------------------------------------------------------------------------------
