/**
* ESP ESPEEDOMETER BY FT
* escreve no e paper a distancia e  avelocidade lida por um sensor hall
* v2
* SPEED / DISTANCE / POWER METER / TIMER / 
* SENT DATA BY WIFI
 */
#include <ESP8266WiFi.h>        // Include the Wi-Fi library
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid     = "HadesLan";         // The SSID (name) of the Wi-Fi network you want to connect to
const char* password = "Password2";     // The password of the Wi-Fi network

#include <SPI.h>
#include "epd2in9b.h"
#include "imagedata.h"
#include "epdpaint.h"

#define COLORED     0
#define UNCOLORED   1

// speed control
int rev;
volatile float mediumPower;
float speed;
double oldSpeed;
float refreshDisplayMillis;
float oldMillisSerial;
double oldSpeedMillis;
double oldCadenceMillis;
int acuRev;
int oldAcuRev;
int cadence;
long acuTime;
const float tireTam = 2.105;

// display


//Controll LED
//#define LED 2
bool ledOn;
bool wifi;

// ports
int speedPortPin = 0;
int cadencePortPin = 5;

void ICACHE_RAM_ATTR handleInterrupt();
void ICACHE_RAM_ATTR handleInterruptCadence();


void setup() {
    Serial.begin(115200);
    pinMode(speedPortPin, INPUT_PULLUP);
 //   pinMode(LED, OUTPUT);
    attachInterrupt(digitalPinToInterrupt(speedPortPin), handleInterrupt, RISING);  
    attachInterrupt(digitalPinToInterrupt(cadencePortPin), handleInterruptCadence, RISING);  


// init vars
rev=0;
mediumPower=0;
speed=0;
oldSpeed=0;
refreshDisplayMillis=0;
oldMillisSerial=0;
oldSpeedMillis=0;
oldCadenceMillis=0;
acuRev=0;
oldAcuRev=0;
cadence=0;
acuTime=0;
ledOn=true;
wifi=false;

      // setup display
      // put your setup code here, to run once:
      Epd epd;
      if (epd.Init() != 0) {
        Serial.print("e-Paper init failed");
        return;
      }
      /* This clears the SRAM of the e-paper display */
      epd.ClearFrame();
      unsigned char image[1024];
      Paint paint(image, 128, 28);    //width should be the multiple of 8 

      //cabeçalho
      paint.Clear(COLORED);
      char texto[] = "BIKE SPEED ...";
      paint.DrawStringAt(15, 7, texto, &Font12, UNCOLORED);
      epd.SetPartialWindowRed(paint.GetImage(), 0, 20, paint.GetWidth(), paint.GetHeight());
      /* This displays the data from the SRAM in e-Paper module */
      epd.DisplayFrame();    
      /* This displays an image */
      epd.DisplayFrame(IMAGE_BLACK, IMAGE_RED);
     /* Deep sleep */
      epd.Sleep();   

// wifi
  delay(10);
  WiFi.begin(ssid, password);             // Connect to the network
  Serial.print("Connecting to ");
  Serial.print(ssid); Serial.println(" ...");
  int i = 0;
  while (WiFi.status() != WL_CONNECTED &&  i < 60) { // Wait for the Wi-Fi to connect
    delay(1000);
    Serial.print(++i); Serial.print(' ');
  }
  Serial.println('\n');

  if (WiFi.status() == WL_CONNECTED)
  {
    wifi=true;
    Serial.println("Connection established!");  
    Serial.print("IP address:\t");
    Serial.println(WiFi.localIP());         // Send the IP address of the ESP8266 to the computer
  }else
  {
    Serial.print("WIFI not conected:\t");
  }

}

