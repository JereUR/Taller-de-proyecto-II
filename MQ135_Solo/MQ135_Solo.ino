#define MQ135 36

int co2MQ135;

void setup() {
  Serial.begin(115200);
  pinMode(MQ135, INPUT);
}

void loop() {
  co2MQ135 = analogRead(MQ135);
  Serial.println("Co2: " + String(co2MQ135) + " ppm");
  delay(1000);
}
