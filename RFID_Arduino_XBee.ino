/*
 Name:		RFID_Arduino_XBee.ino
 Created:	2/4/2016 3:19:12 PM
 Author:	joran.kvalvaag
*/

/*
Description:
Use one or more RFID Readers (ID-20LA (125 kHz)), each connected to an Arduino as access control. The Arduino's communicates with a centralized access control server using XBee.

Uses two RGB LEDs (in parallel?). One on each side of the door.
Continuous green = Door lock is continuously open as demanded from server.
Blinking green = Door lock is temporarily open for passage.
Continuous red = Door lock is continuously closed as demanded from server.
Blinking red = Error. -- elaborate.... slow blink, fast blink etc...
Continuous blue = 
Blinking blue = Communication or validation is ongoing.

Uses one switch on the inside.
Short press: Open lock for passage when lock is closed. Close lock if open.
Long press: ?

A table of valid RFID numbers are stored in EEPROM. This table can be updated from the server.

Structure of table:
RFID-tag-number (12 chars = 12 bytes)	|	Zone (16 zones @ 1 bit each = 2 bytes)

1024 / 14 = max 70 active cards. Use external EEPROM if more cards are needed.

I addition a unique ID (RFID node ID) and zone for this reader is stored in EEPROM.....



*/

// Se på state maskin i eksempelbruk av elapsedMillis....
// og her: https://hackingmajenkoblog.wordpress.com/2016/02/01/the-finite-state-machine/

/*
State machine states (Static and Transitional):
Timers, Check And Update (T) (Er denne nødvendig?)
Wait for input form RFID, XBee or switch  (S)
	
RFID Collect And Validate Input (T)
RFID Check ID Against Database (T)
RFID Take Action On ID (T)
RFID Report ID And Action To Server (T)

XBee Collect And Validate Input (T)
XBee Update Local Database (T)
XBee Set Internal Clock (T) Nødvendig med klokke?
XBee Set Door To Open Or Closed (T)
XBee Report Action Taken To Server (T)

Switch state of lock (T)

Action LEDs (T) (Should be done in several of the states above....)
Action Lock (T) (Er denne nødvendig?)
// Forskjellige typer lås må håndteres. NO, NC osv.
// Noen lås har tilbakemelding....
// F.eks: com  o------|
//        open o--\___|
//      closed o--
*/

enum State { INIT, WAITFORINPUT, RFID_READ, RFID_CHECK_TAG, RFID_ACTION_VALID_ID, TIMEOUT, PROCESSING, FINISHED, ERROR } state;

// Read about elapsedMillis here:
// http://www.forward.com.au/pfod/ArduinoProgramming/TimingDelaysInArduino.html


#include <SoftwareSerial.h>
#include <elapsedMillis.h>
#include <EEPROMex.h>


// just for testing of elapsedMillis
int led = 13; // Pin 13 has an LED connected on most Arduino boards.

// Definition of all global timers
elapsedMillis timer0; // Timer for x
#define timer0interval 1000 // the interval in mS 

//declare global variables
char tagString[13]; //Last read RFID tag string
boolean lockStatus = 1;
int RFIDResetPin = 13;


void setup() {
	state = INIT

		//RFID reader
		Serial.begin(9600);
		pinMode(RFIDResetPin, OUTPUT);
		digitalWrite(RFIDResetPin, HIGH);
	//End RFID reader

	//XBee reader

	//End XBee reader


	// just for testing of elapsedMillis
		pinMode(led, OUTPUT); // initialize the digital pin as an output.
		timer0 = 0; // clear the timer at the end of startup
}

// the loop function runs over and over again until power down or reset
void loop() {

	//Declare variables valid for one loop
	

	//Timer example
	if (timer0 > timer0interval) {
		timer0 -= timer0interval; // reset the timer

		// just for testing of elapsedMillis
		int ledPin = digitalRead(led); // read the current state and write the opposite
		digitalWrite(led, !ledPin); // switch the LED
	}
	//Timer example end

	switch (state) {

	//###############################################################
	case INIT:
		//Move INIT to setup()?
		Serial.print("RFID reader");
		Serial.print("XXX"); // Send f.eks XBee adresse. Kan resolves mot fonuftig navn på server....
		Serial.print("power up.....");

		// Ask server if current state of lock should be locked or unlocked.
		// If no answer, set to locked.

		state = WAITFORINPUT;
		break;

	//case nnn:
		//Timers, Check And Update(T) (Er denne nødvendig ? )
		//break;

		
	//###############################################################	
	case WAITFORINPUT:
		// Look for input from RFID-chip, XBee, and switch

		if (Serial.available()) {
			state = RFID_READ;
			//Start blinking blue LED...
		}

		// Les XBee her....

		break;

	//###############################################################
	case RFID_READ:
		//RFID Collect And Validate Input(T)
		boolean reading = false;
		int index = 0;
		while (Serial.available()) {
			//from: http://bildr.org/2011/02/rfid-arduino/
			int readByte = Serial.read(); //read next available byte

			if (readByte == 2) reading = true; //beginning of tag
			if (readByte == 3) reading = false; //end of tag

			if (reading && readByte != 2 && readByte != 10 && readByte != 13) {
				//store the tag
				tagString[index] = readByte;
				index++;
				//Continue blinking blue LED
			}
		}

		//Reset the RFID reader to read again.
		digitalWrite(RFIDResetPin, LOW);
		digitalWrite(RFIDResetPin, HIGH);
		delay(150); // erstattes med en timer....

		state = RFID_CHECK_TAG;

		break;

	//###############################################################
	case RFID_CHECK_TAG:
		//RFID Check ID Against Database(T)
			//Continue blinking blue LED
		// See: http://thijs.elenbaas.net/2012/07/extended-eeprom-library-for-arduino
		// regarding use of EEPROM

		//If tagString is found in EEPROM and zone for this reader matches zone-bit for this tagString
		//	state = RFID_ACTION_VALID_ID
		//	Report valid ID to server for logging

		//else
		//	blink red LED
		//	report invalid tag to server for logging
		//	state = WAITFORINPUT;

		//clear the char array by filling with null - ASCII 0. Will think same tag has been read otherwise.
		//See: http://bildr.org/2011/02/rfid-arduino/


		break;

		//###############################################################
	case RFID_ACTION_VALID_ID:
		//RFID Take Action On ID(T)
		//if current state of lock is open, switch to locked, and vise versa.





		

		//XBee Collect And Validate Input(T)
		//XBee Update Local Database(T)
		//XBee Set Internal Clock(T) Nødvendig med klokke ?
		//XBee Set Door To Open Or Closed(T)
		//XBee Report Action Taken To Server(T)

		//Switch state of lock(T)

		//Action LEDs(T) (Should be done in several of the states above....)
		//Action Lock(T)

}
