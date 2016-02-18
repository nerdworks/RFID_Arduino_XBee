/*
 Name:		RFID_Arduino_XBee.ino
 Created:	2/4/2016 3:19:12 PM
 Author:	joran.kvalvaag
*/

/*
Description:
Use one or more RFID Readers (ID-20LA (125 kHz)),
each connected to an Arduino Pro Mini as access control.
The Arduino's communicates with a centralized access control server using XBee.

The Arduino is connected to an electric deadbolt lock. See link below
### Make sure not to close the lock when the door is still open....

###############################################################################
// Hardware Description #######################################################
###############################################################################

Place only the RFID reader on the outside of the house
in case it's enclosure is broken into.
Keep the Arduino and XBee on the "inside".

One Ethernet cable or similar goes from the Arduino to the outside for
communication with the RFID chip,
and for controlling the diode, and to the exit button and the inside LED.

This code is written to be used with the following electric dead bolt lock:
http://udohow.en.made-in-china.com/product/lSjmtBYUbbcH/China-Electronic-Hook-Drop-Bolt-Lock.html

RFID chip connection to the Arduino:
RFID pin	-----		Arduino pin
1  GND		Brown		GND
2  RES
7  FORM
9  D0		Blue		8 (rxPin) (Cannot be changed when using Altsoftserial
11 VCC		Brown/W		5V
Also: RFID pin 1 (GND) is strapped to RFID pin 7 (FORM),
and RFID pin 11 (VCC) is strapped to RFID pin 2 (RES).


XBee Explorer connections to the Arduino:
XBee Explorer pin	-----		Arduino pin
GND					-----		GND
5V					-----		5V
DIN					-----		D0/TX
DOUT				-----		D1/RX


Door lock connections to Arduino:
Lock function	-----		Arduino pin
Solenoid relay	-----		D7
Door position	-----		D4
Lock position	-----		D2
GND				-----		GND 


Status LEDs connection to Arduino:
LED pin		-----		Arduino pin
Red pin		Blue/W		D6
Green pin	Green		D5
Blue pin	Green/W		D3
Cathode		Brown		GND


Exit button connection to Arduino
Button Pin	-----		Arduino pin
Pin 1		Orange		D12
Pin 2		Brown		GND

//The GND on the RFDI chip, the LEDs and the exit button can be joined.

LED function:
Uses two RGB LEDs in parallel. One on each side of the door.

Continuous green = 
	Door lock is continuously open as demanded from server.
	(Alarm off, and house is open)
Blinking green = 
	Door lock is temporarily open for passage.
Continuous red = 
	Door lock is continuously closed as demanded from server.
Blinking red = 
	Error.                    -- elaborate.... slow blink, fast blink etc...
Continuous blue = 
	?
Blinking blue = 
	Communication or validation is ongoing.

Uses one switch on the inside of the door as exit button ++.
Short press: 
	Open lock for passage when lock is closed. Send message to server.
Long press: 
	If alarm is on (house is locked);
	Send message to server to turn off alarm and open house. 
	If alarm is off (house is open);
	Send message to server to turn alarm on and lock house.


###############################################################################
// EEPROM Stuff ###############################################################
###############################################################################

A table of valid RFID numbers are stored in EEPROM.
This table is updated from the server.
Structure of table:
RFID tag number (10 HEX digits = 5 bytes)
	(keep CRC (1 byte) and EOL (1 byte) out of database)
Zone (16 zones @ 1 bit each = 2 bytes)

1024 / 7 = about 140 active cards. Use ext EEPROM if more cards are needed.
140 * 7 = 980. This leaves 44 bytes available for other data.

In addition the following variables are stored in EEPROM:
A unique ID for this node (64 bit XBee ID) =8 bytes //Fetch from the module
Zone for this physical reader =				2 bytes
int lockOpenTime =							1 byte
16 bit XBEE address for the server			2 bytes
64 bit XBee address for the server			8 bytes

Total additional bytes in EEPROM =			21 bytes out of 40....

Start and end addresses for the info stored in EEPROM:
Name			Start address	End address
*/

