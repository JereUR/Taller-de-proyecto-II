#include <WiFi.h>
#include "Adafruit_CCS811.h" //CSS811
#include <SoftwareSerial.h>  //MHZ19
#include <MHZ19.h>           //MHZ19

#define RX 25                //MHZ19
#define TX 27                //MHZ19
#define MQ135 36             //MQ135
#define MQ2 34               //MQ2
#define BUZZER_PIN 19        //Buzzer

Adafruit_CCS811 ccs;
SoftwareSerial mySerial(RX, TX);  //MHZ19
MHZ19 mhz;                        //MHZ19

//------------------Servidor Web en puerto 80---------------------

WiFiServer server(80);

//---------------------Credenciales de WiFi-----------------------

const char* ssid     = "VirusDetected 2.4Ghz";
const char* password = "losveguita";

//---------------------VARIABLES GLOBALES-------------------------
unsigned long timeElapse = 0;
int contconexion = 0;
int co2CSS, co2Z19, co2MQ135, co2MQ2;
int buzzerlvl1, buzzerlvl2;
String values = "";                 //Valores para html
String valueCSS = "";
String valueZ19 = "";
String valueMQ135 = "";
String valueMQ2 = "";
String header; // Variable para guardar el HTTP request

//------------------------CODIGO HTML------------------------------
String paginaInicio = "<!DOCTYPE html>"
"<html>"
"<head>"
"<meta charset='utf-8' />"
"<META HTTP-EQUIV='Refresh' CONTENT='1'>"   //Para refresh cada 1 segundo
"<title>Medidor de CO2 en el ambiente</title>"
"</head>"
"<body>"
"<center>"
"<h1>Medidor de CO2 en el ambiente</h1>"
"<h2> <u>Sensores:</u></h2>"
"<table>"
"<tr>"
"<td WIDTH=\"200\"; ALIGN = \"center\"><h3 <b>  CSS-811 </b></h3></td>"
"<td WIDTH=\"200\"; ALIGN = \"center\"><h3 <b>  MH-Z19B </b></h3></td>"
"<td WIDTH=\"200\"; ALIGN = \"center\"><h3 <b>  MQ-135 </b></h3></td>"
"<td WIDTH=\"200\"; ALIGN = \"center\"><h3 <b>  MQ-2 </b></h3></td>"
"</tr>"
"<tr>";

String paginaFin = "</tr>"
"</table>"
"</center>"
"</body>"
"</html>";

//---------------------------SETUP--------------------------------
void setup() {
  
  Serial.begin(115200);
  //---------------------------- CSS811 ----------------------------
  Serial.println("Calibrating CSS811..");
  if(!ccs.begin()){
    Serial.println("Failed to start sensor! Please check your wiring.");
    while(1);
  }
  while(!ccs.available());
  float temp = ccs.calculateTemperature();
  ccs.setTempOffset(temp - 25.0);

  //---------------------------- MHZ19 ----------------------------
  mhz.begin(mySerial);
  mhz.autoCalibration(false);     // make sure auto calibration is off
  Serial.println("Calibrating MHZ19B..");
  mhz.calibrate();

  //---------------------------- MQ135 ----------------------------
  pinMode(MQ135, INPUT);
  
  //---------------------------- MQ2 ----------------------------
  pinMode(MQ2, INPUT);

  //---------------------------- Buzzer ----------------------------
  pinMode(BUZZER_PIN, OUTPUT);
  
  //---------------------------- Conexión WIFI ----------------------------
  WiFi.begin(ssid, password);
  Serial.print("Connecting");
  //Cuento hasta 50, si no se puede conectar lo cancela
  while (WiFi.status() != WL_CONNECTED and contconexion <50) { 
    ++contconexion;
    delay(500);
    Serial.print(".");
  }
  if (contconexion <50) {      
      Serial.println("");
      Serial.println("WiFi conectado");
      Serial.println(WiFi.localIP());
      server.begin(); // inicio servidor
  }
  else { 
      Serial.println("");
      Serial.println("Error de conexion");
  }
}

