//this version tested on a board with wies, manually connected disconnected to simulate shutter open/ close
// and flap open / close.
// 29-3-19 changed the revcount variables so that they are separate for open/ close. this seemed to fix the 
// problem of needing 5 switch closures for open and six for close. I have no idea why - there is no logic.

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
#define Flapopen       42                   //connected to new limit mechanism with aluminium and wood disc actuator
#define Flapclosed     38                    //connected to new limit mechanism with aluminium and wood disc actuator
#define shutter_limit_switch  9             // now used to detect transitions on the winch cam
#define open_shutter_command 36             //input pin
#define close_shutter_command 47            //input pin
#define shutter_status 48                   //OUTPUT pin

String last_state = "closed";
const int number_of_revs = 5;     // set this empirically depending upon number of turns of the winch required to open / close the shutter
long motor_time_limit    = 30000; // 30 second limit for winch motor to be active
long motor_start_time    = 0;     // used to measure how long the winch motor runs
//int flaprelay1count = 0;        // used for testing with breakpoints
// boolean os, cs;
// int opencount=0;
// int closecount=0;

void setup()
{

 Serial.begin(9600);   //not required outside of testing

  // Define the pin modes. This avoids pins being low (activates realays) on power reset.
  // pinmodes for the open and close command pins and the shutter status pin
  pinMode(open_shutter_command, INPUT_PULLUP);
  pinMode(close_shutter_command, INPUT_PULLUP);
  pinMode(shutter_status, OUTPUT);           //this routine sets this pin and it is read by the command processor arduino
  
  digitalWrite(shutter_status, HIGH);        // HIGH means closed

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


  // ALL THE RELAYS ARE ACTIVE LOW, SO SET THEM ALL HIGH AS THE INITIAL STATE

  initialise_relays();      // sets all the relay pins HIGH for power off state

// delay below introduced to give the command processor time to define its pin states, which are used by this sketch
//there was a problem where relay 1 (OS) was activated due to indeterminate state of pin 46 as was the thinking.
//this delay seemed to cure the problem. fURTHER ANALYSIS - 30-3-19 shows that if power is lost to the two arduino boards 
// i.e. command processor and shutter, the relay system activates relay 1 which is open flap

delay(5000);
  Serial.println("Shutter Processor ready");


}  // end setup

