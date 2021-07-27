/**
 * test for TTGO OLED Batt + Wifi board
 * 
 * Xtase - fgalliat @Jul 2021
 */

//Libraries for SPI
#include <SPI.h>

//Libraries for OLED Display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//define the pins used by the LoRa transceiver module
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26

//OLED pins
#define OLED_SDA 4
#define OLED_SCL 15 
#define OLED_RST 16

#define OLED_SDA 5
#define OLED_SCL 4 
#define OLED_RST -1

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

//packet counter
int counter = 0;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

void setup() {
  //initialize Serial Monitor
  Serial.begin(115200);

  //reset OLED display via software
  if ( OLED_RST > 0 ) {
    pinMode(OLED_RST, OUTPUT);
    digitalWrite(OLED_RST, LOW);
    delay(20);
    digitalWrite(OLED_RST, HIGH);
  }

  //initialize OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("TTGO Tester ");
  display.display();
  
  Serial.println("TTGO Test");

  //SPI pins
  // SPI.begin(SCK, MISO, MOSI, SS);
  
  Serial.println("Dummy Initializing OK!");
  display.setCursor(0,10);
  display.print("Dummy Initializing OK!");
  display.display();
  delay(2000);
}

void loop() {
   
  Serial.print("Sending packet: ");
  Serial.println(counter);

  display.clearDisplay();
  display.setCursor(0,0);
  display.println("TEST SENDER");
  display.setCursor(0,20);
  display.setTextSize(1);
  display.print("Test packet sent.");
  display.setCursor(0,30);
  display.print("Counter:");
  display.setCursor(50,30);
  display.print(counter);      
  display.display();

  counter++;
  
  delay(10000);
}