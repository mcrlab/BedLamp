#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "fauxmoESP.h"
#include <FastLED.h>
#include <DNSServer.h>
#include <WiFiManager.h> 
#include <EEPROM.h>

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
fauxmoESP fauxmo;
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

const unsigned long CONNECT_TIMEOUT = 10; // Wait 10 Seconds  to connect to the real AP before trying to boot the local AP
const unsigned long AP_TIMEOUT = 20; // Wait 20 Seconds in the config portal before trying again the original WiFi creds

/********************** Begin EEPROM Section *****************/
#define EEPROM_SALT 12664
typedef struct
{
  int   salt = EEPROM_SALT;
  char light_name[20]  = "";
}

WMSettings;
WMSettings lamp;

void eeprom_read()
{
  EEPROM.begin(512);
  EEPROM.get(0, lamp);
  EEPROM.end();
}


void eeprom_saveconfig()
{
  EEPROM.begin(512);
  EEPROM.put(0, lamp);
  EEPROM.commit();
  EEPROM.end();
}

/*********************************************************************************/

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
}

void setup() {
    Serial.begin(SERIAL_BAUDRATE);
    Serial.println();
    Serial.println();

    eeprom_read();


    WiFiManagerParameter custom_light_name("light name", "light name", lamp.light_name, 20);

    WiFiManager wifiManager;
    wifiManager.setSaveConfigCallback(saveConfigCallback);
    wifiManager.addParameter(&custom_light_name);

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

    strcpy(lamp.light_name, custom_light_name.getValue());
    eeprom_saveconfig();

    Serial.println(lamp.light_name);

    FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS);

    //wifiSetup();

    fauxmo.createServer(true);
    fauxmo.setPort(80);
    fauxmo.enable(true);

    fauxmo.addDevice(lamp.light_name);

    fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool state, unsigned char value) {
                
        Serial.printf("[MAIN] Device #%d (%s) state: %s value: %d\n", device_id, device_name, state ? "ON" : "OFF", value);

        if (strcmp(device_name, lamp.light_name)==0) {
            FastLED.setBrightness(value);
        }
        
    });

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
    fauxmo.handle();

    //fill_rainbow( leds, NUM_LEDS, gHue, 7);                  
    sinelon();
    FastLED.show();  
    FastLED.delay(1000/FRAMES_PER_SECOND); 

    EVERY_N_MILLISECONDS( 20 ) { gHue++; }
}
