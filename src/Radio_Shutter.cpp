//Version 1.0 update the variable pkversion too 
//If open or close is interrupted by an emergency stop button press, just press the same button again e.g. Open/ ES/ Open
//It has a stepper control for the shutter belt drive and a DC motor control section for the actuator which opens the bottom flap
//
// this routine processes commands handed off by the command processor. This means that processes executed here (open close shutter and flap)
// are non blocking from the ASCOM driver's perspective.
// flap now refers to the lower flap which hinges outwards
// this routine receives commands from the radio master arduino - OS# CS# and SS#
// data is only returned by SS# - if the shutter is open return char message 'open' or 'closed'



// Compiler declarations follow

#include <AccelStepper.h>

// step, dir and enable pin definitions
#define stepPin               7             // step pin tested and works - motor moves
#define dirPin                8
#define enaPin                9             // presently n/c - the enable pin

// create the stepper instance
AccelStepper  stepper(AccelStepper::DRIVER, stepPin, dirPin, true);

// shutter relay pin definitions

// These data pins link to  Relay board pins IN1, IN2, in3 and IN4
#define FLAPRELAY1             4             // arduino  pin 4
#define FLAPRELAY2             5             // arduino  pin 5

// shutter microswitch pin definitions

//Pin 11 was defined here previously, but no longer used. It is still brought out from the arduino as pin 11 blue wire to the terminal block
#define Flapopen                  42             // these two pins are connected to new limit mechanism for the lower flap
#define Flapclosed                38

#define open_shutter_command      36             // input pin
#define close_shutter_command     47             // input pin
#define emergency_stop	          30             // input pin
#define shutter_status            48             // OUTPUT pin
#define push_button_open_shutter  52             // green to hand controll switch unit
#define push_button_close_shutter 53             // blue to hand controll switch unit

String last_state           ;
long   openposition         ;
long   closeposition        ;

int    normalAcceleration   ;
float  StepsPerSecond       ;
bool   open_command         ;
bool   close_command        ;


// end declarations -------------------------------------------------------------------------------------------------------

void setup() // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
{

  Serial.begin(19200);                               // not required outside of testing

  // Define the pin modes. This avoids pins being low (which activates relays) on power reset.
  // pinmodes for the open, close and emergency stop command pins and the shutter status pin
  pinMode(open_shutter_command,       INPUT_PULLUP) ;
  pinMode(close_shutter_command,      INPUT_PULLUP) ;
  pinMode(emergency_stop,             INPUT_PULLUP) ;    // user push button to stop the motor
  pinMode(shutter_status,             OUTPUT)       ;    // this routine sets this pin and it is read by the command processor arduino

  pinMode(push_button_open_shutter,   INPUT) ;           //changed from INPUT_PULLUP
  pinMode(push_button_close_shutter,  INPUT_PULLUP) ;

  digitalWrite(shutter_status,        HIGH);             // HIGH means closed

  // pinmodes for flap relays
  // Initialise the Arduino data pins for OUTPUT

  pinMode(FLAPRELAY1,            OUTPUT) ;
  pinMode(FLAPRELAY2,            OUTPUT) ;


  // initialise the pins for shutter and flap microswitches - input_pullup sets initial state to 1

  pinMode (Flapopen,             INPUT_PULLUP) ;
  pinMode (Flapclosed,           INPUT_PULLUP) ;


  // ALL THE RELAYS ARE ACTIVE LOW, SO SET THEM ALL HIGH AS THE INITIAL STATE

  initialise_relays();                            // sets all the relay pins HIGH for power off state

  //stepper setup:
  StepsPerSecond     = 500.0 ;                   // changed following empirical testing
  normalAcceleration = 50     ;                  // changed following empirical testing
  stepper.setMaxSpeed(StepsPerSecond);           // steps per second see below -
  // if the controller electronics is set to 0.25 degree steps, 15 stepspersecond*0.25= 3.75 degrees of shaft movement per second
  stepper.setAcceleration(normalAcceleration);
  stepper.setCurrentPosition(closeposition);     // initial position for stepper is closed


  last_state      = "closed" ;
  openposition    = 4000     ;                 //set back to  8000; once tests are complete
  closeposition   = 0        ;



  // delay below introduced to give the command processor time to define its pin states, which are used by this sketch
  //there was a problem where relay 1 (OS) was activated due to indeterminate state of pin 46 as was the thinking.

  delay(5000) ;
  Serial.println("Shutter Processor ready");


}  // end setup -----------------------------------------------------------------------------------------------------------

