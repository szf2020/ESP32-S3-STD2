#include <FastLED.h>

// Define for on-board WS2812B LED (Pinout)
#define LED_DIN          9
#define LED_PWR          10
#define LED_NB           1
#define LED_BRT          20
#define LED_TYPE         WS2812B
#define COLOR_ORDER      GRB

CRGB LED[LED_NB];

bool LED_STATE = false;  // Used to know if the LED is ON or OFF

void setup() {
  Serial.begin(115200);
  Serial.print("Setup Configuration...");

  pinMode(LED_PWR, OUTPUT);
  digitalWrite(LED_PWR, HIGH);

  FastLED.addLeds<LED_TYPE, LED_DIN, COLOR_ORDER>(LED, LED_NB);
  FastLED.setBrightness(LED_BRT);
}

void loop() {
  static uint32_t lastTime = 0;

  if((millis() - lastTime) >= 1000){
    lastTime = millis();
    LED[0] = 0x01244a;  // Dark Blue Sky
    FastLED.show();
    LED_STATE = true;
    Serial.println("LED STATE: ON");
  }

  if(LED_STATE == true){
    if((millis() - lastTime) >= 100){
      lastTime = millis();
      LED[0] = CRGB(0, 0, 0); // Off
      FastLED.show();
      LED_STATE = false;
      Serial.println("LED STATE: OFF");
    }
  }
}