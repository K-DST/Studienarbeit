#include <Adafruit_NeoPixel.h>

#define LED_PIN    6 
#define LED_COUNT 144

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

int analogPin = 0; // MSGEQ7 OUT
int strobePin = 2; // MSGEQ7 STROBE
int resetPin = 4; // MSGEQ7 RESET
int spectrumValue[7];
 
// MSGEQ7 OUT pin produces values around 50-80
// when there is no input, so use this value to
// filter out a lot of the chaff.
int filterValue = 80;
 
void setup(){
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(150); // Set BRIGHTNESS to about 1/5 (max = 255)
  //---------------------------
  Serial.begin(1200);
  pinMode(analogPin, INPUT);
  pinMode(strobePin, OUTPUT);
  pinMode(resetPin, OUTPUT);
 
  // Set analogPin's reference voltage
  analogReference(DEFAULT); // 5V
 
  // Set startup values for pins
  digitalWrite(resetPin, LOW);
  digitalWrite(strobePin, HIGH);
}
 
void loop(){
  collectAudio();
  seperateStripIn3Pieces();
  //int low = 0;
  //if(spectrumValue[1] > 150){
  //  low = spectrumValue[1];
  //}
  //setColorForEntireStrip(0, low, 0);
  //setColorForEntireStrip(spectrumValue[1], spectrumValue[3], spectrumValue[5]);
}

void setColorForEntireStrip(int r, int g, int b){
  strip.fill(strip.Color(r, g, b));
  strip.show();
}

void seperateStripIn3Pieces(){
  int l = LED_COUNT / 3;
  int low = 0;
  int mid = 0;
  int high = 0;
  if(spectrumValue[1] > 130){
    low = spectrumValue[1];
  }
  if(spectrumValue[4] > 70){
    mid = spectrumValue[4];
  }
  if(spectrumValue[6] > 70){
    high = spectrumValue[6];
  }
  strip.fill(strip.Color(low, 0, 0), 0, l-1);
  strip.fill(strip.Color(0, mid, 0), l, 2*l-1);
  strip.fill(strip.Color(0, 0, high), l+l, 3*l-1);
  strip.show();
}

void collectAudio(){
  // Set reset pin low to enable strobe
  digitalWrite(resetPin, HIGH);
  digitalWrite(resetPin, LOW);
  // Get all 7 spectrum values from the MSGEQ7
  for (int i = 0; i < 7; i++)
  {
    digitalWrite(strobePin, LOW);
    delayMicroseconds(40); // Allow output to settle
     spectrumValue[i] = analogRead(analogPin);
    //Serial.print(spectrumValue[i]);
    // Constrain any value above 1023 or below filterValue
    spectrumValue[i] = constrain(spectrumValue[i], filterValue, 1023);
    // Remap the value to a number between 0 and 255
    spectrumValue[i] = map(spectrumValue[i], filterValue, 1023, 0, 255);
    // Remove serial stuff after debugging
    Serial.print(spectrumValue[i]);
    Serial.print(" ");
    digitalWrite(strobePin, HIGH);
   }
   Serial.println();
}
