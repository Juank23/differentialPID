/*
  Receives commands on Serial and either controls a Polulu Qik Motor controller
  on Serial1 or reads quadrature encoded sensors on pins 11, 12, 13, 14 and
  responds.
  
  Works with Stellaris Launchpad. Open with Energia.
  
  Tim Coyne, 2014
 
*/

// Some serial command values
#define RQ_ERR 0x82
#define SPOOF_COM 0xBF
#define FORMAT_ERR 0x40
#define CONNECT 0xAA
#define QIK_ERR 0x04
#define ERR_PIN 10

// Some defined status colours
#define ERROR 63, 0, 0
#define POST  15, 0, 31
#define COMMS 0, 15, 31
#define A_OK  3, 31, 0

byte err = 0;
volatile int portABuf;
int portA;
int countl = 0;
int countr = 0;

enum photoInts {
  ONE = B00000011,
  TWO = B00000010,
  THREE = B00000000,
  FOUR = B00000001
};

void setup() {
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(8, INPUT);
  pinMode(9, INPUT);
  pinMode(10, INPUT);
  pinMode(13, INPUT);
  // Set up track state
  readQEs();
  initSensors();
  portA = portABuf;
  setRGB(POST);
  Serial.begin(9600); // Comms to host
  Serial1.begin(38400); // Comms to Qik
  initQik();
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
    calcMovement();
  }
}

/**
 * Calculate the change in track position, change track counters
 **/
void calcMovement() {
  static int prevPortA = portA;
  
  // Mask lower two bits, right track
  if ((prevPortA & 0x03) != (portA & 0x03)) {
    countr++;
  }
  
  // Mask two bits for left track
  if ((prevPortA & 0x0C) != (portA & 0x0C)) {
    countl++;
  }
  prevPortA = portA;
}

// TODO - create a QE class with enums. Can set of error. Can count

// track l, r = blah
// track update left, right
//  calc current speed


/**
 * Convert a bit pattern (from QE) to an int
 */
int patternToState(int pattern) {
  switch (pattern) {
    case ONE:
      return 1;
      break;
    case TWO:
      return 2;
      break;
    case THREE:
      return 3;
      break;
    case FOUR:
      return 4;
      break;
    default:
      return 0;
  }
}
// Assumes only the lowest two bits are relevant, returns change in position.
// Direction 00 > 01 > 11 > 10 > 00 being positive
static int quadratureChange(int prevState, int state) {
  // Check for looped conditions
  if (prevState == 4 && state == 1) {
    return 1;
  }
  if (prevState == 1 && state == 4) {
    return -1;
  }
  
  // Normal circumstances
  return state - prevState;
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
      Serial.print("Left: ");
      Serial.print(countl);
      Serial.print("\tRight: ");
      Serial.println(countr);
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
      
