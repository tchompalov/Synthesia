/******************************************************************************
  Synthesia - Uses a Sparkfun Shield and Programmable LED Strip.
  LED Strip tutorial: https://www.temposlighting.com/guides/how-to-add-custom-leds-to-any-project
  Sparkfun Shield: https://learn.sparkfun.com/tutorials/spectrum-shield-hookup-guide-v2

  Step 1: Power the LED Strip w/ a 5V 3A power supply. I've noticed it averages 18W peak.
  Step 2: Tap the ground wire from the LED strip's program wire to the Arduino Ground
  Step 3: Tap the program pin from the LED strip to Digital Pin 11 on the Arduino.
  Step 4: Attach the Sparkfun sheld to the Arduino. I used male headers just for the VIN and Analog pins.
  Step 5: Upload this sketch. Import the FastLED library.
  Step 6: Plug in some tunes via the 3.5mm jack into the Shield. 
  (Use another 3.5mm jack to output the tunes to a speaker, from the shield's output port.
  Step 7: Enjoy the lights dance.
*********************************************************************************/

// LED Library
#include <FastLED.h>

// SOUND - Spectrum Shield pin connections
#define STROBE 4
#define RESET 5
#define DC_One A0
#define DC_Two A1

// SOUND spectrum variables
int freq_amp;
int Frequencies_One[7];
int Frequencies_Two[7];
int i;

// LED variables
int TOTAL_LEDS = 150;
CRGB leds[150];        // Total number of LEDs on your strip
#define LED_PIN 11     // Output from the Arduino to the LED Strip

// This Color class is used to make the rest of the code cleaner & more reusable.
// I
class Color {
  private:
    int red;
    int green;
    int blue;
    float brightnessPct; // If a color is too bright, dial it back

    // Hold a memory of what the value was before: simulates how analog tubes fade; gives a warm feeling.
    int redAfterGlow;
    int greenAfterGlow;
    int blueAfterGlow;
    int fadeRate = 1; // Gets subtracted from the Glow Values; 1 = slowest fade; 1000 = immediate fade.
    
  public:
    Color(int r, int g, int b, float dimmerSwitch) {
      this->red = r;
      this->green = g;
      this->blue = b;
      this->brightnessPct = dimmerSwitch;
 
      this->redAfterGlow = 0;
      this->greenAfterGlow = 0;
      this->blueAfterGlow = 0;
    }

    int getRed() {
      return (int) (red * brightnessPct);
    }
    int getGreen() {
      return (int) (green * brightnessPct);
    }
    int getBlue() {
      return (int) (blue * brightnessPct);
    }

    // Frequency Value = 0 ~ 1000
    // ColorMax = 0, 150, 255 Typical
    int getVibrance(int colorMax, int frequencyValue) {
      if (colorMax == 0) {
        return 0; // Optimization
      }
      float vibrancePercentage = frequencyValue / 1000.0;
      float rawVibrance = colorMax * vibrancePercentage;

      return (int) rawVibrance;
    }
    
    CRGB getColorValues(int frequencyValue) {

      /* Debug:
      Serial.print("getColorValues(");
      Serial.print( frequencyValue );
      Serial.print(") >> ");
      Serial.print("[");
      Serial.print( getRed() );
      Serial.print(" --> ");
      Serial.print( getVibrance(getRed(), frequencyValue));
      Serial.print(",  ");
      Serial.print( getGreen() );
      Serial.print(" --> ");
      Serial.print( getVibrance(getGreen(), frequencyValue));
      Serial.print(",  ");
      Serial.print( getBlue() );
      Serial.print(" --> ");
      Serial.print( getVibrance(getBlue(), frequencyValue));
      Serial.print("]  ");
      Serial.println(" ");
      */

      int redVibrance = getVibrance(getRed(), frequencyValue);
      if (redVibrance > redAfterGlow) {
        redAfterGlow = redVibrance;
      } 
      else {
        redVibrance = redAfterGlow;
        redAfterGlow = redAfterGlow - fadeRate;
      }

      int greenVibrance = getVibrance(getGreen(), frequencyValue);
      if (greenVibrance > greenAfterGlow) {
        greenAfterGlow = greenVibrance;
      } 
      else {
        greenVibrance = greenAfterGlow;
        greenAfterGlow = greenAfterGlow - fadeRate;
      }

      int blueVibrance = getVibrance(getBlue(), frequencyValue);
      if (blueVibrance > blueAfterGlow) {
        blueAfterGlow = blueVibrance;
      } 
      else {
        blueVibrance = blueAfterGlow;
        blueAfterGlow = blueAfterGlow - fadeRate;
      }
      
      return CRGB( 
        redVibrance,
        greenVibrance,
        blueVibrance
      );
    }
}; // Remember Semicolon at the end of definition

/********************Setup Loop*************************/
void setup() {
  // Set spectrum Shield pin configurations
  pinMode(STROBE, OUTPUT);
  pinMode(RESET, OUTPUT);
  pinMode(DC_One, INPUT);
  pinMode(DC_Two, INPUT);

  // Initialize Spectrum Analyzers
  digitalWrite(STROBE, LOW);
  digitalWrite(RESET, LOW);
  delay(5);

  Serial.begin(9600); // For Serial debugging

  // LED:
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, TOTAL_LEDS);
}


/**************************Main Function Loop*****************************/
void loop() {
  Read_Frequencies(); // Sound Spectrum Shield
  // Graph_Frequencies(); // Serial -- If you want to visualize the frequencies with your Arduino Plotter
  Beam_Frequencies(); // LED Lights
}

