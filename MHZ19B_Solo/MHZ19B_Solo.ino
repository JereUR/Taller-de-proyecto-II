#include <SoftwareSerial.h>
#include <MHZ19.h>

#define RX 25
#define TX 27

int co2Z19;
unsigned long timeElapse = 0;

SoftwareSerial mySerial(RX, TX);
MHZ19 mhz;

void setup() {
  Serial.begin(9600);
  Serial.println("Starting...");
  mySerial.begin(9600);                              
  mhz.begin(mySerial);
  mhz.autoCalibration(false);     // make sure auto calibration is off
  Serial.print("ABC Status: "); mhz.getABC() ? Serial.println("ON") :  Serial.println("OFF");  // now print it's status
  //Serial.println("Waiting 20 minutes to stabalise...");
   /* if you don't need to wait (it's already been this amount of time), remove the 2 lines */
 /* timeElapse = 12e5;   //  20 minutes in milliseconds
  delay(timeElapse);*/    //  wait this duration
  Serial.println("Calibrating..");
  mhz.calibrate();
}

void loop() {
   if (millis() - timeElapse >= 1000){  // Check if interval has elapsed (non-blocking delay() equivilant)
    co2Z19 = mhz.getCO2();        
    if(mhz.errorCode == RESULT_OK){              // Si todo salio bien...
      Serial.print("CO2: ");
      Serial.println(String(co2Z19) + " ppm");
    } else{
      Serial.println("Failed to recieve CO2 value - Error");
      Serial.print("Response Code: ");
      Serial.println(mhz.errorCode);          // Codigo de error.
    } 
    timeElapse = millis();    // Update inerval
  }
}
