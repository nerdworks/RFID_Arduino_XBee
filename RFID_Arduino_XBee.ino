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

*/


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

}