void loop() {

// se tiver + q 4 segundos sem receber info detecta parado
if (millis()-oldSpeedMillis > 4000){
  speed=0;
  oldSpeedMillis=millis();
}

printDisplay();

if ((millis() - oldMillisSerial) > 3000){
    acuTime += (millis() - oldMillisSerial) / 1000;
    Serial.print("Speeed:  ");  Serial.print(speed); Serial.println(" Km/h");
    Serial.print("Distance:");  Serial.print((acuRev * tireTam) / 1000); Serial.println(" Km");
    Serial.print("Cadence: ");  Serial.println(cadence);
    Serial.print("Power:   ");  Serial.print(mediumPower); Serial.println(" Watts");
    Serial.print("Time:    ");  Serial.print(acuTime/60); Serial.println(" Min");
    Serial.println("");

    if (ledOn)
    {
//      digitalWrite(LED, LOW);
      ledOn = false;
    }else
    {
//      digitalWrite(LED, HIGH);
      ledOn = true;
    }

  if (wifi)
  {
    sendData();
  }
  
    oldMillisSerial = millis(); 
    oldAcuRev = acuRev;
}
  
//double revInTime = rev / (millis() - oldMillis);
//int rpm = (revInTime * 60000);
//int intSpeed = (revInTime * 2.105 * 3600)
  
  
}

void handleInterrupt()
{
  rev++;
  double difMillis = (millis() - oldSpeedMillis);
  double revInTime = 0;
  
  if (difMillis > 10)
  {
    revInTime = rev / difMillis;
    acuRev++;
    calcPower(speed);  
    speed = (revInTime * tireTam * 3600);
  }else
  {
    speed = oldSpeed;
  } 
  
    rev = 0;    
    oldSpeedMillis = millis();  
}

void handleInterruptCadence()
{
  double difMillis = (millis() - oldCadenceMillis);
  cadence = (1 / (difMillis/1000)) * 60;
  oldCadenceMillis = millis();  
}

void printDisplay()
{ 
  // min time to refresh display after 16s
  if (((millis() - refreshDisplayMillis) > 20000) && (speed > (oldSpeed +2) || speed < (oldSpeed -2)))
  {   
    Serial.println(speed);
    Serial.println((acuRev * tireTam) / 1000);
    Serial.println(cadence);
    Serial.println(mediumPower);
    Serial.println(acuTime/60);   


  Epd epd;
  if (epd.Init() != 0) {
    Serial.print("e-Paper init failed");
    return;
  }
  /* This clears the SRAM of the e-paper display */
  epd.ClearFrame();
  unsigned char image[1024];
  Paint paint(image, 128, 28);

      paint.Clear(COLORED);

            
      char titulo[] = "BIKE SPEED";
      if (wifi)
      {
        String str = String("BIKE SPEED Wifi");
        str.toCharArray(titulo, 16);
      }
      
      paint.DrawStringAt(15, 7, titulo, &Font12, UNCOLORED);
      epd.SetPartialWindowRed(paint.GetImage(), 0, 20, paint.GetWidth(), paint.GetHeight());
  
  // LCD Speed
  char texto[] = "0";
  int intSpeed = speed;
  String strvel = String(intSpeed) + "Km/h";
  strvel.toCharArray(texto,8); 
    paint.Clear(UNCOLORED);
    paint.DrawStringAt(2, 2, texto, &Font24, COLORED);
    epd.SetPartialWindowBlack(paint.GetImage(), 0, 64, paint.GetWidth(), paint.GetHeight());

  // LCD distancia
  char dist[] = "0";
  String strDist = String((acuRev * tireTam) / 1000) + " Km";
  strDist.toCharArray(dist, 9);  
    paint.Clear(COLORED);
    paint.DrawStringAt(5, 7, dist, &Font20, UNCOLORED);
    epd.SetPartialWindowBlack(paint.GetImage(), 0, 104, paint.GetWidth(), paint.GetHeight());

  // LCD - Potencia
    paint.Clear(UNCOLORED);
    paint.DrawStringAt(30, 2, "POWER", &Font16, COLORED);
    epd.SetPartialWindowBlack(paint.GetImage(), 0, 144, paint.GetWidth(), paint.GetHeight());
  char pPower[] = "0";
  String strPower = String(mediumPower) + " Watts";
  strPower.toCharArray(pPower, 11);
    paint.Clear(UNCOLORED);
    paint.DrawStringAt(0, 2, pPower, &Font20, COLORED);
    epd.SetPartialWindowBlack(paint.GetImage(), 0, 164, paint.GetWidth(), paint.GetHeight());

 // Cadence
    paint.Clear(UNCOLORED);
    paint.DrawStringAt(25, 2, "Cadence", &Font16, COLORED);
    epd.SetPartialWindowBlack(paint.GetImage(), 0, 186, paint.GetWidth(), paint.GetHeight());
  char sCad[] = "0";
  String strCad = String(cadence);
  strCad.toCharArray(sCad, 3);
    paint.Clear(UNCOLORED);
    paint.DrawStringAt(55, 2, sCad, &Font24, COLORED);
    epd.SetPartialWindowBlack(paint.GetImage(), 0, 200, paint.GetWidth(), paint.GetHeight());  
  
  
  // LCD - Timer
    paint.Clear(UNCOLORED);
    paint.DrawStringAt(35, 2, "Timer", &Font16, COLORED);
    epd.SetPartialWindowBlack(paint.GetImage(), 0, 232, paint.GetWidth(), paint.GetHeight());
  char sTimer[] = "0:00.0";
  String strTime = String(acuTime/60) + " Min";
  strTime.toCharArray(sTimer, 11);
    paint.Clear(UNCOLORED);
    paint.DrawStringAt(0, 2, sTimer, &Font20, COLORED);
    epd.SetPartialWindowBlack(paint.GetImage(), 0, 236, paint.GetWidth(), paint.GetHeight());

  //Escreve LCD e coloca em sleep
    epd.DisplayFrame();
    epd.DisplayFrame(IMAGE_BLACK, IMAGE_RED);
    epd.Sleep();

    // atualiza velocidade atual e tempo atual
    refreshDisplayMillis = millis();
    oldSpeed = speed;   
  } 
}