void loop()
{
// os=digitalRead(open_shutter_command);
// cs=digitalRead(close_shutter_command);

  if ((digitalRead(open_shutter_command) == LOW) && (last_state == "closed"))   // open shutter command
  {
    // opencount++;
    // Serial.println ("received OS");              // for testing
    open_process();
    digitalWrite(shutter_status, LOW) ;        // set the status pin - low is open
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

} // end void loop

void initialise_relays()
{
  //  Serial.println( "  Initialising relays ");
  digitalWrite(FLAPRELAY1, HIGH);
  digitalWrite(FLAPRELAY2, HIGH);
  digitalWrite(SHUTTERRELAY3, HIGH);
  digitalWrite(SHUTTERRELAY4, HIGH);
}

void close_process()
{
  // commands to close shutters
  // commands to close shutters reverse POLARITY TO BOTH motors

  // Serial.println( "  closing shutter ");
    Serial.println("---------------- CLOSE Process-------------------");
    Serial.println("");
    Serial.println( " closing shutter First just wait 15 seconds");



  int closerevcount = 0;                           //changed var name


      digitalWrite(SHUTTERRELAY3, HIGH);          // closing POLARITY shutter - closes first
      digitalWrite(SHUTTERRELAY4, LOW);
      motor_start_time = millis();

  while (closerevcount <= number_of_revs)
  {

  	  	  if ((millis() - motor_start_time) > motor_time_limit)   // don't let the motor run longer than defined time limit
  	  	  {
	  	  	  closerevcount = number_of_revs +1;           // kill the while loop
  	  	  }

    // now poll the limit switch for activations as the pulley rotates
    if (digitalRead(shutter_limit_switch) == LOW)  // the limit switch has been pressed by the rotating cam
    {
      delay(750);  // wait for the switch to open as the rotating cam moves on
      closerevcount++;


    }   //  endif digital read

  }  // end while

  

  //   Serial.print( "Rev count is : ");
  //    Serial.println( revcount);



  initialise_relays();  // TURN THE POWER OFF

  // Serial.println (" Waiting for flap to close " + String(digitalRead( Flapclosed)));


      digitalWrite(FLAPRELAY1, HIGH);          // EXTENDING POLARITY - Flap CLOSES second - the way the mechanics works is that the
      digitalWrite(FLAPRELAY2, LOW);           // linear actuator has to extend to close the flap

Serial.println("waiting for the flap switch to close ...");

  while (digitalRead(Flapclosed) == HIGH)       //high when not pushed closed, so use the NO connection to arduino for the closed state switch
  {

    digitalRead(Flapclosed);


  }   // endwhile flapclosed


  //Serial.println (" ++++++++++++++  Flap now closed +++++++++++++" );
  // Serial.println( "  end of shutter closed routine ");
  // The flap and shutter are now closed so set the relays back to initial status -

    Serial.println("---------------- END of CLOSE Process-------------------");
    Serial.println("");


  initialise_relays();  // TURN THE POWER OFF

} // end  CS




void open_process()
{


  // Open the flap first

  // Serial.println ("Waiting for Flap to open  " + String(digitalRead( Flapopen)));
 // flaprelay1count++;
      digitalWrite(FLAPRELAY1, LOW);             // retracting polarity - Flap opens first - the mechanics means that the actuator
      digitalWrite(FLAPRELAY2, HIGH);            // retracts in order to open the flap
	  
	    // debug prints below
	    Serial.println("----------------Open Process-------------------");
	    Serial.print("flap relay 1 (expect LOW)  ");
	    Serial.println( digitalRead(FLAPRELAY1));
	    Serial.print("flap relay 2 (expect HIGH) ");
	    Serial.println(digitalRead(FLAPRELAY2));

	    Serial.print("Flapvalue before while loop (Expect HIGH)" );
	    Serial.println(digitalRead(Flapopen));

// end debug

  while (digitalRead(Flapopen) == HIGH)         //high when not pushed closed, so use the NO connection to arduino for the open state switch
  {
// on the open brace - {digitalRead(FLAPRELAY1)}{digitalRead(FLAPRELAY2)}

    digitalRead(Flapopen);                     // will go LOW to signify flap is fully open i.e. the switch has ben pushed closed


  }

    // debug prints below
    Serial.print("Other side of while, Flapvalue (Expect LOW) " );
    Serial.println(digitalRead(Flapopen));
    Serial.println ("Flap should now be open wait 15 secs for shutter to open ");

    
  // Serial.println ("Flap now open  ");


  initialise_relays();  // TURN THE POWER OFF


  // then open the shutter (winch)


  int openrevcount = 0;

  
  
      digitalWrite(SHUTTERRELAY3, LOW);          // these two lines from version 2 - they set the motor direction
      digitalWrite(SHUTTERRELAY4, HIGH);
	  
motor_start_time = millis();

  while (openrevcount <= number_of_revs )
  {
  // on the open brace {digitalRead(SHUTTERRELAY3)}{digitalRead(SHUTTERRELAY4)}
  	 if ((millis() - motor_start_time) > motor_time_limit)   // don't let the motor run longer than defined time limit
  	 {
	 	  openrevcount = number_of_revs +1;           // kill the while loop
  	 }
    // now poll the limit switch for activations as the pulley rotates
    if (digitalRead(shutter_limit_switch) == LOW)   // the limit switch has been pressed by the rotating cam
    {
      delay(750);  // wait for the switch to open as the rotating cam moves on
      openrevcount++;


    }  //endif
  }  //end while

  initialise_relays();  // TURN THE POWER OFF


    Serial.println("---------------- END of Open Process-------------------");
    Serial.println("");
}// end  OS
