/*
	Author bobness09, Mail bobnessdev@gmail.com
	
	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Mavlink2Walkera.h"

#include <SoftwareSerial.h>
#include <FastSerial.h>
#include "SimpleTelemetry.h"
#include "Mavlink.h"
#include "WalkeraTelemOut.h"
#include "SimpleFIFO.h"
#include <GCS_MAVLink.h>


#define HEARTBEATLED 13
#define HEARTBEATFREQ 500

// Do not enable both at the same time
#define DEBUG
//#define DEBUGWALKERA

// Comment this to run simple telemetry protocol
#define MAVLINKTELEMETRY

#ifdef MAVLINKTELEMETRY
Mavlink *dataProvider;
#else
SimpleTelemetry *dataProvider;
#endif

FastSerialPort0(Serial);

WalkeraTelem *walkera;
SoftwareSerial *walkeraSerial;
FastSerial *mavlinkSerial;

#ifdef DEBUG
SoftwareSerial *debugSerial;
#endif

SimpleFIFO<char, 128> queue;

int		counter = 0;
unsigned long	hbMillis = 0;
unsigned long	rateRequestTimer = 0;

unsigned long	lastSentWalkera = 0;

byte	hbState;
bool	firstParse = false;

void setup() {

// Debug serial port pin 11 rx, 12 tx
#ifdef DEBUG
	debugSerial = new SoftwareSerial(8, 9); // 9 tx, 8 rx
	debugSerial->begin(38400);
#endif

	// Walkera data port pin 6 rx, 5 tx
	walkeraSerial = new SoftwareSerial(6, 5);
	walkeraSerial->begin(38400);

	// Incoming data from APM
	mavlinkSerial = &Serial;
	mavlinkSerial->begin(57600);
	mavlinkSerial->flush();

#ifdef DEBUG
	debugSerial->println("Initializing Mavlink2Walkera 0.9");
	debugSerial->print("Free ram: ");
	debugSerial->print(freeRam());
	debugSerial->println(" bytes");
#endif

#ifdef MAVLINKTELEMETRY
	dataProvider = new Mavlink(mavlinkSerial);
#else
	dataProvider = new SimpleTelemetry();
#endif

	walkera = new WalkeraTelem();

	digitalWrite(HEARTBEATLED, HIGH);
	hbState = HIGH;

#ifdef DEBUG
	debugSerial->println("Waiting for APM to boot.");
#endif

	// Blink fast a couple of times to wait for the APM to boot
	for (int i = 0; i < 250; i++)
	{
		if (i % 2)
		{
			digitalWrite(HEARTBEATLED, HIGH);
			hbState = HIGH;
		}
		else
		{
			digitalWrite(HEARTBEATLED, LOW);
			hbState = LOW;
		}
		delay(50);
	}


#ifdef DEBUG
	debugSerial->println("Initialization done.");
	debugSerial->print("Free ram: ");
	debugSerial->print(freeRam());
	debugSerial->println(" bytes");
#endif
}

void loop() {

#ifdef MAVLINKTELEMETRY
	if( dataProvider->enable_mav_request || (millis() - dataProvider->lastMAVBeat > 5000) )
	{
		if(millis() - rateRequestTimer > 2000)
		{
			for(int n = 0; n < 3; n++)
			{
#ifdef DEBUG
				debugSerial->println("Making rate request.");
#endif
				dataProvider->makeRateRequest();
				delay(50);
			}

			dataProvider->enable_mav_request = 0;
			dataProvider->waitingMAVBeats = 0;
			rateRequestTimer = millis();
		}

	}
#endif

	while (mavlinkSerial->available() > 0)
	{
		if (queue.count() < 128)
		{
			char c = mavlinkSerial->read();
			queue.enqueue(c);
		}
		else
		{
#ifdef DEBUG
			debugSerial->println("QUEUE IS FULL!");
#endif
		}
	}

	processData();
	updateHeartbeat();

	sendWalkeraData();
}

void updateHeartbeat()
{
	long currentMilillis = millis();
	if(currentMilillis - hbMillis > HEARTBEATFREQ) {
		hbMillis = currentMilillis;
		if (hbState == LOW)
		{
			hbState = HIGH;
		}
		else
		{
			hbState = LOW;
		}
		digitalWrite(HEARTBEATLED, hbState);
	}
}

void sendWalkeraData()
{
    if ((millis() - lastSentWalkera) > 1000) {
		walkera->sendTelemetry(walkeraSerial, dataProvider);
		lastSentWalkera = millis();
	#ifdef DEBUG
		debugSerial->println("sendTelemetry");
		walkera->printValues(debugSerial, dataProvider);
	#endif
	}
}

void processData()
{
	while (queue.count() > 0)
	{
		bool done = dataProvider->parseMessage(queue.dequeue());

		if (done && !firstParse)
		{
			firstParse = true;
#ifdef DEBUG
			debugSerial->println("First parse done. Start sending on Walkera port.");
#endif
		}
	}
}


int freeRam () {
	extern int __heap_start, *__brkval;
	int v;
	return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}
