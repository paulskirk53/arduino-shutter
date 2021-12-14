

//December '21 wdt added BUT the shutter takes about 30 seconds to close, with the stepper.run in a while loop with no no dogkick
//so why hasn't the wdt rest the system? Obviously we don't want a reset, so once the problem has been sorted, dogkick will need to be in the loops
// July 9th '21 started removing the flap based code as this is not required for the pulsar dome
// this file was a clone done on 9th July from origin so is a working version for the new Pulsar dome.
// 22-7-21 adding a new function to switch on and off the DC (battery) power to the Shutter stepper - tested and works
// 30-7-21 adding in a rain sensor - tested and works

// If open or close is interrupted by an emergency stop button press, just press the same button again e.g. Open/ ES/ Open

// this routine processes commands handed off by the command processor. This means that processes executed here (open / close shutter )
// are non blocking from the ASCOM driver's perspective.

// this routine receives commands from the radio master MCU - OS# CS# and SS#
// data is only returned by SS# - if the shutter is open return char message 'open' or 'closed'
#include <avr/wdt.h>            //implement the watchdog timer
#include "Radio_Shutter.h"
// Compiler declarations follow

// power management for shutter stepper controller - MA860H - this is via the SS relay
// and the power for the rain sensor device - this only draws 8 mA so can easily be powered from a pin
#define power_pin    9                  // this drives the gate of the SSR40DD device to turn on the power to the motor which opens / closes the shutter added the pin on 22-7-21
#define SensorPower 10                  // rain sesnor power pin added 30-7-21

// step, dir and enable pin definitions
#define stepPin 7                       // step pin tested and works - motor moves
#define dirPin  8

// create the stepper instance
AccelStepper stepper(AccelStepper::DRIVER, stepPin, dirPin, true);

// Data pin definitions

#define open_shutter_command      36   // input pin
#define close_shutter_command     47   // input pin
#define emergency_stop            30   // input pin   green (mcu side ) blue on HC side
#define shutter_status            48   // OUTPUT pin
#define push_button_open_shutter  52   // mauve (MCU side) to green (HC cable side) to hand control switch unit
#define push_button_close_shutter 53   // blue (MCU side) to yellow (HC cable side) hand control switch unit
#define Rain_Monitor              11   // rain monitor data pin
#define closedSensor_pin          12   // looks like it's free - Pin goes LOW when magnet is detected
#define MCU_reset                 13   // this is pulled LOW by the command processor to reset this cpu
#define on true
#define off false

String last_state;
long openposition;
long closeposition;

int normalAcceleration;
float StepsPerSecond;                 // has to be this type as per library
bool open_command;
bool close_command;
bool rainSensorEnable;

bool powerOnFlag;
unsigned long powerOnStartTime;
unsigned long powerOnDuration;       // this is a failsafe for the shutter opening motor - ha ha haha ha haha ha haha ha haha ha haha ha haha ha haha ha ha ha ha haha ha haha ha ha
//ha ha haha ha haha ha haha ha haha ha haha ha haha ha haha ha haha ha haha ha haha ha haha ha haha ha ha he he he ha ha ha hoo hoo hoo ha ha eyes watering ha ha ha
// end declarations -------------------------------------------------------------------------------------------------------

void setup() // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
{

  // Serial.begin(19200);                               // not required outside of testing

  // Define the pin modes. This avoids pins being low (which activates relays) on power reset.
  // pinmodes for the open, close and emergency stop command pins and the shutter status pin
  pinMode (MCU_reset, INPUT_PULLUP);
  pinMode(open_shutter_command, INPUT_PULLUP);
  pinMode(close_shutter_command, INPUT_PULLUP);
  pinMode(emergency_stop, INPUT_PULLUP);        // user push button to stop the motor
  pinMode(shutter_status, OUTPUT);              // this routine sets this pin and it is read by the command processor arduino
  pinMode(closedSensor_pin, INPUT_PULLUP );            // this wilL BE high WHEN THE SHUTTER is open
  pinMode(push_button_open_shutter, INPUT);     // changed from INPUT_PULLUP
  pinMode(push_button_close_shutter, INPUT_PULLUP);

  digitalWrite(shutter_status, HIGH); // HIGH means closed

  // pinmode for DC power management of the Shutter Stepper
  pinMode(power_pin, OUTPUT);
  digitalWrite(power_pin, LOW);        //  power will be off when the setup routine executes
  pinMode(Rain_Monitor, INPUT_PULLUP); //  the rain monitor is active low
  pinMode(SensorPower, OUTPUT);
  digitalWrite(SensorPower, LOW); // this pin acts as a power supply for the rain sensor device.
  // Power should only be on when testing for rain to avoid corrosion of the sensor pad.

  // stepper setup:
  closeposition    = 0;
  StepsPerSecond = 500.0;              // changed following empirical testing
  normalAcceleration = 50;             // changed following empirical testing
  stepper.setMaxSpeed(StepsPerSecond); // steps per second see below -
  // if the controller electronics is set to 0.25 degree steps, 15 stepspersecond*0.25= 3.75 degrees of shaft movement per second
  stepper.setAcceleration(normalAcceleration);
  stepper.setCurrentPosition(closeposition); // initial position for stepper is closed

  last_state       = "closed";    //this is an assumption which may not be true, especially if wdt resets
  openposition     = 3300; // was set to 4000 which was a bit too high. Changed December 2021
  
  rainSensorEnable = true;
  powerOnDuration  = 30000; // 30 seconds
  powerOnFlag      = off;
  // delay below introduced to give the command processor time to define its pin states, which are used by this program

  delay(5000);

/* write some code here which checks if the hall sesnor at the closed position is detecting 'closed'
   if not, invoke a shutter initialisation routine which calls closeshutter but stops the movement when the Hall sensor detects closed

*/
 if ( digitalRead(closedSensor_pin) == HIGH )   // pin is HIGH if the shutter is OPEN
  {    // write code here to close the shutter
       // assume the shutter is at worst case fully open when a reset occurs
       // it might be best to go at half the normal speed or less, so as not to shock the motor / chain when the hall sensor is reached

    stepper.setCurrentPosition(openposition);  // assume worst case of shutter fully open
    PowerOn();
    close_shutter();    // note an amendment to the while loop condition in close shutter needs to be checked for Hall sensor logic - is closed HIGH or LOW?
    PowerOff();

  }


  wdt_enable(WDTO_4S);                 //Watchdog set to 4 seconds note this statement is placed after the delay
  // Note there is a 2 second delay in the poweron() routine, so 4 seconds seems reasonable for the wdt.
} // end setup -----------------------------------------------------------------------------------------------------------

