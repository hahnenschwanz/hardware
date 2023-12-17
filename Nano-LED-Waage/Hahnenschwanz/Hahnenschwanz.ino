#include <HX711_ADC.h>
#include <FastLED.h>

//pins:
const int HX711_dout = 4; //mcu > HX711 dout pin
const int HX711_sck = 5; //mcu > HX711 sck pin
const int LEDPinMain = 3; 
const int LEDPinSchlitten = 6; 

//Const
#define NUM_STRIPS 2
#define NUM_LEDS_Main 73
#define NUM_LEDS_Schlitten 5
#define UPDATES_PER_SECOND 100
#define BRIGHTNESS  64
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB

//HX711 constructor:
HX711_ADC LoadCell(HX711_dout, HX711_sck);

//FastLED constructor:
CRGB leds[NUM_STRIPS][NUM_LEDS_Main];

//global vars
unsigned long t = 0;
unsigned int LEDstatus = 0;
CRGBPalette16 PaletteMain;
CRGBPalette16 PaletteSchlitten;
TBlendType    BlendingMain;
TBlendType    BlendingSchlitten;
unsigned int animationscounter = 0;

void setup() {
  Serial.begin(9600); delay(10);

  LoadCell.begin();
  //LoadCell.setReverseOutput(); //uncomment to turn a negative output value to positive
  float calibrationValue; // calibration value (see example file "Calibration.ino")
  calibrationValue = -1286.76; // uncomment this if you want to set the calibration value in the sketch

  unsigned long stabilizingtime = 2000; // preciscion right after power-up can be improved by adding a few seconds of stabilizing time
  LoadCell.start(stabilizingtime, false);
  if (LoadCell.getTareTimeoutFlag()) {
    Serial.println("LC:err");
    while (1);
  }
  else {
    LoadCell.setCalFactor(calibrationValue); // set calibration value (float)
  }

  FastLED.addLeds<WS2811, LEDPinMain,     GRB>(leds[0], NUM_LEDS_Main).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2811, LEDPinSchlitten,GRB>(leds[1], NUM_LEDS_Schlitten).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  fill_solid( PaletteMain,      16, CRGB::Black);
  fill_solid( PaletteSchlitten, 16, CRGB::Black);
  LEDstatus = 2;

}

void loop() {

  SerialRead();
  LEDs();
  Loadcell();
}

void SerialRead(){

   if (Serial.available() > 0) {
    String SerialInstr = Serial.readStringUntil('\n');  //read until timeout
    //SerialInstr.trim(); // remove any \r \n whitespace at the end of the String
      if (SerialInstr == "LED:err")
        LEDstatus = 1;
      else if (SerialInstr == "LED:idl") //Idle
        LEDstatus = 2;
      else if (SerialInstr == "LED:run") //Cocktail in Arbeit
        LEDstatus = 3;
      else if (SerialInstr == "LED:fin") //Cocktail feritg
        LEDstatus = 4;
      else if (SerialInstr == "LED:ser") //service
        LEDstatus = 5;
      else if (SerialInstr == "LC:tare")
       Loadcell_tare();
  }

}

void Loadcell(){
  static boolean newDataReady = 0;
  const int serialPrintInterval = 800; //increase value to slow down serial print activity

  // check for new data/start next conversion:
  if (LoadCell.update()) newDataReady = true;

  // get smoothed value from the dataset:
  if (newDataReady) {
    if (millis() > t + serialPrintInterval) {
      float i = LoadCell.getData();
      Serial.print("LC:");
      Serial.println(i);
      newDataReady = 0;
      t = millis();
    }
  }
}
void Loadcell_tare(){
  LoadCell.tareNoDelay();
}


void LEDs(){

  if (LEDstatus == 0) 
    LEDanimation_error();
  if (LEDstatus == 1) 
    LEDanimation_error();
  if (LEDstatus == 2) 
    LEDanimation_idle();
  if (LEDstatus == 3) 
    LEDanimation_run();
  if (LEDstatus == 4) 
    LEDanimation_fin();
  if (LEDstatus == 5) 
    LEDanimation_ser();
    

/*
  uint8_t brightness = 255;

  for(int i = 0; i < NUM_LEDS_Main; i++) {
    leds[0][i] = ColorFromPalette(PaletteMain, colorIndex, brightness, BlendingMain);;
  }
  for(int i = 0; i < NUM_LEDS_Schlitten; i++) {
    leds[1][i] = ColorFromPalette(PaletteSchlitten, colorIndex, brightness, BlendingSchlitten);;
  }
*/
  
  


  FastLED.show();
  FastLED.delay(1000 / UPDATES_PER_SECOND);

}

