#include "AS5600.h"
#include "Wire.h"

AS5600L _AS5600L;  // use default Wire

static uint32_t lastTime = 0;

void setup(){
  Serial.begin(115200);

  Wire.begin(2, 1);  // SDA pin 2, SDL pin 1.
  _AS5600L.begin(8);  // Set dir pin. Dir pin is connected to GND. Pin 8 is not connected and only use for the begin().
  if(_AS5600L.isConnected()){
    Serial.println("[AS5600L] Connected.");
  }
  _AS5600L.getCumulativePosition();
  _AS5600L.resetPosition();
  delay(1000);
}

void loop(){
  if (millis() - lastTime >= 100){  // Read position every 100ms. Adjust depending on RPM.
    lastTime = millis();
    Serial.print("[AS5600L] Cumulative Position: ");
    Serial.print(_AS5600L.getCumulativePosition());
    Serial.print("\tRevolutions: ");
    Serial.println(_AS5600L.getRevolutions());
  }

  if (_AS5600L.getRevolutions() >= 9){
    _AS5600L.resetPosition();
  }
}