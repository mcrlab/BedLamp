#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <FastLED.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
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
WiFiServer server(80);
String header;
String outputState = "off";


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


 WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // turns the GPIOs on and off
            if (header.indexOf("GET /output/on") >= 0) {
              Serial.println("Output on");
              outputState = "on";
            } else if (header.indexOf("GET /output/off") >= 0) {
              Serial.println("Output off");
              outputState = "off";
            }
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>ESP8266 Web Server</h1>");
            
            // Display current state, and ON/OFF buttons for the defined GPIO  
            client.println("<p>Output - State " + outputState + "</p>");
            // If the outputState is off, it displays the ON button       
            if (outputState=="off") {
              client.println("<p><a href=\"/output/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/output/off\"><button class=\"button button2\">OFF</button></a></p>");
            }                  
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }

    //fill_rainbow( leds, NUM_LEDS, gHue, 7);                  
    sinelon();
    FastLED.show();  
    FastLED.delay(1000/FRAMES_PER_SECOND); 
    EVERY_N_MILLISECONDS( 20 ) { gHue++; }
}
