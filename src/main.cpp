#include <Arduino.h>
#ifdef ESP32
    #include <WiFi.h>
#else
    #include <ESP8266WiFi.h>
#endif
#include "fauxmoESP.h"
#include <FastLED.h>
#include "credentials.h"

// -----------------------------------------------------------------------------

#define SERIAL_BAUDRATE     115200
#define ID_NAME              "Bed Lamp"
#define DATA_PIN            5
#define LED_TYPE            WS2811
#define COLOR_ORDER         GRB
#define NUM_LEDS            7
#define BRIGHTNESS          255
#define FRAMES_PER_SECOND   120

CRGB leds[NUM_LEDS];
fauxmoESP fauxmo;
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Wifi
// -----------------------------------------------------------------------------

void wifiSetup() {

    // Set WIFI module to STA mode
    WiFi.mode(WIFI_STA);

    // Connect
    Serial.printf("[WIFI] Connecting to %s ", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    // Wait
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(100);
    }
    Serial.println();

    Serial.printf("[WIFI] STATION Mode, SSID: %s, IP address: %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());

}


void setup() {
    Serial.begin(SERIAL_BAUDRATE);
    Serial.println();
    Serial.println();

    FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS);

    wifiSetup();

    fauxmo.createServer(true);
    fauxmo.setPort(80);
    fauxmo.enable(true);

    fauxmo.addDevice(ID_NAME);

    fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool state, unsigned char value) {
                
        Serial.printf("[MAIN] Device #%d (%s) state: %s value: %d\n", device_id, device_name, state ? "ON" : "OFF", value);

        if (strcmp(device_name, ID_NAME)==0) {
            FastLED.setBrightness(value);
        } 
        
    });

}


void loop() {
    fauxmo.handle();

    fill_rainbow( leds, NUM_LEDS, gHue, 7);                  
    FastLED.show();  
    FastLED.delay(1000/FRAMES_PER_SECOND); 

    EVERY_N_MILLISECONDS( 20 ) { gHue++; }
}
