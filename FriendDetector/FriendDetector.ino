/*      ______     _                ______       __            __            
 *     / ____/____(_)__  ____  ____/ / __ \___  / /____  _____/ /_____  _____
 *    / /_  / ___/ / _ \/ __ \/ __  / / / / _ \/ __/ _ \/ ___/ __/ __ \/ ___/
 *   / __/ / /  / /  __/ / / / /_/ / /_/ /  __/ /_/  __/ /__/ /_/ /_/ / /    
 *  /_/   /_/  /_/\___/_/ /_/\__,_/_____/\___/\__/\___/\___/\__/\____/_/  
 * 
 *  A a program to detect known devices, written for the WiFi Nugget
 *  github.com/HakCat/FriendDetector
 * 
 *  By Alex Lynd | alexlynd.com
 *  
 */

#define fren_num 2 // number of known devices

#include <ESP8266WiFi.h>       
#include <Adafruit_NeoPixel.h>
#include <Wire.h>
  #include "SH1106Wire.h"
#include "OLEDDisplayUi.h"

#include "nuggs.h" // Nugget Face bitmap files

Adafruit_NeoPixel pixels {1, D8, NEO_GRB + NEO_KHZ800 }; // initialize 1 NeoPixel on D8

SH1106Wire display(0x3c, D2, D1); // initialize OLED on I2C pins
OLEDDisplayUi ui     ( &display );

extern "C" {
#include "user_interface.h"
}

int detected_fren = -1;

const short channels[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13}; // Max: US 11, EU 13, JAP 14

// known device mac address list
uint8_t frens[fren_num][6] = {
   {0xf6, 0xcf, 0xa2, 0xd4, 0x4c, 0xea}
  ,{0x4a, 0x3f, 0xda, 0x45, 0x7a, 0x0f}
};

// known device neopixel colors (RGB)
uint8_t colors[fren_num][3] = {
  {255,69,0}, // orange
  {128,0,128}, // purple
};

// compare byte array mac addresses
bool maccmp(uint8_t *mac1, uint8_t *mac2) {
  for (int i=0; i < 6; i++) {
    if (mac1[i] != mac2[i]) {
      return false;
    }
  }
  return true;
}

// list of available nugget faces
const uint8_t* nugget_faces[]= {
  happy_halloween_nugg,
  angy_halloween_nugg
};


int ch_index { 0 };               
int packet_rate { 0 };            
int attack_counter { 0 };         
unsigned long update_time { 0 };  
unsigned long ch_time { 0 };

void sniffer(uint8_t *buf, uint16_t len) {
  if (!buf || len < 28) return;
  uint8_t* addr_a = &buf[16]; uint8_t* addr_b = &buf[22];
  
  for (int i=0; i<fren_num; i++) {
    if (maccmp(addr_a,frens[i]) || maccmp(addr_b,frens[i])) {
      detected_fren = i;    
      break; // exit when a known device is detected
    }
  }
}

void setup() {
  Serial.begin(115200);           // initialize serial communication
  pixels.begin(); pixels.clear(); // initialize NeoPixel
  ui.setTargetFPS(60); ui.init(); // initialize OLED screen

  // initalize WiFi card for scanning
  WiFi.disconnect();
  wifi_set_opmode(STATION_MODE);       
  wifi_set_promiscuous_rx_cb(sniffer);
  wifi_set_channel(1);        
  wifi_promiscuous_enable(true);       

  Serial.println();
  Serial.println("   ____    _             _____      __          __");   
  Serial.println("  / __/___(_)__ ___  ___/ / _ \\___ / /____ ____/ /____  ____");
  Serial.println(" / _// __/ / -_) _ \\/ _  / // / -_) __/ -_) __/ __/ _ \\/ __/");
  Serial.println("/_/ /_/ /_/\\__/_//_/\\_,_/____/\\__/\\__/\\__/\\__/\\__/\\___/_/   ");
                                                            
  Serial.println("\ngithub.com/HakCat-Tech/FriendDetector");
  Serial.println("A WiFi Nugget sketch by Alex Lynd");

  display.clear();
  display.flipScreenVertically();
  drawDefaultNugg();

}

void loop() {
  unsigned long current_time = millis();
  
  if (detected_fren >=0) {
    Serial.print("Detected friend number "); Serial.print(detected_fren);
    Serial.println("!");
    drawNugg(detected_fren);
    detected_fren = -1;
    update_time = current_time; 
  }
  else if (current_time - update_time > 2000) { // cooldown time 2 seconds
    drawDefaultNugg();
    update_time = current_time;    
  }

  // Channel hopping
  if (sizeof(channels) > 1 && current_time - ch_time >= 100) {
    ch_time = current_time; // Update time variable
    ch_index = (ch_index + 1) % (sizeof(channels) / sizeof(channels[0]));
    short ch = channels[ch_index];
    wifi_set_channel(ch);
  }
}

void drawNugg(int nugget_face) {
  display.clear();
  display.drawXbm(0, 0, alive_nugg_width, alive_nugg_height, nugget_faces[nugget_face]);
  display.display();
  pixels.setPixelColor(0, pixels.Color(colors[nugget_face][0],colors[nugget_face][1],colors[nugget_face][2])); 
  pixels.show();
}

void drawDefaultNugg() {
  pixels.setPixelColor(0, pixels.Color(150, 0, 0)); pixels.show(); // red
  display.clear();
  display.drawXbm(0, 0, alive_nugg_width, alive_nugg_height, dead_nugg);
  display.display();
}
