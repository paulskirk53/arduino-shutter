/*December 2018 
Changes to limit switches coded in this version
The one switch is now operated from the periphery of the wheel and the code counts the number of revolutions to open state/ closed state
The flap is unaffected.
November 2018
It is only possible to transmit a char array[32] max size using radio.write. No other data format works
*/

/*
pseudo code open shutter

String last_state = "closed";
const int number_of_revs = 5;   // set this empirically depending upon number of turns of the winch required to open / close the shutter

if last_state = closed"; 
{
  int revcount = 0;
                     
  while last_state = closed
  {
      digitalWrite(SHUTTERRELAY3, LOW);          // these two lines from version 2 - they set the motor direction
      digitalWrite(SHUTTERRELAY4, HIGH);
	  // now poll the limit switch for activations as the pulley rotates
	  if (digitalRead(shutter_limit_switch) = high)
	  {
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

pseudo code close shutter

String last_state;
const int number_of_revs = 5;   // set this empirically depending upon number of turns of the winch required to open / close the shutter

if last_state = open";
{
	int revcount = 0;
	
	while last_state = open
	{
		    digitalWrite(SHUTTERRELAY3, HIGH);          // close POLARITY
		    digitalWrite(SHUTTERRELAY4, LOW);
		// now poll the limit switch for activations as the pulley rotates
		if (digitalRead(shutter_limit_switch) = high)
		{
			revcount++;               //
			if (revcount >= number_of_revs)
			{
				last_state = "closed";
				initialise_relays();  // TURN THE POWER OFF
				shutterstatus=true;  // this variable is set in the V2 code and the shutter status routine will need a small change
			}
		}
	}

}

*/