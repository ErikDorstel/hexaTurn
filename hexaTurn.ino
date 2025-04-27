const bool logging=true;

#include <Wire.h>
#include <MCP23017.h>
#include "eth.h"

MCP23017 myMCP0=MCP23017(&Wire,0x20);
MCP23017 myMCP1=MCP23017(&Wire,0x21);
MCP23017 myMCP2=MCP23017(&Wire,0x22);
MCP23017 myMCP3=MCP23017(&Wire,0x23);
MCP23017 myMCP4=MCP23017(&Wire1,0x20);
MCP23017 myMCP5=MCP23017(&Wire1,0x21);
MCP23017 myMCP6=MCP23017(&Wire1,0x22);
MCP23017 myMCP7=MCP23017(&Wire1,0x23);

volatile bool mcpChange=0;
volatile uint8_t mcp=0;
volatile uint8_t mcpA=0;
volatile uint8_t mcpB=0;

struct encStruct { uint8_t seqIndex[32]; uint8_t nextCW[32]; uint8_t nextCCW[32]; int32_t value[32]; bool button[32]; } enc;
const uint8_t encNumber[32]={3,2,1,0,9,8,16,17,27,26,24,25,13,10,11,12,7,6,5,4,15,14,22,23,29,28,30,31,20,19,18,21};
const uint8_t encSequence[6]={1,0,2,3,1,0};

void setup() {
  if (logging) { Serial.begin(115200); }

  initEth();

  pinMode(2,INPUT_PULLUP); attachInterrupt(digitalPinToInterrupt(2),isrMCPa,CHANGE);
  pinMode(3,INPUT_PULLUP); attachInterrupt(digitalPinToInterrupt(3),isrMCPb,CHANGE);
  pinMode(4,INPUT_PULLUP); attachInterrupt(digitalPinToInterrupt(4),isrMCPc,CHANGE);
  pinMode(5,INPUT_PULLUP); attachInterrupt(digitalPinToInterrupt(5),isrMCPd,CHANGE);
  pinMode(6,INPUT_PULLUP); attachInterrupt(digitalPinToInterrupt(6),isrMCPe,CHANGE);
  pinMode(7,INPUT_PULLUP); attachInterrupt(digitalPinToInterrupt(7),isrMCPf,CHANGE);
  pinMode(8,INPUT_PULLUP); attachInterrupt(digitalPinToInterrupt(8),isrMCPg,CHANGE);
  pinMode(9,INPUT_PULLUP); attachInterrupt(digitalPinToInterrupt(9),isrMCPh,CHANGE);
  
  Wire.begin(); Wire.setClock(1700000UL);
  Wire1.begin(); Wire1.setClock(1700000UL);
  myMCP0.Init(); myMCP0.setPortMode(0b00000000,A); myMCP0.setPortPullUp(0b11111111,A); myMCP0.setInterruptOnChangePort(0x3F,A);
  myMCP0.setPortMode(0b00000000,B); myMCP0.setPortPullUp(0b11111111,B); myMCP0.setInterruptOnChangePort(0x3F,B); myMCP0.setIntMirror(ON);
  myMCP1.Init(); myMCP1.setPortMode(0b00000000,A); myMCP1.setPortPullUp(0b11111111,A); myMCP1.setInterruptOnChangePort(0x3F,A);
  myMCP1.setPortMode(0b00000000,B); myMCP1.setPortPullUp(0b11111111,B); myMCP1.setInterruptOnChangePort(0x3F,B); myMCP1.setIntMirror(ON);
  myMCP2.Init(); myMCP2.setPortMode(0b00000000,A); myMCP2.setPortPullUp(0b11111111,A); myMCP2.setInterruptOnChangePort(0x3F,A);
  myMCP2.setPortMode(0b00000000,B); myMCP2.setPortPullUp(0b11111111,B); myMCP2.setInterruptOnChangePort(0x3F,B); myMCP2.setIntMirror(ON);
  myMCP3.Init(); myMCP3.setPortMode(0b00000000,A); myMCP3.setPortPullUp(0b11111111,A); myMCP3.setInterruptOnChangePort(0x3F,A);
  myMCP3.setPortMode(0b00000000,B); myMCP3.setPortPullUp(0b11111111,B); myMCP3.setInterruptOnChangePort(0x3F,B); myMCP3.setIntMirror(ON);
  myMCP4.Init(); myMCP4.setPortMode(0b00000000,A); myMCP4.setPortPullUp(0b11111111,A); myMCP4.setInterruptOnChangePort(0x3F,A);
  myMCP4.setPortMode(0b00000000,B); myMCP4.setPortPullUp(0b11111111,B); myMCP4.setInterruptOnChangePort(0x3F,B); myMCP4.setIntMirror(ON);
  myMCP5.Init(); myMCP5.setPortMode(0b00000000,A); myMCP5.setPortPullUp(0b11111111,A); myMCP5.setInterruptOnChangePort(0x3F,A);
  myMCP5.setPortMode(0b00000000,B); myMCP5.setPortPullUp(0b11111111,B); myMCP5.setInterruptOnChangePort(0x3F,B); myMCP5.setIntMirror(ON);
  myMCP6.Init(); myMCP6.setPortMode(0b00000000,A); myMCP6.setPortPullUp(0b11111111,A); myMCP6.setInterruptOnChangePort(0x3F,A);
  myMCP6.setPortMode(0b00000000,B); myMCP6.setPortPullUp(0b11111111,B); myMCP6.setInterruptOnChangePort(0x3F,B); myMCP6.setIntMirror(ON);
  myMCP7.Init(); myMCP7.setPortMode(0b00000000,A); myMCP7.setPortPullUp(0b11111111,A); myMCP7.setInterruptOnChangePort(0x3F,A);
  myMCP7.setPortMode(0b00000000,B); myMCP7.setPortPullUp(0b11111111,B); myMCP7.setInterruptOnChangePort(0x3F,B); myMCP7.setIntMirror(ON);

  for (uint8_t encIndex=0;encIndex<32;encIndex++) {
    enc.seqIndex[encIndex]=encSequence[3]; enc.nextCW[encIndex]=encSequence[4]; enc.nextCCW[encIndex]=encSequence[2];
    enc.value[encIndex]=0; enc.button[encIndex]=0; } }