void loop() // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
{
  while (digitalRead(emergency_stop) == LOW)    // the ES is normally low and goes open circuit (HIGH) when pressed.
  {
    open_command = false;
    close_command = false;

    // Serial.print("open shutter command pin read gives ");
    //Serial.println(digitalRead(open_shutter_command));

    //Serial.print("close shutter command pin read gives ");
    //Serial.println(digitalRead(close_shutter_command));
    //delay(500);
    //
// if either the ascom driver or the manual pushbotton asserts open
    if ( (digitalRead(open_shutter_command) == LOW)  | (digitalRead(push_button_open_shutter) == LOW)) //
    {
      open_command  = true;
    }

    if   ( (digitalRead(close_shutter_command) == LOW) | (digitalRead(push_button_close_shutter) == LOW))
    {
      close_command = true;
    }

    //Serial.print("Open command = ");
    //Serial.println(open_command);

    //Serial.print("Close command = ");
    //Serial.println(close_command);
    //delay(500);


    if (open_command && (last_state == "closed"))    // open shutter command
    {
      Serial.println("received open");               // testing only print this to sermon when 36 was grounded

      open_shutter() ;
      digitalWrite(shutter_status, LOW) ;            // set the status pin - low is shutters open
    }


    if (close_command && (last_state == "open")) // close shutter command
    {
      Serial.println("received close");          // testing only
      
      close_shutter();
      digitalWrite(shutter_status, HIGH) ;       // set the status pin - high is closed

    }

  }               //endwhile emergency stop loop

} // end void loop ----------------------------------------------------------------------------------------------------------



void open_shutter()  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
{
  Serial.println("open shutter process");  // testing only


  stepper.moveTo(openposition);


  Serial.print("distance to go function value " );
  Serial.println(stepper.distanceToGo());


  while ( (stepper.distanceToGo() != 0) && (digitalRead(emergency_stop) == LOW) )
  {
    stepper.run();
  }

  //set the open / closed state as follows: If the open process has been interrupted then last_state needs to remain "closed" so that the open command can be used again

  if ((digitalRead(emergency_stop) == HIGH))  // if this is true, the ES was pressed while the shutter was opening
  {
    last_state = "closed";
  }
  else
  {
    last_state = "open";
  }


  Serial.print("Steper current position is ");
  Serial.println(stepper.currentPosition());


} // end  open shutter process -------------------------------------------------------------------------------------

void close_shutter() // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
{
  Serial.println("Shutter close Process");          // testing only

  stepper.moveTo(closeposition);


  Serial.print("distance to go function value " );
  Serial.println(stepper.distanceToGo());



  while ( (stepper.distanceToGo() != 0) && (digitalRead(emergency_stop) == LOW) )
  {
    stepper.run();
  }
 //set the open / closed state as follows: If the close process has been interrupted then last_state needs to remain "open" so that the close command can be used again

  if ((digitalRead(emergency_stop) == HIGH))  // if this is true, the ES was pressed while the shutter was closing
  {
    last_state = "open";
  }
  else
  {
    last_state = "closed";
  }
  Serial.print("Steper current position is ");
  Serial.println(stepper.currentPosition());

} // end close shutter process -------------------------------------------------------------------------------------- -


//--------------------- below not used yet - coded in readiness for the lower flap motor ------------------------------------


void flap_close_process() // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
{
  //close the flap

  initialise_relays();  // TURN THE POWER OFF


  digitalWrite(FLAPRELAY1, HIGH);          // retracting POLARITY - Flap CLOSES first - the way the mechanics works is that the
  digitalWrite(FLAPRELAY2, LOW);           // linear actuator has to retract to close the flap

  // Serial.println("waiting for the flap switch to close ...");

  while (digitalRead(Flapclosed) == HIGH)      //keep reading the Hall sensor
  {

    digitalRead(Flapclosed);                  // this is a redundant statement

  }   // endwhile flapclosed


  initialise_relays();  // TURN THE POWER OFF

} // end void flap_close_process() ------------------------------------------------------------------------------------


void flap_open_process() // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

{
  // open the flap
  initialise_relays();

  digitalWrite(FLAPRELAY1, LOW);
  digitalWrite(FLAPRELAY2, HIGH);


  while (digitalRead(Flapopen) == HIGH)         // keep reading the hall sensor until it changes (i.e. flap is fully open)
  {
    // debug on the open brace - {digitalRead(FLAPRELAY1)}{digitalRead(FLAPRELAY2)}

    digitalRead(Flapopen);                      // this is a redundant statement

  }

  initialise_relays();  // TURN THE POWER OFF

} // end void flap_open_process()  ------------------------------------------------------------------------------------

void initialise_relays() // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
{
  //  Serial.println( "  Initialising relays ");
  digitalWrite(FLAPRELAY1,    HIGH) ;
  digitalWrite(FLAPRELAY2,    HIGH) ;

} // end intialise relays ---------------------------------------------------------------------------------------------------