void calcPower(float speedkph)
{
  float speedMile = speed / 1.6;  
  double powerspeed = 1;
  double level = 1;

  if (speedMile > 1 && speedMile < 60)
  {  
    if (speedMile >= 1 && speedMile < 2){
      powerspeed = 5.3;
    } if (speedMile >= 2 && speedMile < 3){
      powerspeed = 10.6;
    } if (speedMile >= 3 && speedMile < 4){
      powerspeed = 16.3;
    } if (speedMile >= 4 && speedMile < 5){
      powerspeed = 22.2;
    } if (speedMile >= 5 && speedMile < 6){
      powerspeed = 28.7;
    } if (speedMile > 6 && speedMile < 7){
      powerspeed = 43;
    } if (speedMile > 7 && speedMile < 8){
      powerspeed = 52;
    } if (speedMile > 8 && speedMile < 9){
      powerspeed = 61.6;
    } if (speedMile > 10 && speedMile < 11){
      powerspeed = 72.1;
    } if (speedMile > 11 && speedMile < 12){
      powerspeed = 83.9;
    } if (speedMile > 12 && speedMile < 13){
      powerspeed = 96.9;
    } if (speedMile > 13 && speedMile < 14){
      powerspeed = 111.4;
    } if (speedMile > 14 && speedMile < 15){
      powerspeed = 127.4;
    } if (speedMile > 15 && speedMile < 16){
      powerspeed = 145.1;
    } if (speedMile > 16 && speedMile < 17){
      powerspeed = 164;
    } if (speedMile > 17 && speedMile < 18){
      powerspeed = 185;
    } if (speedMile > 18 && speedMile < 19){
      powerspeed = 209;
    } if (speedMile > 19 && speedMile < 20){
      powerspeed = 234;
    } if (speedMile > 20 && speedMile < 21){
      powerspeed = 262;
    } if (speedMile > 21 && speedMile < 22){
      powerspeed = 292;
    } if (speedMile > 22 && speedMile < 23){
      powerspeed = 324;
    } if (speedMile > 23 && speedMile < 24){
      powerspeed = 360;
    } if (speedMile > 24 && speedMile < 25){
      powerspeed = 397;
    } if (speedMile > 25 && speedMile < 26){
      powerspeed = 438;
    } if (speedMile > 26 && speedMile < 27){
      powerspeed = 482;
    } if (speedMile > 27 && speedMile < 28){
      powerspeed = 529;
    } if (speedMile > 28 && speedMile < 29){
      powerspeed = 578;
    } if (speedMile > 29 && speedMile < 30){
      powerspeed = 632;
    } if (speedMile > 30 && speedMile < 31){
      powerspeed = 688;
    } if (speedMile > 31 && speedMile < 32){
      powerspeed = 748;
    } if (speedMile > 32 && speedMile < 33){
      powerspeed = 812;
    } if (speedMile > 33 && speedMile < 34){
      powerspeed = 880;
    } if (speedMile > 34 && speedMile < 35){
      powerspeed = 952;
    } if (speedMile > 35 && speedMile < 36){
      powerspeed = 1227;
    } if (speedMile > 36 && speedMile < 37){
      powerspeed = 1107;
    } if (speedMile > 37 && speedMile < 38){
      powerspeed = 1190;
    } if (speedMile > 38 && speedMile < 39){
      powerspeed = 1279;
    } if (speedMile > 39 && speedMile < 40){
      powerspeed = 1371;
    } if (speedMile > 40 && speedMile < 41){
      powerspeed = 1469;
    } if (speedMile > 41 && speedMile < 42){
      powerspeed = 1571.4;
    } if (speedMile > 42){
      powerspeed = 1790;
    }
  
    int speedInt = speedMile;
    float finalPower = ((speedMile * powerspeed) / speedInt) * level;
    // coment in PROD;
    //Serial.print("I Power:"); Serial.println(finalPower); 
    
    if (finalPower > 0){
      mediumPower = (finalPower + mediumPower) / 2;
    }
  }
}