/*****************Print Out Band Values for Serial Plotter*****************/
void Graph_Frequencies() {

  int numberOfFrequencyBands = 7;
  int offset = 500; // 500; // Separate the plotter lines by this amount, so you can read them better
  int maxFrequencyValueAllowed = 2000; // Defensive: Discard huge values when initializing board
    
  for (i = 0; i < numberOfFrequencyBands; i++) {

    if (Frequencies_One[i] < maxFrequencyValueAllowed) {
      Serial.print(Frequencies_One[i] + i*offset);
      Serial.print(" ");      
//    Serial.print(Frequencies_Two[i]); // VT: These are just unnecessary.
//    Serial.print(" ");
//    Serial.print( (Frequencies_One[i] + Frequencies_Two[i]) / 2 );
//    Serial.print("    ");
    }
    else {
      Serial.print(maxFrequencyValueAllowed);
      Serial.print(" ");      
    }
  }
  Serial.println();
}

/*******************Pull frquencies from Spectrum Shield********************/
void Read_Frequencies() {
  digitalWrite(RESET, HIGH);
  delayMicroseconds(200);
  digitalWrite(RESET, LOW);
  delayMicroseconds(200);

  //Read frequencies for each band
  for (freq_amp = 0; freq_amp < 7; freq_amp++)
  {
    digitalWrite(STROBE, HIGH);
    delayMicroseconds(50);
    digitalWrite(STROBE, LOW);
    delayMicroseconds(50);

    Frequencies_One[freq_amp] = analogRead(DC_One);
    Frequencies_Two[freq_amp] = analogRead(DC_Two);
  }
}


/* Frequencies come in at a minimum of 50, so clip that from the frequency value 
 * Also amounts higher than 1000 are junk, so discard them (probably from clipping into the jack)
*/
int deNoiseFrequency(int input) {
  int minThreshold = 100; // Don't display LED below this (50~75)
  if (input > minThreshold && input <= 1000) {
    return input  - minThreshold; // Optimization: - minThreshold to set LEDs to dark
  } 
  else if (input > 1000) {
    return 1000;
  }
  else {
    return 0;
  }
}

Color sangria(255, 0, 0, 1.0);
Color grapefruit(255, 50, 0, 1.0);
Color tangerine(255, 100, 0, 0.95);
Color orange(255, 150, 0, 0.9);
Color fanta(255, 200, 0, 0.85);
Color sunfire(255, 255, 0, 0.8);
Color hotpepper(200, 255, 0, 0.9);

Color lime(150, 255, 0, 1.0);
Color turquoise(0, 255, 150, 1.0);
Color cyan(0, 255, 255, 0.8);
Color ocean(0, 150, 255, 1.0);
Color bubblegum(150, 0, 255, 1.0);

/* Mode 2: White
Color w1(255, 255, 255, 1.0);
Color w2(255, 255, 255, 0.9);
Color w3(255, 255, 255, 0.8);
Color w4(255, 255, 255, 0.7);
Color w5(255, 255, 255, 0.8);
Color w6(255, 255, 255, 0.9);
Color w7(255, 255, 255, 1.0);
Color colorSequence[7] = {w1, w2, w3, w4, w5, w6, w7};
*/

Color colorSequence[7] = {grapefruit, sunfire, lime, turquoise, cyan, ocean, bubblegum}; // Modern Rainbow
// Color colorSequence[7] = {sangria, grapefruit, tangerine, orange, fanta, sunfire, hotpepper}; // Hot

// int lightsPerSequence[7] = {25, 20, 20, 20, 20, 20, 25};
int lightSequence[7] = {25, 45, 65, 85, 105, 125, 150}; 


void setColors(int ledID, int sequenceID) {
  int cleanFrequencyValue = deNoiseFrequency(Frequencies_One[sequenceID]);
  leds[ledID] = colorSequence[sequenceID].getColorValues( cleanFrequencyValue );
}

void Beam_Frequencies() {

  int frequencyValue = 0;

  /* Version 1: FOR EACH LED - Color Bands
  for(int count = 0; count < 150; count++) {
    if (count < lightSequence[0]) {
      setColors(count, 0);
    }
    else if (count < lightSequence[1]) {
      setColors(count, 1);
    }
    else if (count < lightSequence[2]) {
      setColors(count, 2);
    }
    else if (count < lightSequence[3]) {
      setColors(count, 3);
    }
    else if (count < lightSequence[4]) {
      setColors(count, 4);
    }
    else if (count < lightSequence[5]) {
      setColors(count, 5);
    }
    else {
      setColors(count, 6);
    }
  }
  /**/

  /* Version 2: FOR EACH LED - Color Series (Even) */
  for(int count = 0; count < 150; count++) {
    int factor = 10;
    if (count % factor == 0 || count % factor == 1 || count % factor == 2) {
      setColors(count, 0);
    }
    else if (count % factor == 3) {
      setColors(count, 1);
    }
    else if (count % factor == 4) {
      setColors(count, 2);
    }
    else if (count % factor == 5) {
      setColors(count, 3);
    }
    else if (count % factor == 6) {
      setColors(count, 4);
    }
    else if (count % factor == 7) {
      setColors(count, 5);
    }
    else {
      setColors(count, 6);
    }
  }
  /**/

  /* Version 3: FOR EACH LED - Color Series (Bass-Heavy)
  for(int count = 0; count < 150; count++) {
    int factor = 8; // 7 or greater
    if (count % factor == 0 || count % factor == 6) {
      setColors(count, 0);
    }
    else if (count % factor == 1) {
      setColors(count, 1);
    }
    else if (count % factor == 2) {
      setColors(count, 2);
    }
    else if (count % factor == 3) {
      setColors(count, 3);
    }
    else if (count % factor == 4) {
      setColors(count, 4);
    }
    else if (count % factor == 5) {
      setColors(count, 5);
    }
    else {
      setColors(count, 6);
    }
  }
  /**/
  
  FastLED.show();
  // Serial.println();
}
