//to do pin 11 is no longer used and pin 9 is now shutter_limit_switch 
//change the arduino board connections to the terminal block to reflect this and then delete these lines
//
//change text to command_from_master to improve code undestanding
// this routine receives commands from the radio master arduino - OS# CS# and SS#
// data is only returned by SS# - if the shutter is open return char message 'open' or 'closed'
//
//

#include <SoftwareSerial.h>
#include <SPI.h>
#include <nRF24L01.h>               // shown to be not required
#include <RF24.h>

//for nrf to work, pin 10 must be high if it is not used as an nrf connecton
#define PIN10  10

RF24 radio(7, 8); // CE, CSN

// pin definitions for shutter relays

// These data pins link to  Relay board pins IN1, IN2, in3 and IN4
#define FLAPRELAY1  4      // arduino  pin 4
#define FLAPRELAY2  5      // arduino  pin 5
#define SHUTTERRELAY3  6      // etc
#define SHUTTERRELAY4  3     // 

// shutter microswitches

// #define Shutteropen    11    //no longer required but still used until both the OPEN AND CLOSE routines are modelled - only one done so far
// #define Shutterclosed  9
#define Flapopen       12
#define Flapclosed     13
#define shutter_limit_switch  9             // change this to the right pin


const byte thisaddress[6] =       "shutt";            // 00002 the address of this arduino board/ transmitter
const byte masterNodeaddress[6] = "mastr";
char message[10] = "";
String receivedData;
bool shutterstatus = true;
String last_state = "closed";
const int number_of_revs = 5;   // set this empirically depending upon number of turns of the winch required to open / close the shutter

void setup()
{

  pinMode(PIN10, OUTPUT);                 // this is an NRF24L01 requirement if pin 10 is not used
  Serial.begin(9600);

  radio.begin();
  radio.setChannel(100);
  radio.setDataRate(RF24_250KBPS);  // set RF datarate

  // enable ack payload - slaves reply with data using this feature
  radio.enableAckPayload();

  radio.setPALevel(RF24_PA_LOW);
  radio.enableDynamicPayloads();
  radio.setRetries(15, 15);


  radio.openReadingPipe(1, thisaddress);    // the 1st parameter can be any number 1 to 5 the master routine uses 1
  radio.startListening();

  // ALL THE RELAYS ARE ACTIVE LOW, SO SET THEM ALL HIGH AS THE INITIAL STATE
  // Then define the pin modes. This avoids pins being low (activates realays) on power reset.

  initialise_relays();      // sets all the relay pins HIGH

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



}  // end setup

void loop()
{
  //  already done in setup radio.openReadingPipe(1, thisaddress);    // 00002
 // radio.startListening();

  while (!radio.available())
  {
    //do nothing
  }


  if (radio.available())
  {
    char text[32] = "";             // used to store what the master node sent e.g AZ hash SA hash
    //shutter_status();

    radio.read(&text, sizeof(text));


    Serial.println("this is the shutter node  ");
    Serial.print("The text received from Master was: ");
    Serial.println(text);


    if (text[0] == 'C' && text[1] == 'S' && text[2] == '#') // close shutter command
    {
      //Serial.print ("received CS");
      close_shutter();

    }


    if (text[0] == 'O' && text[1] == 'S' && text[2] == '#') // open shutter command
    {
      //Serial.print ("received OS");
      open_shutter();

    }
    if (text[0] == 'S' && text[1] == 'S' && text[2] == '#') //  shutter status command
    {

      shutter_status();

      Serial.println("the value of shutter status is ");
      Serial.print(message);
      Serial.println("--------------------------------------------------");


      radio.openWritingPipe(masterNodeaddress);
      radio.stopListening();

      delay(50);   // to allow the master to change from tx to rx
      bool rslt =       radio.write(&message, sizeof(message));
      radio.startListening();          //straight away after write to master, in case anothe message is sent

      if (rslt)
    {
      Serial.println("result of shutter Tx was true");
      }
      else
      {
        Serial.println("result of shutter Tx was error");
      }

      for ( int i = 0; i < 10; i++)                //initialise the message array back to nulls
    {
      message[i] = 0;
      }                                           //end for
    }                                             //endif SS

    text[0] = 0;   // set to null character
    text[1] = 0;
    text[2] = 0;

  } //endif radio available

} // end void loop

void initialise_relays()
{
  //  Serial1.println( "  Initialising relays ");
  digitalWrite(FLAPRELAY1, HIGH);
  digitalWrite(FLAPRELAY2, HIGH);
  digitalWrite(SHUTTERRELAY3, HIGH);
  digitalWrite(SHUTTERRELAY4, HIGH);
}