int EE_Database = 0; 			// 979	140 cards @ 7 bytes of RFID + 2 bytes for allowed zones for this card
int EE_UniqueID	= 980;			// 987	64 bit ID of connected XBee module
int EE_Zone	= 988;				// 990	Zone for this reader
int EE_lockOpenTime = 991;		// 991	Time to keep the lock open
int EE_16bitServerID = 992;		// 993	16 bit XBee address of server
int EE_64bitServerID = 994;		// 1001	64 bit XBee address of server


/*
###############################################################################
// Message protocol to server #################################################
###############################################################################

All messages must have a "$" as start byte
After the start byte ($) comes a message identification byte (capital letter).
Next comes the data, if applicable.
All messages must end with a "&".

-------------------------------------------------------------------------------
Message ID: A
Description:
	Door lock state change by use of RFID
Type and size of data:
	RFID (10 byte char array) + new state (boolean)
Example message:
	$A04A9FC65B30&
Comments:
	The server know which door based on senders 64 bit XBee.
	If the door was unlocked, the server will turn the alarm on.
-------------------------------------------------------------------------------
Message ID: B
Description:
	Door lock state change by use of exit button (short press)
Type and size of data:
	New state 0 or 1 (1 bit boolean)
Example message:
	$B1&
Comments:
	0 = unlocked, 1 = locked

-------------------------------------------------------------------------------
Message ID: C
Description:
	Alarm state change by use of exit button (long press)
Type and size of data:
	0 or 1 (1 bit boolean)
Example message:
	$C0&
Comments:
	

-------------------------------------------------------------------------------
Message ID: D
Description:
Type and size of data:
Example message:
Comments:

-------------------------------------------------------------------------------
Message ID: E
Description:
Type and size of data:
Example message:
Comments:

-------------------------------------------------------------------------------
Message ID: F
Description:
Type and size of data:
Example message:
Comments:

-------------------------------------------------------------------------------
Message ID: G
Description:
Type and size of data:
Example message:
Comments:

-------------------------------------------------------------------------------
Message ID: H
Description:
Type and size of data:
Example message:
Comments:

-------------------------------------------------------------------------------
Message ID: I
Description:
Type and size of data:
Example message:
Comments:

-------------------------------------------------------------------------------
Message ID: J
Description:
Type and size of data:
Example message:
Comments:

-------------------------------------------------------------------------------
Message ID: K
Description:
Type and size of data:
Example message:
Comments:

-------------------------------------------------------------------------------
Message ID: L
Description:
Type and size of data:
Example message:
Comments:

-------------------------------------------------------------------------------
Message ID: M
Description:
Type and size of data:
Example message:
Comments:




###############################################################################
// Message protocol from server ###############################################
###############################################################################

... Somewhat similar to the above....

--Alarm has been turned on, lock house //Reply with error if door is open
--Alarm has been turned off, open house.
--Update EEPROM database
--Reply to question for lock status
--reply to question for door status

....

*/


//#############################################################################
// Libraries ##################################################################
//#############################################################################

//Use button library to handle buttons and switches:
http://arduino-info.wikispaces.com/HAL-LibrariesUpdates
//Read about elapsedMillis here: 
http://www.forward.com.au/pfod/ArduinoProgramming/TimingDelaysInArduino.html
//Read about the EEPROMex library here:
http://thijs.elenbaas.net/2012/07/extended-eeprom-library-for-arduino/
//Use AltSoftSerial for communication with the RFID reader.
//Tx = Pin 9 (not used), Rx = Pin 8. With Altsoftserial PWM 10 = unusable.

#include <Altsoftserial.h>
#include <elapsedMillis.h>
#include <EEPROMex.h>
#include <Button.h>
#include <XBee.h>
#include <Printers.h>


//#############################################################################
// Create Objects #############################################################
//#############################################################################

AltSoftSerial RFIDSerial; //AltSoftSerial object for the RFID reader
Button exitButton = Button(12, PULLUP); //De-bounce buttons and switches
Button doorPositionSwitch = Button(4, PULLUP);
Button doorLockSwitch = Button(2, PULLUP);
XBee xbee = XBee(); // Create an XBee object 


//#############################################################################
// Declare variables, timers, etc #############################################
//#############################################################################

// just for testing of elapsedMillis
int led = 13; // Pin 13 has an LED connected on most Arduino boards.

