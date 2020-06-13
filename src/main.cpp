#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <FastLED.h>
#include <DNSServer.h>
#include <WiFiManager.h> 

// -----------------------------------------------------------------------------

#define SERIAL_BAUDRATE     115200
#define ID_NAME             "lantern"
#define DATA_PIN            5
#define LED_TYPE            WS2811
#define COLOR_ORDER         GRB
#define NUM_LEDS            60
#define BRIGHTNESS          128
#define FRAMES_PER_SECOND   120

char light_name[40];
CRGB leds[NUM_LEDS];

uint8_t gHue = 0; // rotating "base color" used by many of the patterns

const unsigned long CONNECT_TIMEOUT = 10; // Wait 10 Seconds  to connect to the real AP before trying to boot the local AP
const unsigned long AP_TIMEOUT = 20; // Wait 20 Seconds in the config portal before trying again the original WiFi creds

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

    //fill_rainbow( leds, NUM_LEDS, gHue, 7);                  
    sinelon();
    FastLED.show();  
    FastLED.delay(1000/FRAMES_PER_SECOND); 

    EVERY_N_MILLISECONDS( 20 ) { gHue++; }
}
