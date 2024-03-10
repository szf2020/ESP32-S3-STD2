#include "FastLED.h"     // WS2812B addressable GRB LED
#include "AS5600.h"      // AS5600L absolute encoder
#include "Wire.h"        // AS5600L uses I2C
#include "TMCStepper.h"  // TMC2209 Stepper motor driver

// Define for AS5600L
#define SDA_PIN    2  // I2C pins are map to any GPIO
#define SCL_PIN    1

// Define for OnBoard WS2812B LED
#define LED_DIN        9   // Data In pin
#define LED_PWR        10  // VDD pin
#define LED_NB         1   // Quanity of LEDs
#define LED_BRT        20  // LED Brightness
#define LED_TYPE       WS2812B
#define COLOR_ORDER    GRB
#define DarkBlue       0x01244a  // Looks turquoise with low brigthness
#define Blank          CRGB(0, 0, 0)

// Define for OnBoard TMC2209 Driver
#define TMC_EN        7    // TMC enable pin   
#define TMC_DIR       16   // TMC dir pin
#define TMC_STEP      15   // TMC step pin  
#define U1RXD         5    // Serial1 RX pin
#define U1TXD         4    // Serial1 TX pin
#define DRIVER_ADDRESS      0b00   // TMC2209 Driver address according to MS1 and MS2
#define R_SENSE             0.11f  // 0.110 Ohms
#define TMC_maxSpeed        778
#define TMC_minSpeed        156
#define moveBufferLength    3
#define moveDelay           200    // milliseconds. Time between move.
#define SERIAL_PORT         Serial1
#define SERIAL_BAUDRATE     115200

CRGB LED[LED_NB];
AS5600L _AS5600L;
TMC2209Stepper TMCdriver(&SERIAL_PORT, R_SENSE, DRIVER_ADDRESS);

// Using with millis() to avoid delay() stoping the program
static uint32_t timeLED = 0;
static uint32_t timeAS5600L = 0;
static uint32_t timeTMCdriver = 0;
static uint32_t timeMove = 0;

bool     statusLED = false;  // Keeps track of LED's on/off state
bool     shaftDir = true;
uint32_t actualSpeed;
int32_t  position;
int32_t  lastPosition;
int32_t  revolutions;
float    angle;
float    displacement;
uint8_t  stepperMoveSelect;  // Use to select the angle in stepperMoveBuffer
uint8_t  stepperMoveStatus;  // Varies between 0 and 2
                             // 0: Delay finish, setting next destination angle.
                             // 1: Destination angle reach, starting delay.
                             // 2: In delay, waiting for it to end.

// Angle the stepper moves to in a loop
float stepperMoveBuffer[moveBufferLength] = {360.00, 180.00, 45.00};

void printTime(void);
void stepperMoveToAngle(float finalAngle);

void setup() {
  Serial.begin(SERIAL_BAUDRATE);
  Serial.println("[ESP32-S3] Setup Configuration...");
  SERIAL_PORT.begin(SERIAL_BAUDRATE, SERIAL_8N1, U1RXD, U1TXD); // Mapping UART1's pins. Not using default ones.

  // TMC2209 Stepper Driver
  pinMode(TMC_EN, OUTPUT);
  digitalWrite(TMC_EN, LOW);
  pinMode(TMC_STEP, OUTPUT);
  pinMode(TMC_DIR, OUTPUT);
  TMCdriver.begin();
  TMCdriver.toff(5);  // Enables driver in software
  //TMCdriver.rms_current(800);
  TMCdriver.microsteps(2);
  TMCdriver.en_spreadCycle(false);
  TMCdriver.pwm_autoscale(true);
  TMCdriver.shaft(shaftDir);

  // WS2812B GRB LED
  pinMode(LED_PWR, OUTPUT);
  digitalWrite(LED_PWR, HIGH);
  FastLED.addLeds<LED_TYPE, LED_DIN, COLOR_ORDER>(LED, LED_NB);
  FastLED.setBrightness(LED_BRT);

  // AS5600L Magnetic Encoder
  Wire.begin(SDA_PIN, SCL_PIN);
  _AS5600L.begin(8); // Set dir pin. Dir pin is connected to GND. Pin 8 is not connected and only use for the begin().
  if(_AS5600L.isConnected()) Serial.println("[AS5600L] Connected.");
  _AS5600L.getCumulativePosition();
  _AS5600L.resetPosition(); // Set current position as initial position
  printTime();
  Serial.println("[AS5600L] Setting current position as initial position.");
  printTime();
  Serial.print("[AS5600L] Cumulative Position: ");
  Serial.print(position);
  Serial.print("\tRevolutions: ");
  Serial.println(revolutions);
}

