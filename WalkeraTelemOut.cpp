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
#include "WalkeraTelemOut.h"

//#define DEBUG
//#define DEBUGFAKEVALS

WalkeraTelem::WalkeraTelem()
{
	devoPacket.header = DEVOM_SYNC_BYTE;
    devoPacket.crc8   = 0x00;
    for (int i = 0; i < sizeof(devoPacket.data); i++)
    	 devoPacket.data[i] = 0x00;
}

WalkeraTelem::~WalkeraTelem(void)
{
}

void WalkeraTelem::sendWalkeraBuffer(SoftwareSerial* serialPort)
{
	devoPacket.crc8 = 0; // Init Checksum with zero Byte
	writeToPort(serialPort, devoPacket.header);
	for (int i = 0; i < sizeof(devoPacket.data); i++) {
		writeToPort(serialPort, devoPacket.data[i]);
	}
	writeToPort(serialPort, devoPacket.crc8); // Write Checksum to serial

#ifdef DEBUG
	 debugSerial->println();
#endif
}

void WalkeraTelem::writeToPort(SoftwareSerial* serialPort, uint8_t writeByte)
{
	serialPort->write(writeByte);
	devoPacket.crc8 += writeByte; // Add Checksum

#ifdef DEBUG
	debugSerial->print(writeByte, HEX);
	debugSerial->print(" ");
#endif
}

void WalkeraTelem::sendTelemetry(SoftwareSerial* serialPort, ITelemDataProvider* dataProvider)
{
	float gpsLatitude;
	float gpsLongitude;
	int32_t gpsAltitude; // Altitude in CM.

	uint16_t gpsGroundSpeed;
	uint16_t unknownVal1;
	uint16_t batteryVoltage;

#ifndef DEBUGFAKEVALS
	gpsLatitude    = dataProvider->getLatitude() / 100.0f;
	gpsLongitude   = dataProvider->getLongitud() / 100.0f;

	gpsAltitude    = round(dataProvider->getGpsAltitude()); // is already cm!
	gpsGroundSpeed = (dataProvider->getGpsGroundSpeed() * 100.0f);  // * 100 for cm

	unknownVal1    = 0;
	batteryVoltage = round(dataProvider->getMainBatteryVoltage() * 1000);
#else

	gpsLatitude    = 51.49257f; // = 51°29.3325 N
	gpsLongitude   = 7.45174f;  // =  7°27.0626 E

	gpsAltitude    = 20000; // 200m
	gpsGroundSpeed = 4567;  // 12,3m/s // Speed only works, when coordinates are set!

	unknownVal1    = 555;
	batteryVoltage = 12600; // 12.6V
#endif

	if ((gpsLatitude < -180.0f) || (gpsLatitude > 180.0f))
		gpsLatitude = 0.0f;

	if ((gpsLongitude < -90.0f) || (gpsLongitude > 90.0f))
		gpsLongitude = 0.0f;

	int32_t lat = round(gpsLatitude  * 10000000.0f);
	int32_t lon = round(gpsLongitude * 10000000.0f);

	devoPacket.data[0]  = lsByte(lat);
	devoPacket.data[1]  = s8Byte(lat);
	devoPacket.data[2] = s16Byte(lat);
	devoPacket.data[3] = s24Byte(lat);

	devoPacket.data[4]  = lsByte(lon);
	devoPacket.data[5]  = s8Byte(lon);
	devoPacket.data[6] = s16Byte(lon);
	devoPacket.data[7] = s24Byte(lon);

	devoPacket.data[8]  = lsByte(gpsAltitude);
	devoPacket.data[9]  = s8Byte(gpsAltitude);
	devoPacket.data[10] = s16Byte(gpsAltitude);
	devoPacket.data[11] = s24Byte(gpsAltitude);

	devoPacket.data[12] = lsByte(gpsGroundSpeed);
	devoPacket.data[13]	= s8Byte(gpsGroundSpeed);

	devoPacket.data[14] = lsByte(unknownVal1);
	devoPacket.data[15]	= s8Byte(unknownVal1);

	devoPacket.data[16] = lsByte(batteryVoltage);
	devoPacket.data[17] = s8Byte(batteryVoltage);

	sendWalkeraBuffer(serialPort);
}

byte WalkeraTelem::lsByte(int value)
{
   return ((byte) ((value) & 0xff));
}

byte WalkeraTelem::s8Byte(int value)
{
   return ((byte) ((value) >> 8));
}

byte WalkeraTelem::s16Byte(int32_t value)
{
   return ((byte) ((value) >> 16));
}

byte WalkeraTelem::s24Byte(int32_t value)
{
   return ((byte) ((value) >> 24));
}

void WalkeraTelem::printValues(SoftwareSerial* debugSerial, ITelemDataProvider* dataProvider)
{
	debugSerial->print("Voltage: ");
	debugSerial->print(dataProvider->getMainBatteryVoltage(), 2);
	debugSerial->print(" Current: ");
	debugSerial->print(dataProvider->getBatteryCurrent(), 2);
	debugSerial->print(" Fuel: ");
	debugSerial->print(dataProvider->getFuelLevel());
	debugSerial->print(" Latitude: ");
	debugSerial->print(dataProvider->getLatitude(), 6);
	debugSerial->print(" Longitude: ");
	debugSerial->print(dataProvider->getLongitud(), 6);
	debugSerial->print(" GPS Alt: ");
	debugSerial->print(dataProvider->getGpsAltitude(), 2);
	//debugSerial->print(" GPS hdop: ");
	//debugSerial->print(dataProvider->getGpsHdop(), 2);
	debugSerial->print(" GPS status + sats: ");
	debugSerial->print(dataProvider->getTemp2());
	debugSerial->print(" GPS speed: ");
	debugSerial->print(dataProvider->getGpsGroundSpeed(), 2);
	debugSerial->print(" Home alt: ");
	debugSerial->print(dataProvider->getAltitude(), 2);
	debugSerial->print(" Mode: ");
	debugSerial->print(dataProvider->getTemp1());
	debugSerial->print(" Course: ");
	debugSerial->print(dataProvider->getCourse(), 2);
	debugSerial->print(" RPM: ");
	debugSerial->print(dataProvider->getEngineSpeed());
	debugSerial->print(" AccX: ");
	debugSerial->print(dataProvider->getAccX(), 2);
	debugSerial->print(" AccY: ");
	debugSerial->print(dataProvider->getAccY(), 2);
	debugSerial->print(" AccZ: ");
	debugSerial->print(dataProvider->getAccZ(), 2);
	debugSerial->println("");
}
