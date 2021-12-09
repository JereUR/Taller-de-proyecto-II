#include <TimeLib.h>         //Temporizador
#include <WiFi.h>            //Sitio web
#include "Adafruit_CCS811.h" //CSS811
#include <SoftwareSerial.h>  //MHZ19
#include <MHZ19.h>           //MHZ19

#define RX 25                //MHZ19
#define TX 27                //MHZ19
#define MQ135 36             //MQ135
#define MQ2 34               //MQ2
#define BUZZER_PIN 19        //Buzzer

Adafruit_CCS811 ccs;              //CSS811
SoftwareSerial mySerial(RX, TX);  //MHZ19
MHZ19 mhz;                        //MHZ19
time_t fechaInicio;               //Temporizador
time_t fechaActual;               //Temporizador

//------------------Servidor Web en puerto 80---------------------

WiFiServer server(80);

//---------------------Credenciales de WiFi-----------------------

const char* ssid     = "TdPII";
const char* password = "12345678";

//---------------------VARIABLES GLOBALES-------------------------
int ticks = 0;
bool buzzerEnable = false;
int contconexion = 0;
int co2CSS, co2Z19, co2MQ135, co2MQ2;
int buzzerlvl1, buzzerlvl2, hours, minutes, seconds;
String values = "";                 //Valores para html
String valueCSS = "";
String valueZ19 = "";
String valueMQ135 = "";
String valueMQ2 = "";
String timer = "";
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
"<table style=\"box-shadow: 10px 5px 5px #D6D5D5; background-color: #F3F2F2;border-radius: 10px;\">"
"<tr>"
"<td WIDTH=\"200\"; ALIGN = \"center\"><h3 <b>  CSS-811 </b></h3></td>"
"<td WIDTH=\"200\"; ALIGN = \"center\"><h3 <b>  MH-Z19B </b></h3></td>"
"<td WIDTH=\"200\"; ALIGN = \"center\"><h3 <b>  MQ-135 </b></h3></td>"
"<td WIDTH=\"200\"; ALIGN = \"center\"><h3 <b>  MQ-2 </b></h3></td>"
"</tr>"
"<tr>";

String paginaFin = "</center>"
"</body>"
"</html>";

void settingCSS811(){     //Función de seteado de sensor CSS811
  Serial.println("Calibrating CSS811...");
  if(!ccs.begin()){
    Serial.println("Failed to start sensor! Please check your wiring.");
    while(1);
  }
  while(!ccs.available());
  float temp = ccs.calculateTemperature();
  ccs.setTempOffset(temp - 25.0);
}

void settingMHZ19(){       //Función de seteado de sensor MH-Z19B
  mySerial.begin(9600); 
  mhz.begin(mySerial);
  mhz.autoCalibration(false);     
  Serial.println("Calibrating MHZ19B...");
  mhz.calibrate();
}

void settingMQ135(){      //Función de seteado de sensor MQ135
  Serial.println("Calibrating MQ135...");
  pinMode(MQ135, INPUT);
}

void settingMQ2(){        //Función de seteado de sensor MQ2
  Serial.println("Calibrating MQ2...");
  pinMode(MQ2, INPUT);
}

void settingBuzzer(){     //Función de seteado de buzzer
  Serial.println("Calibrating Buzzer...");
  pinMode(BUZZER_PIN, OUTPUT);
}

void settingWiFi(){       //Función de seteado de wifi de ESP32 como Access Point
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  
  server.begin();
}

int readCSS811(){         //Función de lectura de sensor CSS811
  if(ccs.available()){
    float temp = ccs.calculateTemperature();
    if(ccs.readData()){
      Serial.println("CO2 CSS811: " + String(co2CSS) + " ppm");
      return ccs.geteCO2();
    }
    /*Queda deshabilitado el buzzer para este sensor
    if(co2CSS >= 500 and co2CSS <= 1000){
      buzzerlvl1++;
    } else if (co2CSS > 1000){
      buzzerlvl2++;
    }*/
  }
}