void loop() {
  // LED ON every 1000ms
  if((millis() - timeLED) >= 1000){
    timeLED = millis();
    LED[0] = DarkBlue;
    FastLED.show();
    statusLED = true;
    //printTime();
    //Serial.println(" | [WS2812B] State: ON");
  }

  // LED OFF after being ON for 100ms
  if(statusLED == true){
    if((millis() - timeLED) >= 100){
      timeLED = millis();
      LED[0] = Blank; // Off
      FastLED.show();
      statusLED = false;
      //printTime();
      //Serial.println(" | [WS2812B] State: OFF");
    }
  }

  if(millis() - timeAS5600L >= 10){
    timeAS5600L = millis();
    position = _AS5600L.getCumulativePosition();
    revolutions = _AS5600L.getRevolutions();  // Cumulative Position is divided by 4096
    angle = (float(position) * float(360)) / float(12288);
    stepperMoveToAngle(stepperMoveBuffer[stepperMoveSelect]);
  }

  // Print position to serial if it changes
  if((lastPosition < (position-1)) || (lastPosition > (position+1))){
    lastPosition = position;
    printTime();
    Serial.print("[AS5600L] Cumulative Position: ");
    Serial.print(position);
    Serial.print("\tRevolutions: ");
    Serial.print(revolutions);
    Serial.print("\tAngle: ");
    Serial.println(angle);
  }
}

void printTime(void){
  int mSeconds = millis()%1000;
  int seconds = (millis()/1000)%60;
  int minutes = (millis()/60000)%60;
  char buffer[64];
  sprintf(buffer, "%dmin %ds %dmS | ", minutes, seconds, mSeconds);
  Serial.print(buffer);
}

void stepperMoveToAngle(float finalAngle){
  if((finalAngle >= (angle-0.3)) && (finalAngle <= (angle+0.3))){ // Accuracy of +/-0.3 degree
    TMCdriver.VACTUAL(0);
    if(stepperMoveStatus == 0){
      stepperMoveStatus = 1;  // Destination angle reach, ready to start delay.
    }else if(stepperMoveStatus == 1){
      timeMove = millis();
      stepperMoveStatus = 2; // Delay as been start, waiting.
    }else if(stepperMoveStatus == 2){
      if(millis() - timeMove >= moveDelay){
        stepperMoveStatus = 0; // Delay ended, ready to load next destination angle
        stepperMoveSelect++;
        stepperMoveSelect %= moveBufferLength;
      }
    }
    printTime();
    char buffer[128];
    sprintf(buffer, "[ESP32-S3] Destination angle of %f degrees reach.\tAccurate current angle: %f", finalAngle, angle);
    Serial.println(buffer);
  }
  else if(finalAngle <= (angle-0.3)){ // Accuracy of +/-0.3 degree
    shaftDir = false;
    TMCdriver.shaft(shaftDir);
    displacement = angle - finalAngle;
    if((displacement/10) >= 5){
      if(actualSpeed >= TMC_maxSpeed){
        actualSpeed = TMC_maxSpeed;
      }else{
        actualSpeed += TMC_minSpeed/3;
      }
      if(actualSpeed < TMC_minSpeed){
        actualSpeed = TMC_minSpeed;
      }
      TMCdriver.VACTUAL(actualSpeed);
    }else if((displacement/10) < 5){
      actualSpeed = (TMC_maxSpeed/5)*(displacement/10);
      if(actualSpeed > TMC_minSpeed){
        TMCdriver.VACTUAL(actualSpeed);
      }else{
        TMCdriver.VACTUAL(TMC_minSpeed);
      }
    }
  }
  else if(finalAngle >= (angle+0.3)){ // Accuracy of +/-0.3 degree
    shaftDir = true;
    TMCdriver.shaft(shaftDir);
    displacement = finalAngle - angle;
    if((displacement/10) >= 5){
      if(actualSpeed >= TMC_maxSpeed){
        actualSpeed = TMC_maxSpeed;
      }else{
        actualSpeed += TMC_minSpeed/3;
      }
      if(actualSpeed < TMC_minSpeed){
        actualSpeed = TMC_minSpeed;
      }
      TMCdriver.VACTUAL(actualSpeed);
    }else if((displacement/10) < 5){
      actualSpeed = (TMC_maxSpeed/5)*(displacement/10);
      if(actualSpeed > TMC_minSpeed){
        TMCdriver.VACTUAL(actualSpeed);
      }else{
        TMCdriver.VACTUAL(TMC_minSpeed);
      }
    }
  }
}