void FillLEDMainsFromPaletteColors( uint8_t colorIndex)
{
   
    for( int i = 0; i < (NUM_LEDS_Main); i++) {
        leds[0][i] = ColorFromPalette( PaletteMain, colorIndex, 255, BlendingMain);
        colorIndex += 1;
    }
    for(int i = 0; i < NUM_LEDS_Schlitten; i++) {
      leds[1][i] = leds[0][i+2];
    }
}


void FillLEDSchlittensFromPaletteColors( uint8_t colorIndex)
{
   
    for( int i = 0; i < (NUM_LEDS_Schlitten); i++) {
        leds[1][i] = ColorFromPalette( PaletteSchlitten, colorIndex, 255, BlendingSchlitten);
        colorIndex += 1;
    }
}

void LEDanimation_error(){

  if (animationscounter < 50){
    fill_solid( PaletteMain, 16, CRGB::Red);
    fill_solid( PaletteSchlitten, 16, CRGB::Black);
  }
  else{
    fill_solid( PaletteMain, 16, CRGB::Black);
    fill_solid( PaletteSchlitten, 16, CRGB::Red);
  }
  animationscounter = animationscounter + 1;

  if (animationscounter > 100) 
    animationscounter = 0;

  for( int i = 0; i < (NUM_LEDS_Main); i++) {
    leds[0][i] = ColorFromPalette( PaletteMain, 1, 255, BlendingMain);
  }
  for(int i = 0; i < NUM_LEDS_Schlitten; i++) {
    leds[1][i] = ColorFromPalette( PaletteSchlitten, 1, 255, BlendingMain);
  }


}
void LEDanimation_idle(){

  PaletteMain = RainbowColors_p;         
  BlendingMain = LINEARBLEND;

  static uint8_t startIndex = 0;
  startIndex = startIndex + 1; /* motion speed */

  FillLEDMainsFromPaletteColors(startIndex);

  for(int i = 3; i < 7; i++) { //die vorderb LEDs bleiben weiß
    leds[0][i] = CRGB::White;
  }
}

void LEDanimation_run(){

  PaletteMain = RainbowColors_p;         
  BlendingMain = LINEARBLEND;

  static uint8_t startIndex = 0;
  startIndex = startIndex - 4; /* motion speed */

  FillLEDMainsFromPaletteColors(startIndex);

  //PaletteSchlitten = RainbowStripeColors_p;
  fill_solid( PaletteSchlitten, 16, CRGB::Yellow);
  FillLEDSchlittensFromPaletteColors(1);
}

void LEDanimation_fin(){

  fill_solid( PaletteMain, 16, CRGB::Green);
   // and set every fourth one to white.
  PaletteMain[0] = CRGB::Grey;
  PaletteMain[4] = CRGB::Grey;
  PaletteMain[8] = CRGB::Grey;
  PaletteMain[12] = CRGB::Grey;

  fill_solid( PaletteSchlitten, 16, CRGB::Green);
   // and set every fourth one to white.
  PaletteSchlitten[0] = CRGB::Grey;
  PaletteSchlitten[4] = CRGB::Grey;
  PaletteSchlitten[8] = CRGB::Grey;
  PaletteSchlitten[12] = CRGB::Grey;

  BlendingMain = NOBLEND;
  BlendingSchlitten = NOBLEND;

  static uint8_t startIndex = 0;
  startIndex = startIndex + 1; /* motion speed */

  FillLEDMainsFromPaletteColors(startIndex);
  FillLEDSchlittensFromPaletteColors(startIndex);

  for(int i = 0; i < 15; i++) { //die vorderb LEDs bleiben grün
    leds[0][i] = CRGB::Green;
  }
/*
  for(int i = 0; i < NUM_LEDS_Schlitten; i++) {
    leds[1][i] = ColorFromPalette( PaletteSchlitten, 1, 255, BlendingMain);
  }
*/
}
void LEDanimation_ser(){
  fill_solid( PaletteMain, 16, CRGB::White);


 if (animationscounter < 50){
    fill_solid( PaletteSchlitten, 16, CRGB::White);
  }
  else{
    fill_solid( PaletteSchlitten, 16, CRGB::Red);
  }
  animationscounter = animationscounter + 2;

  if (animationscounter > 100) 
    animationscounter = 0;

  for( int i = 0; i < (NUM_LEDS_Main); i++) {
    leds[0][i] = ColorFromPalette( PaletteMain, 1, 255, BlendingMain);
  }
  for(int i = 0; i < NUM_LEDS_Schlitten; i++) {
    leds[1][i] = ColorFromPalette( PaletteSchlitten, 1, 255, BlendingMain);
  }

}



