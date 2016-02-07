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

enum State { INIT, WAITFORINPUT, RFIDREAD, TIMEOUT, PROCESSING, FINISHED, ERROR } state;

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
  

	if (timer0 > timer0interval) {
		timer0 -= timer0interval; // reset the timer

		// just for testing of elapsedMillis
		int ledPin = digitalRead(led); // read the current state and write the opposite
		digitalWrite(led, !ledPin); // switch the LED
	}

	switch (state) {
	case INIT:		
		Serial.print("RFID reader");
		Serial.print("XXX"); // Send f.eks XBee adresse. Kan resolves mot fonuftig navn på server....
		Serial.print("power up.....");
		state = WAITFORINPUT;
		break;

		//Timers, Check And Update(T) (Er denne nødvendig ? )

		
		
	case WAITFORINPUT:
		// Look for input from RFID-chip, XBee, and switch

		if (Serial.available()) {
			state = RFIDREAD;
		}

		// Les XBee her....

		break;

	case RFIDREAD;
		//RFID Collect And Validate Input(T)
		while (Serial.available()) {
			//from: http://bildr.org/2011/02/rfid-arduino/
			int readByte = Serial.read(); //read next available byte

			if (readByte == 2) reading = true; //begining of tag
			if (readByte == 3) reading = false; //end of tag

			if (reading && readByte != 2 && readByte != 10 && readByte != 13) {
				//store the tag
				tagString[index] = readByte;
				index++;
			}
		}
		break;

		//RFID Check ID Against Database(T)
		//RFID Take Action On ID(T)
		//RFID Report ID And Action To Server(T)

		//XBee Collect And Validate Input(T)
		//XBee Update Local Database(T)
		//XBee Set Internal Clock(T) Nødvendig med klokke ?
		//XBee Set Door To Open Or Closed(T)
		//XBee Report Action Taken To Server(T)

		//Switch state of lock(T)

		//Action LEDs(T) (Should be done in several of the states above....)
		//Action Lock(T)

}
