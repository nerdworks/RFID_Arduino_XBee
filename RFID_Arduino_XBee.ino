/*
 Name:		RFID_Arduino_XBee.ino
 Created:	2/4/2016 3:19:12 PM
 Author:	joran.kvalvaag
*/

// Se på state maskin i eksempelbruk av elapsedMillis....
// og her: https://hackingmajenkoblog.wordpress.com/2016/02/01/the-finite-state-machine/


// Read about elapsedMillis here:
// http://www.forward.com.au/pfod/ArduinoProgramming/TimingDelaysInArduino.html
#include <elapsedMillis.h>

// just for testing
int led = 13; // Pin 13 has an LED connected on most Arduino boards.

// Define timers
elapsedMillis timer0;
#define timer0interval 1000 // the interval in mS 


void setup() {

	
	pinMode(led, OUTPUT); // initialize the digital pin as an output.
	timer0 = 0; // clear the timer at the end of startup
}

// the loop function runs over and over again until power down or reset
void loop() {
  

	if (timer0 > timer0interval) {
		timer0 -= timer0interval; // reset the timer
		int ledPin = digitalRead(led); // read the current state and write the opposite
		digitalWrite(led, !ledPin); // switch the LED
	}

}
