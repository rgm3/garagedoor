/*
Particle Photon code for reporting on the garage door status and operating
the relay shield via Blynk buttons.

Not currently compatible with the Spark Core, due to Photon Timer library use.
It may be possible to use the freertos4core or SparkIntervalTimer libraries
for Core compatibility.  See:
  https://docs.particle.io/reference/firmware/photon/#software-timers

Sensors:
  Door: A magnet travels along the upper left corner of the door and activates:
    pin A0 (digital) - DOOR_UP
    pin A1 (digital) - DOOR DOWN
    pin A3 (analog) -  DOOR_POWER

Outputs:
  Relays on the [Relay Shield](https://docs.particle.io/datasheets/photon-shields/#relay-shield).
    pin D0 - Relay 1 - UPDOWN - The main up/down button
    pin D1 - Relay 2 - LIGHT  - The light button

[Blynk](http://blynk.cc) provides the interface to a smart phone, either iOS or Android.

On voltages and sensing the door power supply:
  When disconnected from the button panel, the wires have an potential difference of about 22 volts.
  When hooked up to the screw terminals on the panel this voltage falls to about 16 volts.
  The panel wires cary a signal that I didn't document or reverse engineer.
  There's a 5ms low pulse to ground at 80Hz I think, when nothing is pressed.
  Pressing buttons modifies the time that the signal is LOW.
  The wire voltage is fed to the photon's 3.3 V tolerant analog input through a voltage 
  divider and low-pass filter in order to detect when the garage door has power, because there's
  a wall switch that can turn it off, and it'd be nice to know if the power was out.

  Voltage divider w/ low-pass filter:

  + o--+
       R1
       |
 Vs    +---+-----o  +
       |   |
       R2  = C1      Vo, to Particle photon pins
       |   |
  - o--+---+-----o  -
  Vs is between 15 and 18 V, R1 = 15 kΩ, R2 = 3.3 kΩ, and C1 = 2.2 µF
  Divides the signal on the wires down to about 2.9 V.
  Since 0 - 4095 represent 0 - 3.3V, the power pin usually reads value "3600" (2.9 / 3.3 * 4095).
  When it's significantly lower than that, power to the garage door opener is off.

*/

#include "blynk/BlynkSimpleParticle.h"

// Place your Blynk auth token here
// Go to the Project Settings (nut icon).
char auth[] = "deadbeefdeadbeef0123456789012345";  

// A magnet is stuck to the upper left corner of the door and gets close to two reed switches
const int pin_UP = A0;    // Reed switch at top of door track, pulls to ground
const int pin_DOWN = A1;  // Reed switch on face of garage door wall, pulls to ground
const int pin_POWER = A3; // voltage divider on the signal wires
const int pin_DOOR = D0;  // Relay Shield relay 1 (the old square white one)
const int pin_LIGHT = D1; // Relay Shield relay 2

// Variables for sensor reading.
bool up, middle, down, power;
int analogPower = 0;

// Previous readings, used to detect changes
volatile bool up_last, down_last, power_last = 0;

// Blynk LED widgets on "virtual pins"
WidgetLED led_up(V0);
WidgetLED led_mid(V1);
WidgetLED led_down(V2);
WidgetLED led_power(V3);

void checkDoor(void); // pre-define timer function
Timer checkTimer(2000, checkDoor);

void setup()
{
    pinMode(pin_UP, INPUT_PULLUP);
    pinMode(pin_DOWN, INPUT_PULLUP);
    pinMode(pin_POWER, INPUT);
    pinMode(pin_DOOR, OUTPUT);
    pinMode(pin_LIGHT, OUTPUT);

    
    delay(5000);
    Blynk.begin(auth);

    checkTimer.start();
}

void loop()
{
    // The Blynk app controls pin_DOOR and pin_LIGHT automatically via buttons hooked to D0 and D1
    Blynk.run();

    // Read the sensors, set boolean states
    up = digitalRead(pin_UP) == LOW;
    down = digitalRead(pin_DOWN) == LOW;
    middle = !(up || down); // neither up nor down
    analogPower = analogRead(pin_POWER);

    power = analogPower < 1000; // Arbitrary low threshold
    // We could have probably used digitalRead if the divider and voltages 
    // were measured carefully beyond "did it scale to a safe place for the pin."
    // As a matter of fact, R1 should change from 15kΩ to 22kΩ to handle up to 25 volts

    delay(10);
}

/*
 * Periodically looks for changes in the door sensor readings and turns on Blynk LED widgets in response.
 */
void checkDoor()
{
    if (up != up_last) {
        if (up) {
            led_up.on();  // Equivalent to led_up.setValue(255);
        } else {
            led_up.off(); // Equivalent to led_up.setValue(0);
        }
        // led_up.setValue(up ? 255 : 0); // More compact than above, but harder to understand.  See "ternary operator."
    }

    if (down != down_last) {
        led_down.setValue(down ? 255 : 0);
    }

    if (power != power_last) {
       led_power.setValue(power ? 255 : 0);
    }

    if (up != up_last || down != down_last) {
        led_mid.setValue(middle ? 255 : 0);
    }

    up_last = up;
    down_last = down;
    power_last = power;
}
// vim: noai:ts=4:sw=4