void sendData() {
  // Check WiFi Status
  if (WiFi.status() == WL_CONNECTED) {  //Si hay acceso a internet.
    HTTPClient http;  //Objeto de clase HTTPClient
    //http.begin("http://jsonplaceholder.typicode.com/todos/1"); //Conecta con servidor.
    http.begin("http://18.223.239.136/api/v1/users/grant_client");
    int httpCode = http.GET(); //Realiza una petición al servidor.
                                                               
    if (httpCode > 0) { //Si hay algo que recibir del servidor.
//      const size_t bufferSize = JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(8) + 370;
//      DynamicJsonBuffer jsonBuffer(bufferSize);
//      JsonObject& root = jsonBuffer.parseObject(http.getString());
      // Parameters
//      int id = root["id"]; // 1
//      const char * name = root ["secret"]; // "Leanne Graham"
      //const char * username = root ["username"]; // "Bret"
      //const char * email = root ["email"]; // "Sincero@april.biz"
//      Serial.print("id: ");
//      Serial.println(id);
//      Serial.print("Secret: ");
//      Serial.println(name);
//    Serial.print("Username:");
//    Serial.println(username);
//    Serial.print("Email:");
//    Serial.println(email);
    }
    http.end();   //Close connection
  
    String message;
  int intSpeed = speed;
  String strvel = String(intSpeed);
 //   Serial.println((acuRev * tireTam) / 1000);
 //   Serial.println(cadence);
  //  Serial.println(mediumPower);
  //  Serial.println(acuTime/60);  

    message = "{speed:" + strvel + ", Distance:" + ((acuRev * tireTam) / 1000) + ", Cadence:"+ cadence +", Power:"+ mediumPower +",  Time:" + acuTime/60 + ", data: 0}";
    Serial.println(message);
    http.begin("http://177.86.166.68:47160/utilsservice/api/utils");
    http.addHeader("Content-Type", "application/json"); //http.addHeader("Content-Type", "text/plain");
    httpCode = http.POST(message);
    if(httpCode > 0) {
      Serial.printf("[HTTP] POST... code: %d\n", httpCode);
      if(httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println(payload);
      }
    }
    else {
      Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  }
}
