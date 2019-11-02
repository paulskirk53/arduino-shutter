
// NB modded to only run the stepper for open and close - remove the commented out lines for flap open and close for a fully functioning version.

// if emergency stop is pressed, in order to continue operation, the following procedure is required:
// if emergency stop interrupts an open operation, the system sets the last_state variable to open, so press the close button to move from the partially
// open state
// if emergency stop interrupts a close operation, the system sets the last_state variable to closed, so press the open button to move from the partially
// closed state



//this is the BOLLOCKS to BREXIT version October 2019
//It has a stepper control for the shutter belt drive and a DC motor control section for the actuator which opens the bottom flap
//
// this routine processes commands handed off by the command processor. This means that processes executed here (open close shutter and flap)
// are non blocking from the ASCOM driver's perspective.
// flap now refers to the lower flap which hinges outwards
// this routine receives commands from the radio master arduino - OS# CS# and SS#
// data is only returned by SS# - if the shutter is open return char message 'open' or 'closed'

//New November 2019 Shutter open and close by physical button press.
// investigate just adding temp action buttons onto the open and close pins - 36 and 47 (open, close)


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
#define Flapopen              42             // these two pins are connected to new limit mechanism for the lower flap
#define Flapclosed            38             

#define open_shutter_command  36             // input pin
#define close_shutter_command 47             // input pin
#define emergency_stop	      30             // input pin
#define shutter_status        48             // OUTPUT pin

String last_state         ;
long   openposition       ;
long   closeposition      ; 
int    normalAcceleration ;
float  StepsPerSecond     ;


// end declarations -------------------------------------------------------------------------------------------------------

void setup() // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
{

	Serial.begin(9600);                               // not required outside of testing

	// Define the pin modes. This avoids pins being low (which activates relays) on power reset.
	// pinmodes for the open, close and emergency stop command pins and the shutter status pin
	pinMode(open_shutter_command,  INPUT_PULLUP) ;
	pinMode(close_shutter_command, INPUT_PULLUP) ;
	pinMode(emergency_stop,        INPUT_PULLUP) ;    // user push button to stop the motor
	pinMode(shutter_status,        OUTPUT)       ;    // this routine sets this pin and it is read by the command processor arduino

	digitalWrite(shutter_status,   HIGH);             // HIGH means closed

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
	StepsPerSecond     = 1000.0 ;                  // changed following empirical testing
	normalAcceleration = 50     ;                  // changed following empirical testing
	stepper.setMaxSpeed(StepsPerSecond);           // steps per second see below -
	// the controller electronics is set to 0.25 degree steps, so 15 stepspersecond*0.25= 3.75 degrees of shaft movement per second
	stepper.setAcceleration(normalAcceleration);
	stepper.setCurrentPosition(closeposition);     // intial position for stepper is closed
	

	last_state    = "closed" ;
	openposition  = 1000     ;                     // change when implemented
	closeposition = 0        ;

	// delay below introduced to give the command processor time to define its pin states, which are used by this sketch
	//there was a problem where relay 1 (OS) was activated due to indeterminate state of pin 46 as was the thinking.
	//this delay seemed to cure the problem. fURTHER ANALYSIS - 30-3-19 shows that if power is lost to the two arduino boards
	// i.e. command processor and shutter, the relay system activates relay 1 which is open flap

	delay(5000) ;
	Serial.println("Shutter Processor ready");


}  // end setup -----------------------------------------------------------------------------------------------------------

void loop() // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
{
	// Serial.println(last_state); //testing only

	if ((digitalRead(open_shutter_command) == LOW) && (last_state == "closed"))   // open shutter command
	{
		Serial.println("received open");               // testing only print this to sermon when 36 was grounded
		
		open_process() ;
		digitalWrite(shutter_status, LOW) ;            // set the status pin - low is shutters open
		last_state = "open" ;

	}

	if ((digitalRead(close_shutter_command) == LOW) && (last_state == "open")) // close shutter command
	{

		close_process();
		digitalWrite(shutter_status, HIGH) ;       // set the status pin - high is closed
		last_state = "closed";
		Serial.println("received close");          // testing only

	}
	// {last_state}{cs}{os}{x}{digitalRead(FLAPRELAY1)}{digitalRead(FLAPRELAY2)}{digitalRead(SHUTTERRELAY3)}{digitalRead(SHUTTERRELAY4)} {opencount} {closecount}

} // end void loop ----------------------------------------------------------------------------------------------------------

void initialise_relays() // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
{
	//  Serial.println( "  Initialising relays ");
	digitalWrite(FLAPRELAY1,    HIGH) ;
	digitalWrite(FLAPRELAY2,    HIGH) ;

} // end intialise relays ---------------------------------------------------------------------------------------------------

void close_process() // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
{

	// flap_close_process();
	shutter_close_process();

} // end  Close Process ---------------------------------------------------------------------------------------------

void open_process()  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
{
	// Open the shutter first, then the lower flap

	shutter_open_process();
	// flap_open_process();

}     // end Open Process ----------------------------------------------------------------------------------------------

void shutter_open_process()  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
{
	Serial.println("shutter open process");  // testing only
	stepper.moveTo(openposition);
	measure_and_stop();                            //detect proximity (in steps) to the open position. stop when open

} // end shutter open process -------------------------------------------------------------------------------------

void shutter_close_process() // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
{
	stepper.moveTo(closeposition);
	measure_and_stop();                            //detect proximity (in steps) to the closed position. stop when closed
	
} // end shutter close process -------------------------------------------------------------------------------------- -

void measure_and_stop()  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
{

	while ((stepper.distanceToGo() != 0)   && (digitalRead( emergency_stop)==HIGH)) // if the motor is not there yet, and the emergency stop is not pressed, keep it running
		{
	
			stepper.run();
			Serial.println("stepper run...");  // req'd for debug only

		}

	Serial.println("stepper stopped...");

	check_for_emergency_stop();

} // end measure and stop ----------------------------------------------------------------------------------------------

void check_for_emergency_stop()
{

  if ((digitalRead(emergency_stop)==LOW) && (last_state == "closed"))
    {
	  stepper.setCurrentPosition(openposition);
    }

  if ((digitalRead(emergency_stop)==LOW) && (last_state == "open"))
    {
	  stepper.setCurrentPosition(closeposition);
    }

}




void flap_close_process() // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
{
	//close the flap

	initialise_relays();  // TURN THE POWER OFF

	
	digitalWrite(FLAPRELAY1, HIGH);          // retracting POLARITY - Flap CLOSES first - the way the mechanics works is that the
	digitalWrite(FLAPRELAY2, LOW);           // linear actuator has to retract to close the flap

	// Serial.println("waiting for the flap switch to close ...");

	while (digitalRead(Flapclosed) == HIGH)      //keep reading the Hall sensor
	{

		digitalRead(Flapclosed);

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

		digitalRead(Flapopen);                      // will go LOW to signify flap is fully open i.e. the Hall sensor has been activated

	}

	initialise_relays();  // TURN THE POWER OFF

} // end void flap_open_process()  ------------------------------------------------------------------------------------