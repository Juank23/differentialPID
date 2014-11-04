/*
  Receives commands on Serial and either controls a Polulu Qik Motor controller
  on Serial1 or reads quadrature encoded sensors on pins 11, 12, 13, 14 and
  responds.
  
  Works with Stellaris Launchpad. Open with Energia.
  
  Tim Coyne, 2014
 
*/

#include "Track.h"

// Some serial command values
#define RQ_ERR 0x82      // Request error from qik
#define SPOOF_COM 0xBF   // Bogus command for qik
#define FORMAT_ERR 0x40  // Error status from qik if incorrect command given
#define CONNECT 0xAA     // qik connect request sequence
#define QIK_ERR 0x04
#define ERR_PIN 10

// Some defined status colours
#define ERROR 63, 0, 0
#define POST  15, 0, 31
#define COMMS 0, 15, 31
#define A_OK  3, 31, 0

byte err = 0;
volatile byte portABuf;
byte portA;
int countl = 0;
int countr = 0;

Track leftTrack;
Track rightTrack;

void setup() {
  // Set status led pins
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  
  // Set status light to booting colour
  setRGB(POST);
  
  // Set quadrature encoder sensor pins to input
  pinMode(8, INPUT);
  pinMode(9, INPUT);
  pinMode(10, INPUT);
  pinMode(13, INPUT);
  
  // Set up track state
  initSensors();          // attaches interupts, sets initial state (portABuf)
  portA = portABuf;       // grab initial state
  leftTrack.init(portA >> 2);
  rightTrack.init(portA);

  Serial.begin(9600); // Comms to host
  Serial1.begin(38400); // Comms to Qik
  initQik();
  
  // All done, change status light
  setRGB(A_OK);
}

void loop() {
  if (err != 0) {
    setRGB(ERROR);
    digitalWrite(ERR_PIN, HIGH);
  }
  if (Serial.available()) {
    int inByte = Serial.read();
    parseCom(inByte);
  }
  
  // read from port 1, send to port 0:
  if (Serial1.available()) {
    int inByte = Serial1.read();
    Serial.println(inByte);
  }
  
  // check sensors
  if (portA != portABuf) {
    portA = portABuf;
    leftTrack.update(portA >> 2);
    rightTrack.update(portA);
  }
}

/**
 * Rudimentary command interface for testing
 **/
void parseCom(int data) {
  static int buf;
  switch (data) {
    case 'E':
      Serial1.write(RQ_ERR);
      break;
    case 'I':
      Serial.println("initialising qik");
      initQik();
      break;
    case 'C':
      setRGB(A_OK);
      Serial1.write(0x86);
      Serial1.write(0x87);
      break;
    case 'M':
      Serial1.write(0x88);
      Serial1.write(buf);
      break;
    case 'N':
      Serial1.write(0x8C);
      Serial1.write(buf);
      break;
    case 'S':
    case 's':
      Serial.print("Left: ");
      Serial.print(leftTrack.getCount());
      Serial.print("\tRight: ");
      Serial.print(rightTrack.getCount());
      Serial.print("\tTime L: ");
      Serial.print(leftTrack.getAverageTimePerStep());
      Serial.print("\tR: ");
      Serial.println(rightTrack.getAverageTimePerStep());
      break;
    case 'T':
      setRGB(COMMS);
      Serial.println("Test");
      Serial.write(qikCommsTest());
      break;
    default:
      buf = data;
  }
}

/**
 * Reads state of quadrature encoders. (Used as interupt)
 **/
void readQEs() {
  portABuf = GPIOPinRead(GPIO_PORTA_BASE, 0xF0);
  portABuf = portABuf >> 4;
}

/**
 * Initiates interupts for quadrature encoders.
 **/
void initSensors() {
  readQEs();
  attachInterrupt(8, readQEs, CHANGE);
  attachInterrupt(9, readQEs, CHANGE);
  attachInterrupt(10, readQEs, CHANGE);
  attachInterrupt(13, readQEs, CHANGE);
}

/**
 * Set RGB led colour to r, g, b.
 **/
void setRGB(int r, int g, int b) {
  analogWrite(RED_LED, r);
  analogWrite(GREEN_LED, g);
  analogWrite(BLUE_LED, b);
}

/**
 * Initiates connection with the qik motor controller.
 * Performs three atempts, then returns.
 * Sets error bit upon failure.
 **/
void initQik() {
  for(int i = 3; i > 0; --i) {
    Serial1.write(CONNECT);
    delay(1);
    if (qikCommsTest()) {
      break;
    }
  }
  if (!qikCommsTest()) {
    err &= QIK_ERR;
  }
}

/**
 * Tests connection to qik motor controller.
 * Sends a spurious command to qik, then requests error state.
 * Error bit 6 (0x40) should be set (Format Error)
 **/
boolean qikCommsTest() {
  boolean pass = false;
  Serial1.write(SPOOF_COM);
  for(int i = 3; i > 0; --i) {
    delay(1); // Give time for qik to respond
    Serial1.write(RQ_ERR);
    delay(1);
    if (Serial1.available()) {
      byte err = Serial1.read();
      if (err & FORMAT_ERR == FORMAT_ERR) {
        pass = true;
        break;
      }
    }
  }
  return pass;
}
      