void loop() // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
{
  while (digitalRead(emergency_stop) == LOW) // the ES is normally low and goes to the power rail via a pullup (HIGH) when pressed.
  {

    if ( digitalRead(MCU_reset) == LOW )
    {
      while(1)
      {}       //causes the WDT to reset the MCU
    }

    // monitor the poweron timer flag. if the power has been on for more than the set time period, turn it off
    if (powerOnFlag)
    {
      if ((millis() - powerOnStartTime) > powerOnDuration) // if the power to the shutter mechanism has been on past the limit allowed, turn the bast off
      {
        PowerOff();
      }
    }
    open_command = false;
    close_command = false;

    // if either the ascom driver or the manual pushbutton asserts open
    if ((digitalRead(open_shutter_command) == LOW) | (digitalRead(push_button_open_shutter) == LOW)) //
    {
      open_command = true;
    }

    if ((digitalRead(close_shutter_command) == LOW) | (digitalRead(push_button_close_shutter) == LOW))
    {
      close_command = true;
    }

    if (open_command && (last_state == "closed")) // open shutter command
    {
      // Serial.println("received open");               // testing only print this to sermon when 36 was grounded
      PowerOn(); //  power on to the stepper
      open_shutter();
      PowerOff();                        //  power off to the stepper
      digitalWrite(shutter_status, LOW); // set the status pin - low is shutters open
    }

    if (close_command && (last_state == "open")) // close shutter command
    {
      // Serial.println("received close");              // testing only
      PowerOn(); // power on to the stepper
      close_shutter();
      PowerOff();                         // power off to the stepper
      digitalWrite(shutter_status, HIGH); // set the status pin - high is closed
    }


    Check_if_Raining();

    // kick the dog if the MCU_reset pin is high. If the pin has gone LOW, then the command processor MCU has issued a reset command
    dogkick();

  } // endwhile emergency stop loop

  // execution does not get here unless ES is button is pressed

   // kick the dog if the MCU_reset pin is high. If the pin has gone LOW, then the command processor MCU has issued a reset command
    dogkick();

}



// end void loop ----------------------------------------------------------------------------------------------------------
void dogkick()
{
        
     wdt_reset();                       //execute this command within 4 seconds to keep the timer from resetting the CPU
    
}
void open_shutter() // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
{

  // turn on the rain sensor device power, so that it is live whilst the shutter is open
  digitalWrite(SensorPower, HIGH);

  stepper.moveTo(openposition);

  while ((stepper.distanceToGo() != 0) && (digitalRead(emergency_stop) == LOW))
  {
    stepper.run();
  }

  // set the open / closed state as follows: If the open process has been interrupted then last_state needs to remain "closed" so that the open command can be used again

  if ((digitalRead(emergency_stop) == HIGH)) // if this is true, the ES was pressed while the shutter was opening
  {
    last_state = "closed";
  }
  else
  {
    last_state = "open";
  }

} // end  open shutter process -------------------------------------------------------------------------------------

void close_shutter() // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
{

  stepper.moveTo(closeposition);

  while ((stepper.distanceToGo() != 0) && (digitalRead(emergency_stop) == LOW)  && ( digitalRead(closedSensor_pin) == HIGH)   )  // Pin is HIGH while the shutter is OPEN
  {
    stepper.run();
  }
  // set the open / closed state as follows: If the close process has been interrupted then last_state needs to remain "open" so that the close command can be used again

  if ((digitalRead(emergency_stop) == HIGH)) // if this is true, the ES was pressed while the shutter was closing
  {
    last_state = "open";
  }
  else
  {
    last_state = "closed";
  }
  // turn off the rain sensor device power, so that it is unpowered whilst the shutter is closed.
  digitalWrite(SensorPower, LOW); // turn off power to sensor to avoid undue corrosion

} // end close shutter process -------------------------------------------------------------------------------------- -

void PowerOn() // set the power SSR gate high
{
  digitalWrite(power_pin, HIGH);
  // set the poweron timer flag
  powerOnFlag = on;
  delay(2000); // gives time for the MA860H unit to power on and stabilise
}

void PowerOff() // set the power SSR gate low
{
  digitalWrite(power_pin, LOW);
  powerOnFlag = off;
}

void Check_if_Raining()
{
  if ((digitalRead(Rain_Monitor) == LOW) && (last_state == "open")) // if we're open and it's raining....
  {
    PowerOn();
    close_shutter();
    PowerOff(); // this is the MA860H shutter driver power
                //  also turn off the SensorPower - this action is in he shutter close routine
    last_state = "closed";
    digitalWrite(shutter_status, HIGH); // set the status pin - high is closed
  }
}