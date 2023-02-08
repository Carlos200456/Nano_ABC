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
#define SCIn 6
#define CineIn 7
#define SCOut 8
#define SCReady 9
#define CineReady 10
#define AEC_Analog A0
#define XRay 13

String inputString = "";               // a string to hold incoming data
String Tipo ="";
String Signo ="";
String Magnitud ="";
String ACK ="ACK ABC";
String NACK ="NACK ABC";
bool DataReady = false;             // whether the string is complete
bool error = false;                 // whether the string have errors

unsigned char AEC_Limit_UP = 0;
unsigned char AEC_Limit_DW = 0;
unsigned char AEC_Analod_Read = 0;
unsigned char PulseUP = 0;
unsigned char PulseDW = 0;
unsigned char AEC_Limit_In = 0;

unsigned int count = 0;
unsigned int XRayPeriod = 80;
unsigned int XRayTime = 10;
bool debugbool = false;
bool Busy = false;


int eeAddress; //EEPROM address to start reading from

void setup() {
  // initialize serial:
  Serial.begin(57600);
  // reserve 20 bytes for the inputString:
  inputString.reserve(20);

  pinMode(PulseIn, INPUT_PULLUP);
  pinMode(SCIn, INPUT_PULLUP);
  pinMode(CineIn, INPUT_PULLUP);
  pinMode(PulseOut, OUTPUT);
  pinMode(SCOut, OUTPUT);
  pinMode(SCReady, OUTPUT);
  pinMode(CineReady, OUTPUT);
  pinMode(AEC_Analog, INPUT);
  pinMode(KVPlus, OUTPUT);
  pinMode(KVMinus, OUTPUT);
  pinMode(XRay, OUTPUT);
  AEC_Limit_UP = ReadEEPROM(0);
  AEC_Limit_DW = ReadEEPROM(1);
  XRayTime = ReadEEPROM(2);
  Timer2.EnableTimerInterrupt(Xray, 1000);                            // Interrupt every 1 milliseconds cuando hay que controlar los pulsos
  Serial.println("ABC Control");
}


// ------------------ Interupt for Pulse Generation -----------------------------------------
void Xray(void){
  if (debugbool) {   // Generador de pulsos de Prueba solo en Debug
    if (count == 0) {
      digitalWrite (XRay, LOW);
    }
    count++;
    if (count > 10 ) {
      digitalWrite (XRay, HIGH);
    }
    if (count >= XRayPeriod) count = 0;
  }
  if(PulseDW) {
    PulseDW -= 1;
    digitalWrite(KVMinus, HIGH);
  } else digitalWrite(KVMinus, LOW);
  if(PulseUP) {
    PulseUP -= 1;
    digitalWrite(KVPlus, HIGH);
  } else digitalWrite(KVPlus, LOW);
}


void loop() {
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

    if (Tipo == "I"){                   // Up / Down On Time in ms., valid values 1 - 999
      if (Signo == "U"){
        if ((1 <= Magnitud.toInt())&&( Magnitud.toInt() <= 999)){
          XRayTime = Magnitud.toInt();
          WriteEEPROM(2, XRayTime);
          delay(50);
          XRayTime = ReadEEPROM(2);
        } else error = true;
      }
      goto jmp;
    }

    if ((Tipo == "B")&&(debugbool)){    // Command for AEC Calibration
      if (Signo == "U"){
        AEC_Limit_In += Magnitud.toInt();
      }
      if (Signo == "D"){
        AEC_Limit_In -= Magnitud.toInt();
      }
      goto jmp;
    }

    if ((Tipo == "Z")&&(debugbool)){                   // Command to Write AEC Calibration
      if (Signo == "U") {
        WriteEEPROM(0, AEC_Limit_In);
        delay(50);
        AEC_Limit_UP = ReadEEPROM(0);
      }
      if (Signo == "D") {
        WriteEEPROM(1, AEC_Limit_In);
        delay(50);
        AEC_Limit_DW = ReadEEPROM(1);
      }
      goto jmp;
    }

    if ((Tipo == "T")&&(debugbool)){         // -------------------- Interface Reset Program --------------------
      WriteEEPROM(0, 196);
      delay(50);
      WriteEEPROM(1, 64);
      delay(50);
      WriteEEPROM(2, 40);
      delay(50);
      AEC_Limit_UP = ReadEEPROM(0);
      AEC_Limit_DW = ReadEEPROM(1);
      XRayTime = ReadEEPROM(2);
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
      Serial.print("IF Tipo: ");
      Serial.println(0, DEC);
      Serial.print("AEC Limit: ");
      Serial.println(AEC_Limit_In, DEC);
      Serial.print("AEC Limit UP: ");
      Serial.println(AEC_Limit_UP, DEC);
      Serial.print("AEC Limit DW: ");
      Serial.println(AEC_Limit_DW, DEC);
      Serial.print("AEC Limit UP Cine: ");
      Serial.println(0, DEC);
      Serial.print("AEC Limit DW Cine: ");
      Serial.println(0, DEC);
      Serial.print("AEC Lock Volt: ");
      Serial.println(0, DEC);
      Serial.print("T1 Limit Low(Subir): ");
      Serial.println(0, DEC);
      Serial.print("T0 Limit Hi(Bajar): ");
      Serial.println(0, DEC);
      Serial.print("Servo Iris: ");
      Serial.println(0, DEC);
      Serial.print("Servo Limit Up: ");
      Serial.println(0, DEC);
      Serial.print("Servo Limit Dn: ");
      Serial.println(0, DEC);
      Serial.print("Int Time: ");
      Serial.println(XRayTime, DEC);
      Serial.print("Period: ");
      Serial.println(XRayPeriod, DEC);
      Serial.print("Offset: ");
      Serial.println(0, DEC);
      Serial.print("Gain: ");
      Serial.println(0, 2);
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


 if (!digitalRead(SCIn)){
   digitalWrite(SCOut, HIGH);
   digitalWrite(SCReady, HIGH);
  } else {
    digitalWrite(SCReady, LOW);
  }

  if (!digitalRead(CineIn)){
    digitalWrite(SCOut, HIGH);
    digitalWrite(CineReady, HIGH);
  } else {
    digitalWrite(CineReady, LOW);
  }

 if (digitalRead(SCIn) && digitalRead(CineIn)) digitalWrite(SCOut, LOW);


  if (debugbool) {
    //
  } else {
    //
  }

  AEC_Analod_Read = analogRead(AEC_Analog) / 4; // Lee el valor de ABC de entrada
  
  // --------- Detector de Pulsos de RX de Cine ------------------
  digitalWrite(PulseOut,!digitalRead(PulseIn)) ;
  if(digitalRead(PulseIn)){
    Busy = false;
  }

  if (!digitalRead(PulseIn) && !Busy){
    if (AEC_Analod_Read > AEC_Limit_UP){
      PulseDW = XRayTime;
      Busy = true;
    }
    if (AEC_Analod_Read < AEC_Limit_DW){
      PulseUP = XRayTime;
      Busy = true;
    }
  }
  
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
