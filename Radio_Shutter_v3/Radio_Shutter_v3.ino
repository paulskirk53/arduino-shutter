//TO DO
// SET THE CONST number_of_revs to whatever is needed to work the shutter
//
// this routine receives commands from the radio master arduino - OS# CS# and SS#
// data is only returned by SS# - if the shutter is open return char message 'open' or 'closed'
//
//


// pin definitions for shutter relays

// These data pins link to  Relay board pins IN1, IN2, in3 and IN4
#define FLAPRELAY1  4      // arduino  pin 4
#define FLAPRELAY2  5      // arduino  pin 5
#define SHUTTERRELAY3  6      // etc
#define SHUTTERRELAY4  3     // 

// shutter microswitches

//Pin 11 was defined here previously, but no longer used. It is still brought out from the arduino as pin 11 blue wire to the terminal block
#define Flapopen       12                    //connected to new limit mechanism with aluminium and wood disc actuator
#define Flapclosed     13                    //connected to new limit mechanism with aluminium and wood disc actuator
#define shutter_limit_switch  9             // now used to detect transitions on the winch cam
#define open_shutter_command 46             //input pin
#define close_shutter_command 47            //input pin
#define shutter_status 48                   //OUTPUT pin

String last_state = "closed";
const int number_of_revs = 5;   // set this empirically depending upon number of turns of the winch required to open / close the shutter

void setup()
{

  Serial.begin(9600);

  
  // ALL THE RELAYS ARE ACTIVE LOW, SO SET THEM ALL HIGH AS THE INITIAL STATE
  // Then define the pin modes. This avoids pins being low (activates realays) on power reset.

  initialise_relays();      // sets all the relay pins HIGH for power off state

  // pinmodes for shutter and flap relays
  // Initialise the Arduino data pins for OUTPUT

  pinMode(FLAPRELAY1, OUTPUT);
  pinMode(FLAPRELAY2, OUTPUT);
  pinMode(SHUTTERRELAY3, OUTPUT);
  pinMode(SHUTTERRELAY4, OUTPUT);

  // initialsie the pins for shutter and flap microswitches - input_pullup sets initial state to 1
  pinMode(shutter_limit_switch, INPUT_PULLUP);  
  pinMode (Flapopen, INPUT_PULLUP);
  pinMode (Flapclosed, INPUT_PULLUP);

  // pinmodes for the open and close command pins and the shutter status pin

  pinMode(open_shutter_command, INPUT_PULLUP);
  pinMode(close_shutter_command, INPUT_PULLUP);
  pinMode(shutter_status, OUTPUT);           //this routine sets this pin and it is read by the command processor arduino
  digitalWrite(shutter_status, HIGH);        // HIGH means closed
 //  Serial.println( "  exit setup ");

}  // end setup

void loop()
{
 

    if ((digitalRead(open_shutter_command) == LOW) && (last_state == "closed"))   // open shutter command
    {
	  
      //   Serial.println ("received OS");              // for testing
         open_shutter();
	     digitalWrite(shutter_status, LOW) ;        // set the status pin - low is open
		 last_state = "open";
      
	}
    
    if ((digitalRead(close_shutter_command) == LOW) && (last_state == "open")) // close shutter command
    {
	  
	     // Serial.println ("received CS");
	      close_shutter();
	      digitalWrite(shutter_status, HIGH) ;     // set the status pin - high is closed
		  last_state = "closed";
        
	}


} // end void loop

void initialise_relays()
{
  //  Serial.println( "  Initialising relays ");
  digitalWrite(FLAPRELAY1, HIGH);
  digitalWrite(FLAPRELAY2, HIGH);
  digitalWrite(SHUTTERRELAY3, HIGH);
  digitalWrite(SHUTTERRELAY4, HIGH);
}

void close_shutter()
{
  // commands to close shutters
  // commands to close shutters reverse POLARITY TO BOTH motors
 
 // Serial.println( "  closing shutter ");
 
 
	 int revcount = 0;
	 
	 while (revcount <= number_of_revs)
	 {
		digitalWrite(SHUTTERRELAY3, HIGH);          // closing POLARITY shutter - closes first
		digitalWrite(SHUTTERRELAY4, LOW);

		 // now poll the limit switch for activations as the pulley rotates
		 if (digitalRead(shutter_limit_switch) == LOW)  // the limit switch has been pressed by the rotating cam
		 {
		     delay(3000);  // wait for the switch to open as the rotating cam moves on
			 revcount++;
			// Serial.print( "Rev count is : ");
			// Serial.println( revcount); 

		 }   //  endif digital read

	 }  // end while

	 initialise_relays();  // TURN THE POWER OFF
	  
	//   Serial.print( "Rev count is : ");
	//    Serial.println( revcount);
 


  initialise_relays();  // TURN THE POWER OFF

 // Serial.println (" Waiting for flap to close " + String(digitalRead( Flapclosed)));

  while (digitalRead(Flapclosed) == HIGH)       //high when not pushed closed, so use the NO connection to arduino for the closed state switch
  {
    digitalWrite(FLAPRELAY1, HIGH);          // EXTENDING POLARITY - Flap CLOSES second - the way the mechanics works is that the
    digitalWrite(FLAPRELAY2, LOW);           // linear actuator has to extend to close the flap
    digitalRead(Flapclosed);
    
   
  }   // endwhile flapclosed

  
 //Serial.println (" ++++++++++++++  Flap now closed +++++++++++++" );
 // Serial.println( "  end of shutter closed routine ");
  // The flap and shutter are now closed so set the relays back to initial status -

  initialise_relays();  // TURN THE POWER OFF

} // end  CS




void open_shutter()
{
  

  // Open the flap first

 // Serial.println ("Waiting for Flap to open  " + String(digitalRead( Flapopen)));
 
  while (digitalRead(Flapopen) == HIGH)         //high when not pushed closed, so use the NO connection to arduino for the open state switch
  {
    digitalWrite(FLAPRELAY1, LOW);             // retracting polarity - Flap opens first - the mechanics means that the actuator
    digitalWrite(FLAPRELAY2, HIGH);            // retracts in order to open the flap
    digitalRead(Flapopen);                     // will go LOW to signify flap is fully open i.e. the switch has ben pushed closed
    
   
  }

 // Serial.println ("Flap now open  ");
  

  initialise_relays();  // TURN THE POWER OFF


  // then open the shutter

 
	  int revcount = 0;
	  
	  while (revcount <= number_of_revs )
	  {
		  digitalWrite(SHUTTERRELAY3, LOW);          // these two lines from version 2 - they set the motor direction
		  digitalWrite(SHUTTERRELAY4, HIGH);

		  // now poll the limit switch for activations as the pulley rotates
		  if (digitalRead(shutter_limit_switch) == LOW)   // the limit switch has been pressed by the rotating cam
		  {
		      delay(3000);  // wait for the switch to open as the rotating cam moves on
			  revcount++;
			//  Serial.print( "Rev count is : ");
			//  Serial.println( revcount);

		  }  //endif
	  }  //end while

	  initialise_relays();  // TURN THE POWER OFF

 
 //Serial.print( "ending Rev count is : ");
 //Serial.println( revcount);
 //Serial.println( "------------- shutter open - end of shutter open routine ");
}// end  OS


