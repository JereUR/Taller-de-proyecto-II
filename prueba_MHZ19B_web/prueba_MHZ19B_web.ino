#include <WiFi.h>
#include <SoftwareSerial.h>
#include <MHZ19.h>
 
#define RX 25
#define TX 27

SoftwareSerial mySerial(RX, TX);
MHZ19 mhz;

//------------------Servidor Web en puerto 80---------------------

WiFiServer server(80);

//---------------------Credenciales de WiFi-----------------------

const char* ssid     = "VirusDetected 2.4Ghz";
const char* password = "losveguita";

//---------------------VARIABLES GLOBALES-------------------------
int contconexion = 0;
unsigned long timeElapse = 0;
int co2CSS, co2Z19;
String values = "";
String valueCSS = "";
String valueZ19 = "";
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
  
  // Conexión WIFI
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
                     
            // Muestro página web
            values = "<td WIDTH=\"200\"; ALIGN = \"center\"> <h4 style=\"color:#30C90F;\"</h4>" + String(co2Z19) + " ppm    <u> Óptimo </u>" + "</td>";
            if(co2Z19 < 500){
              valueZ19 = "<td WIDTH=\"200\"; ALIGN = \"center\"> <h4 style=\"color:#30C90F;\"</h4>" + String(co2Z19) + " ppm    <u> Óptimo </u>" + "</td>"; 
            } else if(co2Z19 > 500 and co2Z19 < 1000){
              valueZ19 = "<td WIDTH=\"200\"; ALIGN = \"center\"> <h4 style=\"color:#F4D40C;\"</h4>" + String(co2Z19) + " ppm    <u> Precaución </u>" + "</td>";
            } else{
              valueZ19 = "<td WIDTH=\"200\"; ALIGN = \"center\"> <h4 style=\"color:#FF3733;\"</h4>" + String(co2Z19) + " ppm    <u> Peligro </u>" + "</td>";
            }
            values = values + valueZ19 + "<td WIDTH=\"200\"; ALIGN = \"center\"> <h4 style=\"color:#30C90F;\"</h4>" + String(co2Z19) + " ppm    <u> Óptimo </u>" + "</td>";
            client.println(paginaInicio + values + paginaFin);
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
    //Serial.println("");
  }
}
