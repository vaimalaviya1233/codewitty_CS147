#include <Arduino.h>
#include <TFT_eSPI.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <string>
#include <iostream>
#include <sstream>
#include <Arduino_JSON.h>



int soilMoistureSensor();
int uvSensor();
void display();


#define MoistureSensor  39
#define UVIN 37 
#define REF_3v3 38

// never comment, needed for everything...
TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h
int x = tft.width() / 2;
int y = tft.height() / 2;

float moistValue = 0; 
int refValue = 0;
int uvValue = 0;

float outputVoltage = 0;
float uvIntensity = 0;

// WiFi set up

char ssid[] = "dunder_mifflin";    // your network SSID (name) FIXME
char pass[] = "thatswhatshesaid69"; // your network password (use for WPA, or use as key for WEP) FIXME


// Name of the server we want to connect to
const char kHostname[] = "34.238.220.95";  // FIXME
const char* serverName = "http://34.238.220.95:5000";
// Path to download (this is the bit after the hostname in the URL
// that you want to download
std::string kPath = "/?temp="; // FIXME

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 5 seconds (5000)
unsigned long timerDelay = 5000;

const int kport = 5000;

// Number of milliseconds to wait without receiving any data before we give up
const int kNetworkTimeout = 30*1000;
// Number of milliseconds to wait if no data is available before trying again
const int kNetworkDelay = 1000;


void setup() { 
  Serial.begin(9600);
  delay(1000);
  //WiFi.mode(WIFI_STA);
  Serial.println("scan start");

  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0) {
      Serial.println("no networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
      delay(10);
    }
  }

  Serial.println();
  Serial.println();
  Serial.println(WiFi.macAddress());
  Serial.println("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
      Serial.println(WiFi.status());
      delay(500);
      Serial.print("Connecting....");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("MAC address: ");
  Serial.println(WiFi.macAddress());

  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);  // Adding a black background colour erases previous text automatically

  pinMode(UVIN, INPUT);
} 


void loop() { 
  // Soil Moisture
  moistValue = soilMoistureSensor();
  Serial.println();
  Serial.print("\n");
  Serial.print("Soil Moisture Sensor: ");
  Serial.println(moistValue);
  // UV Sensor
  uvValue = uvSensor();
  Serial.print("UV Sensor: ");
  Serial.println(uvValue);
  // Print to ESP32
  display();
  delay(30);

  //int err =0;
  
  WiFiClient client;
  HTTPClient http;
  std::ostringstream ss;
  std::ostringstream uu;
  ss << moistValue;
  uu << uvValue;
  std::string s(ss.str());
  std::string uv(uu.str());
  kPath = "/?Soil_Moisture=" + s + "&UV_Sensor=" + uv;
  const char* path = kPath.c_str();
  http.begin(client, serverName);

  // Specify content-type header
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  // Data to send with HTTP POST
  //String httpRequestData = "&Soil_Moisture=" + s + "&UV_Sensor=" + uv;           
  // Send HTTP POST request
  int httpResponseCode = http.POST(path);
     
  Serial.print("HTTP Response code: ");
  Serial.println(httpResponseCode);
        
  // Free resources
  //http.end();
} 


int soilMoistureSensor(){
  // soil moisture
  for (int i = 0; i <= 100; i++) 
  { 
    moistValue = moistValue + analogRead(MoistureSensor); 
    delay(1); 
  } 
  moistValue = moistValue/100.0; // for stability
  return moistValue;
}


int uvSensor(){

  uvValue = analogRead(UVIN);
  refValue = analogRead(REF_3v3);

  outputVoltage = 3.3 / refValue * uvValue;
  uvIntensity = (outputVoltage - 0.99) * (15) / (2.8 - 0.99);

  

  Serial.print("UV Intensity: ");
  Serial.println(uvIntensity);

  return uvValue;
}

void display(){
  tft.setTextDatum(TC_DATUM); // Centre text on x,y
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);  // Adding a black background colour erases previous text automatically
  tft.setCursor(x, y, 2);

  tft.drawString("Soil Moisture Sensor:", x, y-50, 4);
  //tft.drawFloat(moistValue, 0, x, y-25, 4);

  tft.drawString("UV:", x, y+25, 4);
  //tft.drawFloat(uvValue, 0, x, y+50, 4);
  tft.drawFloat(0, 0, x, y, 4);
  delay(500);
}

