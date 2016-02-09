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
RFID-tag-number (12 chars + EOL = 13 bytes)	|	Zone (16 zones @ 1 bit each = 2 bytes)
1024 / 15 = about 65 active cards. Use external EEPROM if more cards are needed.

//In addition the following variables are stored in EEPROM:
//A unique ID (RFID node ID) 
//Zone for this physical reader = 2 bytes
//int lockOpenTime = 1 byte

Start and end addresses for the info stored in EEPROM:


*/

//Forward door position and lock position to server.
//See lock: http://udohow.en.made-in-china.com/product/lSjmtBYUbbcH/China-Electronic-Hook-Drop-Bolt-Lock.html



// Se på state maskin i eksempelbruk av elapsedMillis....
// og her: https://hackingmajenkoblog.wordpress.com/2016/02/01/the-finite-state-machine/

/*
State machine states (Static and Transitional):
Timers, Check And Update (T) (Er denne nødvendig?)
Wait for input form RFID, XBee or switch  (S)
	
RFID Collect And Validate Input (T)
RFID Check ID Against Database (T)

XBee Collect And Validate Input (T)
XBee Update Local Database (T)
XBee Set Internal Clock (T) Nødvendig med klokke?
XBee Set Door To Open Or Closed (T)
XBee Report Action Taken To Server (T)

Action Lock (T)

Action LEDs (T) (Should be done in several of the states above....)
Action Lock (T) (Er denne nødvendig?)
// Forskjellige typer lås må håndteres. NO, NC osv.
// Noen lås har tilbakemelding....
// F.eks: com  o------|
//        open o--\___|
//      closed o--
*/

enum State { INIT, IDLE, RFID_READ, RFID_CHECK_TAG, ACTION_LOCK, TIMEOUT, PROCESSING, FINISHED, ERROR } state;

// Read about elapsedMillis here:
// http://www.forward.com.au/pfod/ArduinoProgramming/TimingDelaysInArduino.html


#include <SoftwareSerial.h>
#include <elapsedMillis.h>
#include <EEPROMex.h>


// Create a software serial object for the connection to the XBee module
#define rxPin 2
#define txPin 3
SoftwareSerial xbee = SoftwareSerial(rxPin, txPin);


// just for testing of elapsedMillis
int led = 13; // Pin 13 has an LED connected on most Arduino boards.

// Definition of all global timers
elapsedMillis timer0; // Timer for x
#define timer0interval 1000 // the interval in mS 

//declare global variables
char tagString[13]; //Last read RFID tag string
boolean lockStatus = 1; // Current status of the lock. 1=Locked, 0=open
int lockOpenTime = 0; // Read from EEPROM


void setup() {
	state = INIT;

	//RFID reader
	Serial.begin(9600);
	//End RFID reader

	//XBee reader serial comms
	xbee.begin(9600);      // Serial port for connection to XBee module
	
	// Get lockOpenTime from EEPROM (Can be updated from server)


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

		state = IDLE;
		break;

	//case nnn:
		//Timers, Check And Update(T) (Er denne nødvendig ? )
		//break;

		
	//###############################################################	
	case IDLE:
		// Look for input from RFID-chip, XBee, and switch

		if (Serial.available()) {
			state = RFID_READ;
			//Start blinking blue LED...
		}

		// Les XBee her....

		// se om noen timere er utgått her, og ta aksjon....
		// F.eks lås døra når lockOpenTime er ute


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

		state = RFID_CHECK_TAG;

		break;

	//###############################################################
	case RFID_CHECK_TAG:
		//RFID Check ID Against Database(T)
			//Continue blinking blue LED
		// See: http://thijs.elenbaas.net/2012/07/extended-eeprom-library-for-arduino
		// regarding use of EEPROM

		//If tagString is found in EEPROM and zone for this reader matches zone-bit for this tagString
		//	state = ACTION_LOCK
		//	Report valid ID to server for logging

		//else
		//	blink red LED
		//	report invalid tag to server for logging
		//	state = IDLE;

		//clear the char array by filling with null - ASCII 0. Will think same tag has been read otherwise.
		//See: http://bildr.org/2011/02/rfid-arduino/


		break;

		//###############################################################
	case ACTION_LOCK:
		//Action LOCK(T)
		//if current state of lock is unlocked, switch to locked, and send message to server.
		//
		//if current state of lock is locked, switch to unlocked for a while (and send message to server). Then locked again. Blink green while open.

		if (lockStatus == 1) {
			//if lockStatus == 1 
				//Blink unlocked
				//set lockStatus to 0
				//Unlock door
				//Get lockOpenTime from EEPROM (Can be updated from server)				
				//start lockOpenTimer and keep lock open for lockOpenTime
		}

		//if door is supposed to be locked, as demanded from server. Signal if door is left open (and unlocked).


		break;


		

		//XBee Collect And Validate Input(T)
		//XBee Update Local Database(T)
		//XBee Set Internal Clock(T) Nødvendig med klokke ?
		//XBee Set Door To Open Or Closed(T)
		//XBee Report Action Taken To Server(T)

		//Switch state of lock(T)

		//Action LEDs(T) (Should be done in several of the states above....)
		//Action Lock(T)

}
