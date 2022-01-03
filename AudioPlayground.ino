#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#ifdef ESP32
  #include <WiFi.h>
  #include "SPIFFS.h"
#else
  #include <ESP8266WiFi.h>
#endif
#include "AudioFileSourceSPIFFS.h"
#include "AudioFileSourceID3.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2SNoDAC.h"

// Set your ESP8266 build to 160MHz
// Use the "Tools->ESP8266/ESP32 Sketch Data Upload" menu to write the MP3 to SPIFFS

// Wemos pins
// https://github.com/esp8266/Arduino/blob/master/variants/d1_mini/pins_arduino.h

// Used forked copy of ESP8266Audio
// https://github.com/earlephilhower/ESP8266Audio/issues/406
// https://github.com/ChrisVeigl/ESP8266Audio

AudioGeneratorMP3 *mp3;
AudioFileSourceSPIFFS *file;
AudioOutputI2SNoDAC *out;
AudioFileSourceID3 *id3;

// LiquidCrystal_I2C lcd(0x27,20,4); // set the LCD address to 0x3F for a 16 chars and 2 line display

const int TRIGGER_PIN = D6;
int triggerState = 0;

void MDCallback(void *cbData, const char *type, bool isUnicode, const char *string)
{
  (void)cbData;
  Serial.printf("ID3 callback for: %s = '", type);

  if (isUnicode) {
    string += 2;
  }
  
  while (*string) {
    char a = *(string++);
    if (isUnicode) {
      string++;
    }
    Serial.printf("%c", a);
  }
  Serial.printf("'\n");
  Serial.flush();
}


void play(char *phrase, AudioGeneratorMP3 *mp3, AudioFileSourceID3 *id3, AudioOutputI2SNoDAC *out)
{
  Serial.printf("Playing\n");
  file = new AudioFileSourceSPIFFS(phrase);
  id3 = new AudioFileSourceID3(file);
  id3->RegisterMetadataCB(MDCallback, (void*)"ID3TAG");
  mp3->begin(file, out);
}


void setup() 
{
  Wire.begin();
  WiFi.mode(WIFI_OFF); 
  
  pinMode(TRIGGER_PIN, INPUT);

  Serial.begin(115200);
  Serial.println("SETUP - STARTED");

//  lcd.init();
//  lcd.begin(20,4);
//  lcd.backlight();

  audioLogger = &Serial;
  out = new AudioOutputI2SNoDAC();
  mp3 = new AudioGeneratorMP3();
  SPIFFS.begin();
  mp3->begin(nullptr, out);

  delay(1000);
  Serial.println("SETUP - COMPLETE");
}


void loop()
{
  if (mp3->isRunning()) {
    if (!mp3->loop()) mp3->stop();
  } else {
    triggerState = digitalRead(TRIGGER_PIN);
    if (triggerState == HIGH) {
      Serial.println("ON");
      play("/audio-phrases-6.mp3", mp3, id3, out);
    } else {
      Serial.println("OFF");
    }
    delay(200);
  }
}
