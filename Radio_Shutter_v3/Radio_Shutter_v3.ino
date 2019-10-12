//this is the BOLLOCKS to BREXIT version October 2019
//It will have a stepper control for the shutter belt drive and a DC motor control section for the actuator which opens the bottom door
//
// flap now refers to the lower flap which hnges outwards

//the two motors are named shutter_belt and door

// revcount is no longer applicable  

//TO DO
// Routine for shutter belt
//Routine for door
//Shutter belt will goto and from a fixed position - no bastard sensors required - just a number of steps.
// door will probably require sensors as it uses the linear actuator
// this routine receives commands from the radio master arduino - OS# CS# and SS#
// data is only returned by SS# - if the shutter is open return char message 'open' or 'closed'
//
//

#include <AccelStepper.h>

// pin definitions for step, dir and enable and create the stepper instance

#define                stepPin 7             // step pin tested and works - motor moves
#define                dirPin  8
#define                enaPin  9             // presently n/c - the enable pin

AccelStepper  stepper(AccelStepper::DRIVER, stepPin, dirPin, true);

// pin definitions for shutter relays

// These data pins link to  Relay board pins IN1, IN2, in3 and IN4
#define FLAPRELAY1     4   // arduino  pin 4
#define FLAPRELAY2     5   // arduino  pin 5

  

// shutter microswitches

//Pin 11 was defined here previously, but no longer used. It is still brought out from the arduino as pin 11 blue wire to the terminal block
#define Flapopen              42             // connected to new limit mechanism with aluminium and wood disc actuator
#define Flapclosed            38             // connected to new limit mechanism with aluminium and wood disc actuator

#define open_shutter_command  36             // input pin
#define close_shutter_command 47             // input pin
#define shutter_status        48             // OUTPUT pin

String last_state        = "closed";
long openposition =   5000;                 // change this to the number of steps required
long closeposition =  0;					// 
int normalAcceleration;
float StepsPerSecond;


void setup()
{

 // Serial.begin(9600);   //not required outside of testing

  // Define the pin modes. This avoids pins being low (which activates relays) on power reset.
  // pinmodes for the open and close command pins and the shutter status pin
  pinMode(open_shutter_command,  INPUT_PULLUP);
  pinMode(close_shutter_command, INPUT_PULLUP);
  pinMode(shutter_status,        OUTPUT);           //this routine sets this pin and it is read by the command processor arduino
  
  digitalWrite(shutter_status,   HIGH);            // HIGH means closed

  // pinmodes for resin flap relays
  // Initialise the Arduino data pins for OUTPUT

  pinMode(FLAPRELAY1,    OUTPUT);
  pinMode(FLAPRELAY2,    OUTPUT);
 

  // initialsie the pins for shutter and flap microswitches - input_pullup sets initial state to 1
  
  pinMode (Flapopen,            INPUT_PULLUP);
  pinMode (Flapclosed,          INPUT_PULLUP);


  // ALL THE RELAYS ARE ACTIVE LOW, SO SET THEM ALL HIGH AS THE INITIAL STATE

  initialise_relays();      // sets all the relay pins HIGH for power off state

  //stepper setup:
  StepsPerSecond = 500.0;                       // changed following empirical testing
  normalAcceleration = 50;                       // changed following empirical testing
  stepper.setMaxSpeed(StepsPerSecond);          // steps per second see below -
  // the controller electronics is set to 0.25 degree steps, so 15 stepspersecond*0.25= 3.75 degrees of shaft movement per second
  stepper.setAcceleration(normalAcceleration);
  stepper.setCurrentPosition(closeposition);   //intial position for stepper is closed

// delay below introduced to give the command processor time to define its pin states, which are used by this sketch
//there was a problem where relay 1 (OS) was activated due to indeterminate state of pin 46 as was the thinking.
//this delay seemed to cure the problem. fURTHER ANALYSIS - 30-3-19 shows that if power is lost to the two arduino boards 
// i.e. command processor and shutter, the relay system activates relay 1 which is open flap

delay(5000);
  Serial.println("Shutter Processor ready");


}  // end setup

