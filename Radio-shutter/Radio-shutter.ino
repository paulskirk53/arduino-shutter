
#include <SoftwareSerial.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

//for nrf to work, pin 10 must be high if it is not used as an nrf connecton
#define PIN10  10


RF24 radio(7, 8); // CE, CSN


const byte address[6] = "00002";            // 00001 the address of this arduino board/ transmitter

double message = 20.0;

void setup() 
{

  pinMode(PIN10, OUTPUT);                 // this is an NRF24L01 requirement if pin 10 is not used
  Serial.begin(9600);

  radio.begin();


  radio.openReadingPipe(1, address);    // the 1st parameter can be any number 0 to 5
  // but 0 is used for write apparently

  // enable ack payload - slaves reply with data using this feature
  radio.enableAckPayload();

  radio.setPALevel(RF24_PA_MIN);
  radio.enableDynamicPayloads();
  radio.startListening();
  // radio.writeAckPayload(1, &message, sizeof(message));


}

void loop() 
{

  if (radio.available())
  {
    char text[32] = "";             // used to store what the master node sent e.g AZ hash SA hash
    radio.writeAckPayload(1, &message, sizeof(message));
    radio.read(&text, sizeof(text));

    Serial.print("The text received from Mster was: ");
    Serial.println(text);
    Serial.print("this is the shutter node - the value of message is ");
    Serial.println(message);
    Serial.println("--------------------------------------------------");

    if (message < 30 )
    {
      message++;
    }
    else
    {
      message = 20;
    }
  } //endif radio available




} // end void loop
