/*
Code to run on the Particle Photon.
Because the Timer library is used, the Core won't work with the code as is.

Sensors:
  Door: A magnet travels along the upper left corner of the door and activates:
    pin A0 (digital) - DOOR_UP   - Magnetic reed switch at back of door.
    pin A1 (digital) - DOOR DOWN -    "      "      "   on front garage wall.

  Power detect: A voltage divider and capacitor shift the 15-22 volt pulsing 
  signal in the pushbutton wires down to 3.3 V safe levels in order to detect 
  when the wall switch cuts power to the garage door.
    pin A3 (analog) -  DOOR_POWER

Outputs:
  Relays on the [Relay Shield](https://docs.particle.io/datasheets/photon-shields/#relay-shield).
    pin D0 - Relay 1 - UPDOWN - The main up/down button
    pin D1 - Relay 2 - LIGHT  - The light button

[Blynk](http://blynk.cc) provides the interface to a smart phone, either iOS or Android.
*/

//#define BLYNK_DEBUG // Uncomment this to see debug prints
#include "blynk/BlynkSimpleParticle.h"

// Place your Blynk auth token here
// Go to the Project Settings (nut icon).
char auth[] = "deadbeefdeadbeef0123456789012345";  

const int pin_UP = A0;    // Active when low
const int pin_DOWN = A1;  // Active when low
const int pin_POWER = A3;
const int powerThreshold = 1000; // 0 - 4096 maps to 0 to 3.3V.  Voltage divider gives max of 2.9V, or about 3600.

volatile int doorState = 0; // -1 for down, 0 for middle, 1 for up
volatile int lastState = doorState;
volatile int powerState = 1; // 1 for power on, 0 for power off
volatile int upSensor = LOW;
volatile int downSensor = LOW;
volatile int powerReading = 4095;

WidgetLED up(V0);
WidgetLED mid(V1);
WidgetLED down(V2);
WidgetLED power(V3);

void checkDoor(void); // pre-define timer function

Timer checkTimer(2000, checkDoor);

void setup()
{
    //Serial.begin(9600);
    pinMode(pin_UP, INPUT_PULLUP);
    pinMode(pin_DOWN, INPUT_PULLUP);
    pinMode(pin_POWER, INPUT);
    
    pinMode(D7, OUTPUT);
    
    delay(5000);
    Blynk.begin(auth);
    checkTimer.start();
}

void loop()
{
    Blynk.run();
}

void checkDoor() {
    upSensor = digitalRead(pin_UP);
    downSensor = digitalRead(pin_DOWN);
    powerReading = analogRead(pin_POWER);
    
    // Check the wall switch power
    powerState = (powerReading < powerThreshold) ? 0 : 1;
    
    if (upSensor == LOW) { // sensors are active LOW
        doorState = 1; // door is up
        up.on();
        mid.off();
        down.off();
    } else if (downSensor == LOW) {
        doorState = -1; // door is down
        up.off();
        mid.off();
        down.on();
    } else if (downSensor == HIGH && upSensor == HIGH) {
        doorState = 0; // door is between up and down
        up.off();
        mid.on();
        down.off();
    }
    
    if (powerState == 1) {
        power.on();
    } else {
        power.off();
    }
}