void loop()
{


  if ((digitalRead(open_shutter_command) == LOW) && (last_state == "closed"))   // open shutter command
  {
    // opencount++;
    // Serial.println ("received OS");              // for testing
    open_process();
    digitalWrite(shutter_status, LOW) ;            // set the status pin - low is open
    last_state = "open";

  }

  if ((digitalRead(close_shutter_command) == LOW) && (last_state == "open")) // close shutter command
  {
    // closecount++;
    // Serial.println ("received CS");
    close_process();
    digitalWrite(shutter_status, HIGH) ;     // set the status pin - high is closed
    last_state = "closed";
	
  }
 // {last_state}{cs}{os}{x}{digitalRead(FLAPRELAY1)}{digitalRead(FLAPRELAY2)}{digitalRead(SHUTTERRELAY3)}{digitalRead(SHUTTERRELAY4)} {opencount} {closecount}
  if (stepper.distanceToGo() != 0)
  {
    stepper.run();
  }

} // end void loop

void initialise_relays()
{
  //  Serial.println( "  Initialising relays ");
  digitalWrite(FLAPRELAY1,    HIGH);
  digitalWrite(FLAPRELAY2,    HIGH);
 
}

void close_process()
{
  
//close the lower resin flap first

  initialise_relays();  // TURN THE POWER OFF

  // Serial.println (" Waiting for flap to close " + String(digitalRead( Flapclosed)));


      digitalWrite(FLAPRELAY1, HIGH);          // retracting POLARITY - Flap CLOSES first - the way the mechanics works is that the
      digitalWrite(FLAPRELAY2, LOW);           // linear actuator has to retract to close the flap

                                               // Serial.println("waiting for the flap switch to close ...");

  while (digitalRead(Flapclosed) == HIGH)      //keep reading the Hall sensor
  {

    digitalRead(Flapclosed);


  }   // endwhile flapclosed

                                              // Serial.println (" ++++++++++++++  Flap now closed +++++++++++++" );
                                            // Serial.println( "  end of shutter closed routine ");
                                            // The flap and shutter are now closed so set the relays back to initial status -

    // Serial.println("---------------- END of CLOSE Process-------------------");
    // Serial.println("");


  initialise_relays();  // TURN THE POWER OFF

  resin_shutter_close_process();


} // end  Close Process




void open_process()
{


  // Open the resin-shutter first

  resin_shutter_open_process();

  // now open the lower resin flap
  initialise_relays();

  digitalWrite(FLAPRELAY1, LOW);             
  digitalWrite(FLAPRELAY2, HIGH);            
	  
	    /* debug prints below
	    Serial.println("----------------Open Process-------------------");
	    Serial.print("flap relay 1 (expect LOW)  ");
	    Serial.println( digitalRead(FLAPRELAY1));
	    Serial.print("flap relay 2 (expect HIGH) ");
	    Serial.println(digitalRead(FLAPRELAY2));

	    Serial.print("Flapvalue before while loop (Expect HIGH)" );
	    Serial.println(digitalRead(Flapopen));

        */

  while (digitalRead(Flapopen) == HIGH)         // keep reading the hall sensor until it changes (i.e. flap is fully open)
  {
                                                // debug on the open brace - {digitalRead(FLAPRELAY1)}{digitalRead(FLAPRELAY2)}

    digitalRead(Flapopen);                      // will go LOW to signify flap is fully open i.e. the Hall sensor has been activated


  }

  initialise_relays();  // TURN THE POWER OFF


                        // Serial.println("---------------- END of Open Process-------------------");
                        // Serial.println("");

}     // end  Open Process


void resin_shutter_open_process()
{
stepper.moveTo(openposition);
// put stepper code in here

}

void resin_shutter_close_process()
{
stepper.moveTo(closeposition);

}