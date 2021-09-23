const int BUZZER_PIN = 19; 

int x = 0;

void setup() {
  Serial.begin(9600);               
  pinMode(BUZZER_PIN, OUTPUT);       // Configurar pin 19 como salida
}

void loop() {
  if (x <= 3) {
    digitalWrite(BUZZER_PIN, HIGH); // Encender
    delay(100);
    digitalWrite(BUZZER_PIN, LOW);
    delay(100);
  }
  else if (x > 3) {
    digitalWrite(BUZZER_PIN, LOW);  // Apagar
  }
  x++;
  Serial.println("x= " + x);
  delay(1000);
}