void close_shutter()
{
  // commands to close shutters
  // commands to close shutters reverse POLARITY TO BOTH motors
 
 
 
 if (last_state = "open");
 {
	 int revcount = 0;
	 
	 while (last_state = "open")
	 {
		digitalWrite(SHUTTERRELAY3, HIGH);          // closing POLARITY shutter - closes first
		digitalWrite(SHUTTERRELAY4, LOW);
		 // now poll the limit switch for activations as the pulley rotates
		 if (digitalRead(shutter_limit_switch) == LOW)  // the limit switch has been pressed by the rotating cam
		 {
		 delay(3000);  // wait for the switch to open as the rotating cam moves on
			 revcount++;
			 if (revcount >= number_of_revs)
			 {
				 last_state = "closed";
				 initialise_relays();  // TURN THE POWER OFF
				 shutterstatus=false;  // this variable is set in the V2 code and the shutter status routine will need a small change
			 }     // endif revcount

		 }   //  endif digital read

	 }  // endwhile

 }


  initialise_relays();  // TURN THE POWER OFF


  while (digitalRead(Flapclosed) == HIGH)       //high when not pushed closed, so use the NO connection to arduino for the closed state switch
  {
    digitalWrite(FLAPRELAY1, HIGH);          // EXTENDING POLARITY - Flap CLOSES second - the way the mechanics works is that the
    digitalWrite(FLAPRELAY2, LOW);           // linear actuator has to extend to close the flap
    digitalRead(Flapclosed);
    Serial.println (" Waiting for flap to close " + String(digitalRead( Flapclosed)));
   
  }   // endwhile flapclosed
  

  receivedData = "";

  // The flap and shutter are now closed so set the relays back to initial status -

  initialise_relays();  // TURN THE POWER OFF

} // end  CS


void open_shutter()
{
  // //there may be a problem if the SHUTTER is not open by the time the flap relay actuates
  //

  // commands to open shutters OPENING POLARITY TO BOTH motors note active LOW ********************************

  // Open the flap first

  //      Serial1.println("receivedData = " + receivedData);     // prints to HC06
  while (digitalRead(Flapopen) == HIGH)         //high when not pushed closed, so use the NO connection to arduino for the open state switch
  {
    digitalWrite(FLAPRELAY1, LOW);             // retracting polarity - Flap opens first - the mechanics means that the actuator
    digitalWrite(FLAPRELAY2, HIGH);            // retracts in order to open the flap
    digitalRead(Flapopen);                     // will go LOW to signify flap is fully open i.e. the switch has ben pushed closed
    Serial.println ("Waiting for Flap to open LOW HIGH  " + String(digitalRead( Flapopen)));
   
  }
  initialise_relays();  // TURN THE POWER OFF


  // then open the shutter

  if (last_state = "closed");
  {
	  int revcount = 0;
	  
	  while (last_state = "closed")
	  {
		  digitalWrite(SHUTTERRELAY3, LOW);          // these two lines from version 2 - they set the motor direction
		  digitalWrite(SHUTTERRELAY4, HIGH);
		  // now poll the limit switch for activations as the pulley rotates
		  if (digitalRead(shutter_limit_switch) == LOW)   // the limit switch has been pressed by the rotating cam
		  {
		  delay(3000);  // wait for the switch to open as the rotating cam moves on
			  revcount++;
			  if (revcount >= number_of_revs)
			  {
				  last_state = "open";
				  initialise_relays();  // TURN THE POWER OFF
				  shutterstatus=false;  // this variable is set in the V2 code and the shutter status routine will need a small change
			  }
		  }
	  }

  }

  receivedData = "";

}// end  OS

void shutter_status()
{
  //bool shutterstatus = true;
  //shutterstatus = digitalRead(Shutterclosed);   // now set in shutter open and close routines
  if (shutterstatus == true)
  {
    message [0] = 'C';
    message [1] = 'L';
    message [2] = 'O';
    message [3] = 'S';
    message [4] = 'E';
    message [5] = 'D';
    message [6] = '#';
    //Serial.println( "closed#");  // return closed to driver modded for radio
    //Serial.println("the value of shutter status is");
    //  Serial.println(message);
    //    Serial.println("--------------------------------------------------");

  }
  else
  {
    //Serial.println( "open#");   // return open to driver  modded for radio
    message [0] = 'O';
    message [1] = 'P';
    message [2] = 'E';
    message [3] = 'N';
    message [4] = '#';
    message [5] = 0;
    message [6] = 0;
    //Serial.println("the value of shutter status is");
    //  Serial.println(message);
    //    Serial.println("--------------------------------------------------");
  }


}
