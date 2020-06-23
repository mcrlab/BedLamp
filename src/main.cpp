#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <FastLED.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266mDNS.h>
#include <WiFiManager.h> 
#include <ArduinoJson.h>

// -----------------------------------------------------------------------------

#define SERIAL_BAUDRATE     115200
#define ID_NAME             "lantern"
#define DATA_PIN            5
#define LED_TYPE            WS2811
#define COLOR_ORDER         GRB
#define NUM_LEDS            60
#define BRIGHTNESS          128
#define FRAMES_PER_SECOND   120
#define WEB_APP_URL          "http://bed-lamp-iot.s3.eu-west-2.amazonaws.com/build/index.html"
#define FIRMWARE_URL        ""

char light_name[40];
CRGB leds[NUM_LEDS];

uint8_t gHue = 0; // rotating "base color" used by many of the patterns
ESP8266WebServer server(80);
String header;
boolean outputState = false;
int brightness = 255;


const unsigned long CONNECT_TIMEOUT = 10; // Wait 10 Seconds  to connect to the real AP before trying to boot the local AP
const unsigned long AP_TIMEOUT = 20; // Wait 20 Seconds in the config portal before trying again the original WiFi creds

void handleRoot();
void updateLight();
void getLight();
void handleNotFound();
void handleUpdate();


void setup() {
    Serial.begin(SERIAL_BAUDRATE);
    Serial.println();
    Serial.println();

    WiFiManager wifiManager;

   // wifiManager.resetSettings();
    wifiManager.setConnectTimeout(CONNECT_TIMEOUT);
    wifiManager.setTimeout(AP_TIMEOUT);

   // wifiManager.setCustomHeadElement("<style>html{filter: invert(100%); -webkit-filter: invert(100%);}</style>");
    if( !wifiManager.autoConnect("Lamp") ) {
        Serial.println("Failed to connect to device");
        delay(1000);
        ESP.restart();
    }

    Serial.println("Connected");

    FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS);
    if (!MDNS.begin("lamp")) {             // Start the mDNS responder for esp8266.local
      Serial.println("Error setting up MDNS responder!");
    }
    Serial.println("mDNS responder started");

    server.on("/", HTTP_GET, handleRoot);
    server.on("/led", HTTP_GET, getLight);
    server.on("/led", HTTP_POST, updateLight);
    server.on("/update", HTTP_GET, handleUpdate);
    server.onNotFound(handleNotFound);
    server.begin();

}

void handleRoot() {

  HTTPClient http;  //Declare an object of class HTTPClient
  String payload;

  http.begin(WEB_APP_URL);  //Specify request destination
  int httpCode = http.GET();                                                                  //Send the request
 
  if (httpCode > 0) { //Check the returning code
    payload = http.getString();   //Get the request response payload
    Serial.println(payload);                     //Print the response payload
    
  }
  
  http.end();  
  server.send(200, "text/html", payload);
}

void updateLight() {
  StaticJsonDocument<200> doc;
  deserializeJson(doc, server.arg("plain"));
  outputState = doc["status"];
  Serial.println(server.arg("plain"));
  server.send ( 200, "text/json", "{\"success\":true}" );
}

void getLight(){
  if (outputState) {
    server.send ( 200, "text/json", "{\"status\":true}" );
  } else {
     server.send ( 200, "text/json", "{\"status\":false}" );
  }
}
void handleUpdate(){
  Serial.println("Updating...");
  server.send(200, "text/json", "{\"success\": true}");

  t_httpUpdate_return ret = ESPhttpUpdate.update("192.168.1.85", 5000, "/firmware.bin");
  switch(ret) {
      case HTTP_UPDATE_FAILED:
          Serial.println("[update] Update failed.");
          break;
      case HTTP_UPDATE_NO_UPDATES:
          Serial.println("[update] Update no Update.");
          break;
      case HTTP_UPDATE_OK:
          Serial.println("[update] Update ok."); // may not be called since we reboot the ESP
          break;
  }
}

void handleNotFound(){
  server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}

void rainbow() 
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 13, 0, NUM_LEDS-1 );
  leds[pos] += CHSV( gHue, 255, 192);
}


void loop() {
  server.handleClient();                 
  if(!outputState){
    fill_solid( leds, NUM_LEDS, CRGB(0,0,0));
  } else {
    rainbow();
  }
  FastLED.show();  
  FastLED.delay(1000/FRAMES_PER_SECOND); 
  EVERY_N_MILLISECONDS( 20 ) { gHue++; }
}