int readMHZ19(){          //Función de lectura de sensor MH-Z19B
  int value = mhz.getCO2();        
  if(mhz.errorCode == RESULT_OK){              // Si todo salio bien...
    Serial.println("CO2 MHZ19: " + String(co2Z19) + " ppm");
    /*Queda deshabilitado el buzzer para este sensor
    if(co2Z19 >= 500 and co2Z19 <= 1000){
      buzzerlvl1++;
    } else if (co2Z19 > 1000){
      buzzerlvl2++;
    }*/
    return value;
  } else{
    Serial.println("Failed to recieve CO2 MH-Z19B");
    Serial.print("Response Code: ");
    Serial.println(mhz.errorCode); 
    return 0;         
  }
}

int readMQ2(){          //Función de lectura de sensor MQ2
  int value = analogRead(MQ2);
  Serial.println("Co2 MQ2: " + String(co2MQ2) + " ppm");
  if(co2MQ2 >= 500 and co2MQ2 <= 1000){
      buzzerlvl1++;
  } else if (co2MQ2 > 1000){
      buzzerlvl2++;
  }
  return value;
}

int readMQ135(){        //Función de lectura de sensor MQ135
  int value = analogRead(MQ135);
  Serial.println("Co2 MQ135: " + String(co2MQ135) + " ppm");
  if(co2MQ135 >= 500 and co2MQ135 <= 1000){
      buzzerlvl1++;
  } else if (co2MQ135 > 1000){
      buzzerlvl2++;
  }
  return value;
}
//---------------------------SETUP--------------------------------
void setup() {

  fechaInicio = now();
  Serial.begin(115200);
  //---------------------------- CSS811 ----------------------------
  
  settingCSS811();
  
  //---------------------------- MHZ19 ----------------------------
  
  settingMHZ19();

  //---------------------------- MQ135 ----------------------------
  
  settingMQ135();
  
  //---------------------------- MQ2 ----------------------------

  settingMQ2();

  //---------------------------- Buzzer ----------------------------
  
  settingBuzzer();

  //---------------------------- Conexión WIFI ----------------------------

  settingWiFi();
  
}

//----------------------------LOOP----------------------------------

