#define MQ2 34

int co2MQ2 = 0;

void setup() {
  Serial.begin(115200);
  pinMode(MQ2, INPUT);
}

void loop() {
  co2MQ2 = analogRead(MQ2);
  Serial.println("Co2: " + String(co2MQ2) + " ppm");
  delay(1000);
}
