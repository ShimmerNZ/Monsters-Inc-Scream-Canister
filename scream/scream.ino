#include <Adafruit_NeoPixel.h>
#include "DFRobotDFPlayerMini.h"
#include <SoftwareSerial.h> 

// Define the microphone pin and NeoPixel settings

#define MICROPHONE_PIN A0
#define NEOPIXEL_PIN 6
#define NUM_LEDS 9
#define BRIGHTNESS 50
#define MIN_LEVEL 0
#define MAX_LEVEL 300
#define Aud_Rx_PIN 11
#define Aud_Tx_PIN 10
unsigned int backgroundNoiseLevel = 0;
unsigned int maxSoundLevel = 0;

// Create a new NeoPixel object
Adafruit_NeoPixel pixels(NUM_LEDS, NEOPIXEL_PIN, NEO_GRBW + NEO_KHZ800);

// Audio Software
SoftwareSerial UartSound(Aud_Rx_PIN, Aud_Tx_PIN);
DFRobotDFPlayerMini myDFPlayer;
const byte volMp3 = 25;

bool SetupAudio()
{
  UartSound.begin(9600);
  return myDFPlayer.begin(UartSound);
}

// Define a function to initialize the NeoPixels
void initializeNeoPixels() {
  pixels.begin();// Initialize the NeoPixel library
  pixels.setBrightness(BRIGHTNESS);
  pixels.show();   // Turn off all the pixels
}

// Define a function to flash the NeoPixels green for 2 seconds, then red for 2 seconds, then turn them off
void flashNeoPixels() {
  delay(900);
  for (int d =0; d<255; d++) {
    for (int i = 0; i < NUM_LEDS; i++) {
      pixels.setPixelColor(i, 0, d, 0);  // Green
      pixels.show();
    }
  }
  for (int d =255; d>0; d--) {
    for (int i = 0; i < NUM_LEDS; i++) {
      pixels.setPixelColor(i, 0, d, 0);  // Green
      pixels.show();
    }
  }
}

// Define a function to make the NeoPixels sparkle randomly for 3 seconds
void sparkleNeoPixels() {
  int num_sparkles = 20;  // Number of sparkles to create
  int sparkle_delay = 50; // Delay between creating each sparkle in milliseconds
  for (int i = 0; i < num_sparkles; i++) {
    int index = random(NUM_LEDS);  // Choose a random pixel to light up
    int lastindex;
    int w = random(256);            // Choose a random white value
    pixels.setPixelColor(index, 0, 0, 0, w); // Set the pixel to the random color
    pixels.setPixelColor(lastindex, 0, 0, 0, 0); // Set the pixel to the random color
    lastindex=index;
    pixels.show();
    delay(sparkle_delay);
  }

  // Turn off all the pixels after the sparkles are done
  for (int i = 0; i < NUM_LEDS; i++) {
    pixels.setPixelColor(i, 0, 0, 0);
  }
  pixels.show();
}



void setup() {
  initializeNeoPixels();  // Initialize the NeoPixels
  // Serial port
  Serial.begin(115200);
  while (!Serial);

  // Determine background noise level by taking multiple samples over 3 seconds
  for (int i = 0; i < 3000; i++) {
    backgroundNoiseLevel += analogRead(MICROPHONE_PIN);
    delay(1); // 1 ms delay between samples
  }
  backgroundNoiseLevel /= 3000; // Calculate the average background noise level
  Serial.print("Background Noise Level: ");
  Serial.println(backgroundNoiseLevel);
  


  Serial.println("Initialiting..."); 
  if (SetupAudio()) {
    Serial.println("Audio... Ok");   
  }else{
    Serial.println("Audio... Error");   
  }
  myDFPlayer.setTimeOut(800);   //Set serial communictaion time out 500ms
  myDFPlayer.volume(volMp3);    //Set volume value. From 0 to 30
  myDFPlayer.EQ(DFPLAYER_EQ_NORMAL);
  myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);
  myDFPlayer.disableLoop();

  myDFPlayer.play(10); 
  flashNeoPixels();       // Flash the NeoPixels green, red, and then turn off
  randomSeed(analogRead(0)); // Initialize the random number generator
  sparkleNeoPixels();

  // Turn off all the pixels to start
  for (int i = 0; i < NUM_LEDS; i++) {
    pixels.setPixelColor(i, 0, 20, 0);
    pixels.show();
    delay(10);
  }
}



void loop() {
  // Read the voltage from the microphone and map it to a level from 0 to 9
  int soundLevel = analogRead(MICROPHONE_PIN);

  // Check if the sound level is above the background noise level
  if (soundLevel > backgroundNoiseLevel+50) {
    // Sound level is above background noise, sample over 1 seconds
    unsigned int currentMax = 0;
    for (int i = 0; i < 1000; i++) {
      int sample = analogRead(MICROPHONE_PIN);
      Serial.println(sample);
      if (sample > currentMax) {
        currentMax = sample;
      }
      delay(1); // 1 ms delay between samples
    }
    
    // Update maxSoundLevel if a new maximum is found
    if (currentMax > maxSoundLevel) {
      maxSoundLevel = currentMax;
    }
    
  }
  
  // You can add more code here for other tasks to be performed in the main loop
  
  // Print maxSoundLevel and averageSoundLevel periodically
  Serial.print("Max Sound Level: ");
  Serial.println(maxSoundLevel);

  int level = map(maxSoundLevel, backgroundNoiseLevel, MAX_LEVEL, 0, NUM_LEDS);
  if (level > 9) { level=9;}
  if (level >0) {
    myDFPlayer.play(level);
    Serial.println(level);
    delay(750);
    // Update the NeoPixels based on the level
    for (int i = 0; i < NUM_LEDS; i++) {
      if (i < level) {
      pixels.setPixelColor(i, 255, 0, 0); // Red
      pixels.show();
      delay(173);
      } else {
      pixels.setPixelColor(i, 0, 20, 0);   // Off
      pixels.show();
      }
    }
    // Delay for a short period to prevent constant updates
    delay(3000);
    for (int i = NUM_LEDS; i > -1; i--) {
      pixels.setPixelColor(i, 0, 20, 0);
      pixels.show();
      delay(50);
    }
  maxSoundLevel=0;
  }
}