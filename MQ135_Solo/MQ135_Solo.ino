#include "MQ135.h"

int co2MQ135;

void setup() {
  Serial.begin(115200);
}

void loop() {
  MQ135 mq135 = MQ135(A0);
  co2MQ135 = mq135.getPPM();
  Serial.print("CO2: ");  
  Serial.print(co2MQ135);
  Serial.println("  ppm");
  delay(2000);
}