void loop() {
  ethWorker();

  if (mcpChange) { mcpChange=false;

    for (uint8_t encPart=0;encPart<4;encPart++) {
      uint8_t encIndex=(mcp*4)+encPart;
      uint8_t encValue; bool buttonValue;
      if (encPart==0) { encValue=(mcpA>>1)&3; buttonValue=!((mcpA>>0)&1); } else
      if (encPart==1) { encValue=(mcpA>>4)&3; buttonValue=!((mcpA>>3)&1); } else
      if (encPart==2) { encValue=(mcpB>>1)&3; buttonValue=!((mcpB>>0)&1); } else
      if (encPart==3) { encValue=(mcpB>>4)&3; buttonValue=!((mcpB>>3)&1); }

      if (encValue==enc.nextCW[encIndex]) {
        if (enc.seqIndex[encIndex]<4) { enc.seqIndex[encIndex]+=1; } else { enc.seqIndex[encIndex]=1; }
        enc.nextCW[encIndex]=encSequence[enc.seqIndex[encIndex]+1];
        enc.nextCCW[encIndex]=encSequence[enc.seqIndex[encIndex]-1];
        enc.value[encIndex]+=1;
        if (enc.value[encIndex]%4==0) { ethSend(encNumber[encIndex],2);
          if (logging) { Serial.print(encNumber[encIndex]); Serial.print(" +1 "); Serial.println(enc.value[encIndex]/4); } } } else

      if (encValue==enc.nextCCW[encIndex]) {
        if (enc.seqIndex[encIndex]>1) { enc.seqIndex[encIndex]-=1; } else { enc.seqIndex[encIndex]=4; }
        enc.nextCW[encIndex]=encSequence[enc.seqIndex[encIndex]+1];
        enc.nextCCW[encIndex]=encSequence[enc.seqIndex[encIndex]-1];
        enc.value[encIndex]-=1;
        if (enc.value[encIndex]%4==0) { ethSend(encNumber[encIndex],3);
          if (logging) { Serial.print(encNumber[encIndex]); Serial.print(" -1 "); Serial.println(enc.value[encIndex]/4); } } }

      if (buttonValue!=enc.button[encIndex]) { enc.button[encIndex]=buttonValue;
        ethSend(encNumber[encIndex],enc.button[encIndex]);
        if (logging) { Serial.print(encNumber[encIndex]); Serial.print(" Button "); Serial.println(enc.button[encIndex]); } } } } }

void isrMCPa() { mcp=4; mcpA=myMCP4.getIntCap(A); mcpB=myMCP4.getIntCap(B); mcpChange=true; }
void isrMCPb() { mcp=5; mcpA=myMCP5.getIntCap(A); mcpB=myMCP5.getIntCap(B); mcpChange=true; }
void isrMCPc() { mcp=6; mcpA=myMCP6.getIntCap(A); mcpB=myMCP6.getIntCap(B); mcpChange=true; }
void isrMCPd() { mcp=7; mcpA=myMCP7.getIntCap(A); mcpB=myMCP7.getIntCap(B); mcpChange=true; }
void isrMCPe() { mcp=2; mcpA=myMCP2.getIntCap(A); mcpB=myMCP2.getIntCap(B); mcpChange=true; }
void isrMCPf() { mcp=1; mcpA=myMCP1.getIntCap(A); mcpB=myMCP1.getIntCap(B); mcpChange=true; }
void isrMCPg() { mcp=0; mcpA=myMCP0.getIntCap(A); mcpB=myMCP0.getIntCap(B); mcpChange=true; }
void isrMCPh() { mcp=3; mcpA=myMCP3.getIntCap(A); mcpB=myMCP3.getIntCap(B); mcpChange=true; }
