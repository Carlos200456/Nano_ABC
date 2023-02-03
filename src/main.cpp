/* Nano Universal Interface by Meditech s.a. 

   Sketch > Include Library > Manage Libraries...
   Include the following Libraries:
   U8g2 Library

 This code is not in the public domain.

 http://www.meditech.com.ar

 */
#include <Arduino.h>
#include <EEPROM.h>
#include <Wire.h>
#include "..\lib\TimerTwo.h"
#include "..\lib\TimerTwo.cpp"
#include "Functions.h"

// Opcionales de la Interface

#define PulseIn 2
#define PulseOut 3
#define KVPlus 4
#define KVMinus 5
#define AEC_Analog A0


String inputString = "";               // a string to hold incoming data
String Tipo ="";
String Signo ="";
String Magnitud ="";
String ACK ="ACK";
String NACK ="NACK";
bool DataReady = false;             // whether the string is complete
bool error = false;                 // whether the string have errors

unsigned char AEC_Limit_UP = 0;
unsigned char AEC_Limit_DW = 0;
unsigned char AEC_Limit_UP_Cine = 0;
unsigned char AEC_Limit_DW_Cine = 0;
unsigned char AEC_Analod_Read = 0;

unsigned int count = 0;
unsigned int XRayPeriod = 0;
unsigned int XRayTime = 0;
bool debugbool = false;


int eeAddress; //EEPROM address to start reading from

void setup() {
  // initialize serial:
  Serial.begin(57600);
  // reserve 20 bytes for the inputString:
  inputString.reserve(20);

  pinMode(PulseIn, INPUT_PULLUP);
  pinMode(AEC_Analog, INPUT);
  pinMode(KVPlus, OUTPUT);
  pinMode(KVMinus, OUTPUT);
  AEC_Limit_UP = ReadEEPROM(0);
  AEC_Limit_DW = ReadEEPROM(1);
  AEC_Limit_UP_Cine = ReadEEPROM(2);
  AEC_Limit_DW_Cine = ReadEEPROM(3);
  Timer2.EnableTimerInterrupt(Xray, 1000);                            // Interrupt every 1 milliseconds cuando hay que controlar los pulsos
}


// ------------------ Interupt for Pulse Generation -----------------------------------------
void Xray(void){
  count++;
  if (count == 5) AEC_Analod_Read = analogRead(AEC_Analog);     // Lee el Valor de AEC durante el Pulso
}


void loop() {
  if (debugbool) {
    // digitalWrite (T1, LOW);                   // Anula la restriccion de ABC en DEBUG para poder calibrar
    AEC_Analod_Read = analogRead(AEC_Analog); // Lee el valor de ABC en DEBUG para poder calibrar
  } 
  // Read the string when a newline arrives:
  if (DataReady) {
    Tipo = inputString.substring(0,1);
    Signo = inputString.substring(1,2);
    Magnitud = inputString.substring(2);

    if (Tipo == "D"){                     // Activar el Modo Debug
      if (Signo == "B"){
        if (Magnitud.toInt() == 0){
          debugbool = false;
        } else {
          debugbool = true;
        }
      }
      goto jmp;
    }

    if ((Tipo == "B")&&(debugbool)){    // Command for AEC Calibration
      if (Signo == "U"){
        AEC_Limit_UP += Magnitud.toInt();
      }
      if (Signo == "D"){
        AEC_Limit_DW -= Magnitud.toInt();
      }
      if (AEC_Limit_UP > 255) AEC_Limit_UP = 255;
      if (AEC_Limit_DW < 0) AEC_Limit_DW = 0;
      goto jmp;
    }

    if ((Tipo == "T")&&(debugbool)){         // -------------------- Interface Reset Program --------------------
      WriteEEPROM(0, 196);
      delay(50);
      WriteEEPROM(1, 64);
      delay(50);
      WriteEEPROM(2, 196);
      delay(50);
      WriteEEPROM(3, 64);
      delay(50);
      AEC_Limit_UP = ReadEEPROM(0);
      AEC_Limit_DW = ReadEEPROM(1);
      AEC_Limit_UP_Cine = ReadEEPROM(2);
      AEC_Limit_DW_Cine = ReadEEPROM(3);
      delay(500);
      software_Reset();
      goto jmp;
    }

    error = true;      // Si el comando de entrada es desconocido setear error
    
    jmp:
    
    if (debugbool){
      Serial.print("RX: ");
      Serial.print(inputString);
      Serial.print("Tipo: ");
      Serial.print(Tipo);
      Serial.print(" ,Signo: ");
      Serial.print(Signo);
      Serial.print(" ,Magnitud: ");
      Serial.println(Magnitud);
      Serial.print("AEC Limit UP: ");
      Serial.println(AEC_Limit_UP, DEC);
      Serial.print("AEC Limit DW: ");
      Serial.println(AEC_Limit_DW, DEC);
      Serial.print("AEC Limit UP Cine: ");
      Serial.println(AEC_Limit_UP_Cine, DEC);
      Serial.print("AEC Limit DW Cine: ");
      Serial.println(AEC_Limit_DW_Cine, DEC);
    }

    if (error) {
      Serial.println(NACK);
    } else {
      Serial.println(ACK);
    }
    
    error = false;
    // clear the string:
    inputString = "";
    DataReady = false;
  }


  if (debugbool) {
    //
  } else {
    //
  }

  
  // --------- Detector de Pulsos de RX de Cine ------------------
  digitalWrite(PulseOut,!digitalRead(PulseIn)) ;
  
  
}   // -------------- End of main Loop ---------------------


/*
 SerialEvent occurs whenever a new data comes in the
 hardware serial RX.  This routine is run between each
 time loop() runs, so using delay inside loop can delay
 response.  Multiple bytes of data may be available.
 */
void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {
      DataReady = true;
    }
  }
}

int ReadEEPROM(int i){
  int Data;
  eeAddress = sizeof(int) * i; 
  //Get the int data from the EEPROM at position 'eeAddress'
  EEPROM.get(eeAddress, Data);
  return Data;
}

void WriteEEPROM(int i, int Data){
  eeAddress = sizeof(int) * i; 
  EEPROM.put(eeAddress, Data);
}

void software_Reset() // Restarts program from beginning but does not reset the peripherals and registers
{
asm volatile ("  jmp 0");  
}  