// Set up states for the finite state machine
enum State { INIT, IDLE, RFID_READ, RFID_CHECK_TAG, ACTION_LOCK, TIMEOUT, PROCESSING, FINISHED, ERROR } state;
// Se på state maskin i eksempelbruk av elapsedMillis....
// og her: https://hackingmajenkoblog.wordpress.com/2016/02/01/the-finite-state-machine/

// Definition of global timers
elapsedMillis timer0; // Timer for x
#define timer0interval 1000 // the interval in ms 

//declare global variables
static char tagString[10]; //Last read RFID tag string
static boolean doorStatus = 1; // Current status of the door. 1=Closed, 0=open
static boolean lockStatus = 1; // Current status of the lock. 1=Locked, 0=open
static int lockOpenTime = 3; // The time to keep the lock open. From EEPROM
static boolean connXBee = 0; // Connected to XBee=1, connected to a comp=0.



//#############################################################################
// Setup ######################################################################
//#############################################################################
void setup() {

	state = INIT;

	Serial.begin(9600);			// Hardware serial connected to XBee module
	xbee.setSerial(Serial);		// Tell XBee to use Hardware Serial.
	RFIDSerial.begin(9600);		// Serial port for connection to RRID reader
	delay(1);

	// Detect if we are connected to a computer or the XBee.....
	Serial.print(+++); // kun i AT mode? Bedre å spørre etter firmvare-versjon?
	delay(200);
	if (Serial.available()) {
		byte check = Serial.read();
		if (check == 'O') {
			connXBee = 1;
		}
	}

	// Get lockOpenTime from EEPROM 
	lockOpenTime = EEPROM.readByte(EE_lockOpenTime); // Time to keep lock open.

	// Send startup message to server for logging.

	// just for testing of elapsedMillis
		pinMode(led, OUTPUT); // initialize the digital pin as an output.

		timer0 = 0; // clear the timer at the end of startup
}


//#############################################################################
// Loop #######################################################################
//#############################################################################
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

	//#########################################################################
	case INIT:
	//#########################################################################
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

	//#########################################################################	
	case IDLE: // Look for input from RFID-chip, XBee, and switch
	//#########################################################################

		if (RFIDSerial.available()) {
			state = RFID_READ;
			//Start blinking blue LED...
		}

		// Les XBee her....

		// se om noen timere er utgått her, og ta aksjon....
		// F.eks lås døra når lockOpenTime er ute


		break;

	//#########################################################################
	case RFID_READ: //RFID Collect And Validate Input(T)
	//#########################################################################
		
		boolean reading = false;
		int index = 0;
		while (RFIDSerial.available()) {
			//from: http://bildr.org/2011/02/rfid-arduino/
			int readByte = Serial.read(); //read next available byte

			if (readByte == 2) reading = true; //beginning of tag
			if (readByte == 3) reading = false; //end of tag

			if (reading && readByte != 2 && readByte != 10 && readByte != 13) {
				// _Dette må ryddes for å lagre kun 10 chars.... Lagre CRC for seg selv, kutt ut EOL osv.
				// Se f.eks: http://playground.arduino.cc/Code/ID12
				//store the tag
				tagString[index] = readByte;
				index++;
				//Continue blinking blue LED
			}
		}

		state = RFID_CHECK_TAG;

		break;

	//#########################################################################
	case RFID_CHECK_TAG: //RFID Check ID Against Database(T)
	//#########################################################################
		
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

	//#########################################################################
	case ACTION_LOCK: //Action Lock(T)
	//#########################################################################
		
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

			// Forskjellige typer lås må håndteres.NO, NC osv.
			// Noen lås har tilbakemelding....
			// F.eks: com  o------|
			//        open o--\___|
			//      closed o--

		break;


		

		//XBee Collect And Validate Input(T)
		//XBee Update Local Database(T)
		//XBee Set Internal Clock(T) Nødvendig med klokke ?
		//XBee Set Door To Open Or Closed(T)
		//XBee Report Action Taken To Server(T)

		//Action LEDs(T) (Should be done in several of the states above....)
		
		//Handle switches and buttons. Indoor manual exit button, deadbolt position switch and door position switch.

}
