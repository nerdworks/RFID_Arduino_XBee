/*
 Name:		RFID_Arduino_XBee.ino
 Created:	2/4/2016 3:19:12 PM
 Author:	joran.kvalvaag
*/

/*
Description:
Use one or more RFID Readers (ID-20LA (125 kHz)), each connected to an Arduino as access control. The Arduino's communicates with a centralized access control server using XBee.

The Arduino is connected to an electric deadbolt lock. See link below
### Make sure not to close the lock when the door is still open....

Put only the RFID reader on the outside of the house in case it's enclosure is broken into. Keep the Arduino and XBee on the "inside".

One ethernet cable goes from the inside to the outside for comms with the RFID chip, and for controlling the diode.

RFID chip connection to the Arduino:

RFID pin				Arduino pin
1  GND		-----		GND
2  RES
7  FORM
9  D0		-----		8 (rxPin)
11 VCC		-----		5V

In addition:
RFID pin 1 (GND) is strapped to RFID pin 7 (FORM)
RFID pin 11 (VCC) is strapped to RFID pin 2 (RES)


Uses two RGB LEDs (in parallel?). One on each side of the door.
Continuous green = Door lock is continuously open as demanded from server. (Alarm off, and house is open)
Blinking green = Door lock is temporarily open for passage.
Continuous red = Door lock is continuously closed as demanded from server.
Blinking red = Error. -- elaborate.... slow blink, fast blink etc...
Continuous blue = 
Blinking blue = Communication or validation is ongoing.

Uses one switch on the inside.
Short press: Open lock for passage when lock is closed.
Long press: If alarm on (house is locked); turn off alarm and open house. If alarm off (house is open); lock hose and turn alarm on. Message to server of course!

A table of valid RFID numbers are stored in EEPROM. This table is updated from the server.
Structure of table:
RFID-tag-number (10 char ID = 10 bytes) (keep CRC (2 bytes) and EOL (1 byte) out of database)	|	Zone (16 zones @ 1 bit each = 2 bytes)
1024 / 12 = about 83 active cards. Use external EEPROM if more cards are needed.
83 * 12 = 996. This leaves 28 bytes available for other data

//In addition the following variables are stored in EEPROM:
//A unique ID for this node (RFID node ID or 16 bit XBee ID?) =	16 bytes
//Zone for this physical reader =								2 bytes
//int lockOpenTime =											1 byte
Total additional bytes in EEPROM =								19 bytes out of 28....

Start and end addresses for the info stored in EEPROM:
Name			Start address	End address
Database	
Unique ID	
Zone	
lockOpenTime	

*/

//Forward door position and lock position to server.
//See lock: http://udohow.en.made-in-china.com/product/lSjmtBYUbbcH/China-Electronic-Hook-Drop-Bolt-Lock.html
//



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

//Use Altsoftserial.... Tx = Pin 9, Rx = Pin 8 (PWM 10 = unusable)
#include <AltSoftSerial.h>
#include <elapsedMillis.h>
#include <EEPROMex.h>


// Create an Altsoftserial object for the connection to the XBee module
AltSoftSerial xbeeSerial;


// just for testing of elapsedMillis
int led = 13; // Pin 13 has an LED connected on most Arduino boards.

// Definition of all global timers
elapsedMillis timer0; // Timer for x
#define timer0interval 1000 // the interval in mS 

//declare global variables
char tagString[10]; //Last read RFID tag string
boolean lockStatus = 1; // Current status of the lock. 1=Locked, 0=open
int lockOpenTime = 0; // Read from EEPROM


void setup() {
	state = INIT;

	Serial.begin(9600);			// RFID Reader
	xbeeSerial.begin(9600);		// Serial port for connection to XBee module
	
	// Get lockOpenTime from EEPROM 


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

			if (reading && readByte != 2 && readByte != 10 && readByte != 13) { // _Dette må ryddes for å lagre kun 10 chars.... Lagre CRC for seg selv, kutt ut EOL osv.
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
		// See: http://thijs.elenbaas.net/2012/07/extended-eeprom-library-for-arduino regarding use of EEPROM

		// If tagString is found in EEPROM and zone for this reader matches zone-bit for this tagString
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
		//Handle switches. Indoor manual switch, deadbolt position switch and door position switch.

}