//----------------------------LOOP----------------------------------

void loop(){
  //---------------------------- Reseteo valores de buzzer ----------------------------
  buzzerlvl1 = 0;
  buzzerlvl2 = 0;
  
  //---------------------------- Lectura CSS811 ----------------------------
  if(ccs.available()){
    float temp = ccs.calculateTemperature();
    if(ccs.readData()){
      co2CSS = ccs.geteCO2();
      Serial.println("CO2 CSS811: " + String(co2CSS) + " ppm");
    }
    else{
      Serial.println("ERROR!");
      while(1);
    }
  }

  //---------------------------- Lectura MHZ19 ----------------------------
  co2Z19 = mhz.getCO2();        
  if(mhz.errorCode == RESULT_OK){              // Si todo salio bien...
    Serial.print("CO2 MHZ19: " + String(co2Z19) + " ppm");
  } else{
    Serial.println("Failed to recieve CO2 value - Error");
    Serial.print("Response Code: ");
    Serial.println(mhz.errorCode);          // Codigo de error.
  }  
  
  //---------------------------- Lectura MQ2 ----------------------------
  co2MQ2 = analogRead(MQ2);
  Serial.println("Co2 MQ2: " + String(co2MQ2) + " ppm");

  //---------------------------- Lectura MQ135 ----------------------------
  co2MQ135 = analogRead(MQ135);
  Serial.println("Co2: " + String(co2MQ135) + " ppm");

  //---------------------------- Delay para sensores ----------------------------
  delay(1000);
  
  //---------------------------- Web ----------------------------
  WiFiClient client = server.available();   // Escucha a los clientes entrantes

  if (client) {                             // Si se conecta un nuevo cliente
    //Serial.println("New Client.");          // 
    String currentLine = "";                //
    while (client.connected()) {            // loop mientras el cliente está conectado
      if (client.available()) {             // si hay bytes para leer desde el cliente
        char c = client.read();             // lee un byte
        //Serial.write(c);                    // imprime ese byte en el monitor serial
        header += c;
        if (c == '\n') {                    // si el byte es un caracter de salto de linea
          // si la nueva linea está en blanco significa que es el fin del 
          // HTTP request del cliente, entonces respondo:
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            values = ""; //Reseteo values de sensores     
            //---------------------------- Muestro página web ----------------------------
            //---------------------------- CSS811 values ----------------------------
            
            if(co2CSS < 500){
              valueCSS = "<td WIDTH=\"200\"; ALIGN = \"center\"> <h4 style=\"color:#30C90F;\"</h4>" + String(co2CSS) + " ppm    <u> Óptimo </u>" + "</td>"; 
            } else if(co2CSS >= 500 and co2CSS <= 1000){
              buzzerlvl1++;
              valueCSS = "<td WIDTH=\"200\"; ALIGN = \"center\"> <h4 style=\"color:#F4D40C;\"</h4>" + String(co2CSS) + " ppm    <u> Precaución </u>" + "</td>";
            } else{
              buzzerlvl2++;
              valueCSS = "<td WIDTH=\"200\"; ALIGN = \"center\"> <h4 style=\"color:#FF3733;\"</h4>" + String(co2CSS) + " ppm    <u> Peligro </u>" + "</td>";
            }
            
            //---------------------------- MHZ19 values ----------------------------
            
            if(co2Z19 < 500){
              valueZ19 = "<td WIDTH=\"200\"; ALIGN = \"center\"> <h4 style=\"color:#30C90F;\"</h4>" + String(co2Z19) + " ppm    <u> Óptimo </u>" + "</td>"; 
            } else if(co2Z19 >= 500 and co2Z19 <= 1000){
              buzzerlvl1++;
              valueZ19 = "<td WIDTH=\"200\"; ALIGN = \"center\"> <h4 style=\"color:#F4D40C;\"</h4>" + String(co2Z19) + " ppm    <u> Precaución </u>" + "</td>";
            } else{
              buzzerlvl2++;
              valueZ19 = "<td WIDTH=\"200\"; ALIGN = \"center\"> <h4 style=\"color:#FF3733;\"</h4>" + String(co2Z19) + " ppm    <u> Peligro </u>" + "</td>";
            }

            //---------------------------- MQ135 values ----------------------------

            if(co2MQ135 < 500){
              valueMQ135 = "<td WIDTH=\"200\"; ALIGN = \"center\"> <h4 style=\"color:#30C90F;\"</h4>" + String(co2MQ135) + " ppm    <u> Óptimo </u>" + "</td>"; 
            } else if(co2MQ135 >= 500 and co2MQ135 <= 1000){
              buzzerlvl1++;
              valueMQ135 = "<td WIDTH=\"200\"; ALIGN = \"center\"> <h4 style=\"color:#F4D40C;\"</h4>" + String(co2MQ135) + " ppm    <u> Precaución </u>" + "</td>";
            } else{
              buzzerlvl2++;
              valueMQ135 = "<td WIDTH=\"200\"; ALIGN = \"center\"> <h4 style=\"color:#FF3733;\"</h4>" + String(co2MQ135) + " ppm    <u> Peligro </u>" + "</td>";
            }

            //---------------------------- MQ2 values ----------------------------

            if(co2MQ2 < 500){
              valueMQ2 = "<td WIDTH=\"200\"; ALIGN = \"center\"> <h4 style=\"color:#30C90F;\"</h4>" + String(co2MQ2) + " ppm    <u> Óptimo </u>" + "</td>"; 
            } else if(co2MQ2 >= 500 and co2MQ2 <= 1000){
              buzzerlvl1++;
              valueMQ2 = "<td WIDTH=\"200\"; ALIGN = \"center\"> <h4 style=\"color:#F4D40C;\"</h4>" + String(co2MQ2) + " ppm    <u> Precaución </u>" + "</td>";
            } else{
              buzzerlvl2;
              valueMQ2 = "<td WIDTH=\"200\"; ALIGN = \"center\"> <h4 style=\"color:#FF3733;\"</h4>" + String(co2MQ2) + " ppm    <u> Peligro </u>" + "</td>";
            }

            //---------------------------- final values ----------------------------
            
            values = valueCSS + valueZ19 + valueMQ135 + valueMQ2;
            client.println(paginaInicio + values + paginaFin);

            //---------------------------- Buzzer ----------------------------

            if(buzzerlvl1 > buzzerlvl2){
              digitalWrite(BUZZER_PIN, HIGH); // Enciendo nivel 1 (Dos pitidos).
              delay(200);
              digitalWrite(BUZZER_PIN, LOW);
              delay(100);
              digitalWrite(BUZZER_PIN, HIGH);
              delay(200);
              digitalWrite(BUZZER_PIN, LOW);
            }else if(buzzerlvl2 > 0){
              digitalWrite(BUZZER_PIN, HIGH); // Enciendo nivel 2 (Cinco pitidos).
              delay(200);
              digitalWrite(BUZZER_PIN, LOW);
              delay(100);
              digitalWrite(BUZZER_PIN, HIGH);
              delay(200);
              digitalWrite(BUZZER_PIN, LOW);
              delay(100);
              digitalWrite(BUZZER_PIN, HIGH);
              delay(200);
              digitalWrite(BUZZER_PIN, LOW);
              delay(100);
              digitalWrite(BUZZER_PIN, HIGH);
              delay(200);
              digitalWrite(BUZZER_PIN, LOW);
              delay(100);
              digitalWrite(BUZZER_PIN, HIGH);
              delay(200);
              digitalWrite(BUZZER_PIN, LOW);
            }
            
            //---------------------------- final web ----------------------------
            // la respuesta HTTP temina con una linea en blanco
            client.println();
            break;
          } else { // si tengo una nueva linea limpio currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // si C es distinto al caracter de retorno de carro
          currentLine += c;      // lo agrega al final de currentLine
        }
      }
    }
    // Limpio variable header
    header = "";
    // Cierro conexión
    client.stop();
    //Serial.println("Client disconnected.");
    Serial.println("");
  }
}
