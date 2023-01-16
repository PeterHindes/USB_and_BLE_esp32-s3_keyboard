// Bluetooth Libraries
#include <BleKeyboard.h>
BleKeyboard bleKeyboard;

// USB Libraries
#include "USB.h"
#include "USBHIDKeyboard.h"
USBHIDKeyboard usbKeyboard;

// Led Libraries
#include <Adafruit_NeoPixel.h>
#define PIN        6
#define NUMPIXELS 16
#define DELAYVAL 500
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
// Button On Board Feather
// const int buttonPin = 0;
// int buttonState = 0;  // variable for reading the pushbutton status


// Active Program Memory
#define KEYCOUNT 64
#define ISANALOG true
#define MAXLAYERS 100
// You can save memory by specifying a precision that matches your hardware
#if ISANALOG
double keyPos[KEYCOUNT];
#endif
bool keyStates[KEYCOUNT];
// TODO this limits the number of layers the user can use
uint8_t keyMap[MAXLAYERS][KEYCOUNT] = {{
  KEY_RETURN
}};
double actuationPoint = 0.5;

#define LEDCOUNT 64
uint8_t ledRGBStates[3][LEDCOUNT]; // rgb only needs to be 4 for rgbw

void setup() {
  // Start Serial Channel
  Serial.begin(115200);
  delay(1000);
  
  // BLE setup
  // Serial.println("Starting BLE work!");
  bleKeyboard.begin();
  
  // USB setup
  usbKeyboard.begin();
  USB.begin();

  // Led setup
  pixels.begin();

  // On Board Button
  // pinMode(buttonPin, INPUT_PULLUP);

  calibrateHallSensor();
}

void loop() {
  pixels.clear();

  for(int i=0; i<NUMPIXELS; i++) {

    pixels.setPixelColor(i, pixels.Color(0, 150, 0));
    pixels.show();
    delay(DELAYVAL);
  }
  // buttonState = digitalRead(buttonPin);

  // if (buttonState == LOW){
  //   if(bleKeyboard.isConnected()) {
  //     Serial.println("Using Bluetooth");
  //     bleKeyboard.print(msg);
  //   } 
  //   else {
  //     Serial.println("Using USB");
  //     usbKeyboard.print(msg);
  //   }
  //   delay(500);
  // }
  // delay(10);
  updateKeyPos();
  sendKeyMatrix(keyStates,keyMap,0);
  updateLeds();
}

// Key Updating Area
// TODO not returning the value, man handlin a global variable
void updateKeyPos(){
  #if ISANALOG
  analogUpdateKeyPos();
  #endif
  updateKeyStates();
}
void analogUpdateKeyPos(){
  // TODO all def switch for other types of analog keyswitch
  hallSensorReadAll();
}
void updateKeyStates(){
  #if ISANALOG
  for (int i = 0; i < sizeof(keyPos)/sizeof(double); i++){
    keyStates[i] = (keyPos[i]>=actuationPoint);
  }
  // #else
  // read electrical switches with matrix scan (not yet implemented)
  #endif
}

// Hall effect multiplexer driver
int hallCalibratedMax = 1;
int hallCalibratedMin = 0;
int calibrationCycles = 5; // How many times the key can change direction before the max and min are stored
void calibrateHallSensor(){
  int cyclesCounted = 0;
  int lastPos = 0;
  bool goingUp = false;
  // TODO make this detect and work with any key. Idealy the user can select to recalibrate a specific key and it will be stored on board.
  while(cyclesCounted <= calibrationCycles+1){ // add one because we dont know the start position and it may trip
    int thisPos = hallSensorRead(0);
    if (thisPos > lastPos){ // Assume that bigger number is farther down
      // if (goingUp){}
      if (thisPos > hallCalibratedMax){
        hallCalibratedMax = thisPos;
      }
      goingUp = false;
    } else { // does not account for same value
      if (!goingUp) {
        cyclesCounted++;
      }
      if (thisPos < hallCalibratedMin){
        hallCalibratedMin = thisPos;
      }
      goingUp = true;
    }
  }
}
void hallSensorReadAll(){
  for (int i = 0; i < sizeof(keyPos)/sizeof(double); i++){
    // TODO likeley bug, this is integer division. please cast all to double first
    double kpos = (double)(hallSensorRead(i)-hallCalibratedMin)  /  (double)(hallCalibratedMax-hallCalibratedMin);
    // Ensure kpos is below 1
    if (kpos-1>0){
      // TODO possible floating point bug
      kpos -= (kpos-1);
    }
    keyPos[i] = kpos;
  }
}
// Dummy code placeholder
int cycler = 0;
bool down = false;
int hallSensorRead(int _index){
  // returns the value of the hall sensor corisponding to this position
  // TODO make this real with real hardware, currently a dummy test code
  if (cycler > 5){
    down = true;
  }
  if (cycler < 1){
    down = false;
  }
  if (down){
    cycler --;
  } else {
    cycler ++;
  }
  return cycler;
}


// Key Sending Area
void sendKeyMatrix(bool _keyStates[KEYCOUNT], uint8_t _keyMap[MAXLAYERS][KEYCOUNT], uint activeLayer){
  for (int i = 0; i < sizeof(_keyStates)/sizeof(bool); i++){
    if (_keyStates[i]){
      sendKey(_keyMap[activeLayer][i]);
    }
  }
}

void sendKey(uint8_t keyToSend){
  // Logic to route key over usb or bluetooth
  // TODO currently backwards. please detect if usb is connected to a computer and prefer usb. Maybe check if caps lock turns on the led?
  // TODO please ensure that blekeyboard and usb keyboard use the same integer mapping.
  if(bleKeyboard.isConnected()) {
    bleKeyboard.write(keyToSend);
  }
  else {
    usbKeyboard.press(keyToSend);
  }
}

// Led Controll Area
void updateLeds(){
  // Handle animations and led matrix state
  displayWS2812Driver();
}
void displayWS2812Driver(){
  // Output led state to WS2812 chains
}