void loop(){
  //---------------------------- Reseteo valores de buzzer ----------------------------
  buzzerlvl1 = 0;
  buzzerlvl2 = 0;
  
  //---------------------------- Lectura CSS811 ----------------------------

  co2CSS = readCSS811();

  //---------------------------- Lectura MHZ19 ----------------------------

  co2Z19 = readMHZ19();
  
  //---------------------------- Lectura MQ2 ----------------------------

  co2MQ2 = readMQ2();

  //---------------------------- Lectura MQ135 ----------------------------

  co2MQ135 = readMQ135();

  //---------------------------- Delay para sensores y contador de ticks para buzzer ----------------------------
  delay(1000);
  ticks++;
  if(ticks >= 2400){       //Cuenta 40 minutos
    buzzerEnable = true;
  }
  //---------------------------- Actualizo timer ----------------------------
  fechaActual = now();
  hours = hour(fechaActual) - hour(fechaInicio);
  minutes = minute(fechaActual) - minute(fechaInicio);
  seconds = second(fechaActual) - second(fechaInicio);
  
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
              valueCSS = "<td WIDTH=\"200\"; ALIGN = \"center\"> <h4 style=\"color:#F4D40C;\"</h4>" + String(co2CSS) + " ppm    <u> Precaución </u>" + "</td>";
            } else{
              valueCSS = "<td WIDTH=\"200\"; ALIGN = \"center\"> <h4 style=\"color:#FF3733;\"</h4>" + String(co2CSS) + " ppm    <u> Peligro </u>" + "</td>";
            }
            
            //---------------------------- MHZ19 values ----------------------------
            
            if(co2Z19 < 500){
              valueZ19 = "<td WIDTH=\"200\"; ALIGN = \"center\"> <h4 style=\"color:#30C90F;\"</h4>" + String(co2Z19) + " ppm    <u> Óptimo </u>" + "</td>"; 
            } else if(co2Z19 >= 500 and co2Z19 <= 1000){
              valueZ19 = "<td WIDTH=\"200\"; ALIGN = \"center\"> <h4 style=\"color:#F4D40C;\"</h4>" + String(co2Z19) + " ppm    <u> Precaución </u>" + "</td>";
            } else{
              valueZ19 = "<td WIDTH=\"200\"; ALIGN = \"center\"> <h4 style=\"color:#FF3733;\"</h4>" + String(co2Z19) + " ppm    <u> Peligro </u>" + "</td>";
            }

            //---------------------------- MQ135 values ----------------------------

            if(co2MQ135 < 500){
              valueMQ135 = "<td WIDTH=\"200\"; ALIGN = \"center\"> <h4 style=\"color:#30C90F;\"</h4>" + String(co2MQ135) + " ppm    <u> Óptimo </u>" + "</td>"; 
            } else if(co2MQ135 >= 500 and co2MQ135 <= 1000){
              valueMQ135 = "<td WIDTH=\"200\"; ALIGN = \"center\"> <h4 style=\"color:#F4D40C;\"</h4>" + String(co2MQ135) + " ppm    <u> Precaución </u>" + "</td>";
            } else{
              valueMQ135 = "<td WIDTH=\"200\"; ALIGN = \"center\"> <h4 style=\"color:#FF3733;\"</h4>" + String(co2MQ135) + " ppm    <u> Peligro </u>" + "</td>";
            }

            //---------------------------- MQ2 values ----------------------------
            
            if(co2MQ2 < 500){
              valueMQ2 = "<td WIDTH=\"200\"; ALIGN = \"center\"> <h4 style=\"color:#30C90F;\"</h4>" + String(co2MQ2) + " ppm    <u> Óptimo </u>" + "</td>"; 
            } else if(co2MQ2 >= 500 and co2MQ2 <= 1000){
              valueMQ2 = "<td WIDTH=\"200\"; ALIGN = \"center\"> <h4 style=\"color:#F4D40C;\"</h4>" + String(co2MQ2) + " ppm    <u> Precaución </u>" + "</td>";
            } else{
              valueMQ2 = "<td WIDTH=\"200\"; ALIGN = \"center\"> <h4 style=\"color:#FF3733;\"</h4>" + String(co2MQ2) + " ppm    <u> Peligro </u>" + "</td>";
            }

            //---------------------------- final values ----------------------------
            
            values = valueCSS + valueZ19 + valueMQ135 + valueMQ2;
            timer = "</tr>"
            "</table>"
             "<h3 style=\"border: solid #1E37ED; background-color: #8BADF8; width: fit-content; padding: 10px; border-radius: 10px;\"<b>  Tiempo transcurrido: " + String(hours) + ":" + String(minutes) + ":" + String(seconds) + "</b></h3>";
            client.println(paginaInicio + values + timer + paginaFin);
            
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
    //---------------------------- Buzzer ----------------------------
    if(buzzerEnable){                   //Luego de 40 minutos se activa buzzer (Tiempo aproximado de estabilización de sensores)
      if(buzzerlvl1 > buzzerlvl2){
        digitalWrite(BUZZER_PIN, HIGH); // Enciendo nivel 1 (Un pitido largo).
        delay(500);
        digitalWrite(BUZZER_PIN, LOW);
       }else if(buzzerlvl2 > 0){
         digitalWrite(BUZZER_PIN, HIGH); // Enciendo nivel 2 (3 pitidos cortos).
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
    }
    // Limpio variable header
    header = "";
    // Cierro conexión
    client.stop();
    //Serial.println("Client disconnected.");
    Serial.println("");
  }
}
