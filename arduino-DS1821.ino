#include <OneWire.h>
#include <Wire.h> 
#include <SD.h>
#include <SPI.h>
/////////////////////////////////////////////////////////////////////////////
/// Wiring: 
	//Memory Card Module CS On Pin 4
	//Memory Card Module MISO On pin 12
	//Memory Card Module MOSI on pin 11
	//Memory Card Module CSLK on pin 13
	//Sensor middle pin (29h) On pin DIGITAL 5
	//Sensor Right Pin (30h) On GND
	//Sensor Left Pin (28h) On 5v

///////////////////////////////////////////////////////////////////////////////
OneWire  ds(5);
int CS_pin = 4;
int delay = 60000; // Logging periode
char fileName[] = "log"; // Name of the logging file
/////////////////////////////////////////////////////////////////////////////
unsigned int readBytes(int count) {
	unsigned int val = 0;
	for (int i = 0; i < count; i++) {
		val |= (unsigned int)(ds.read() << i * 8);
	}
	return val;
}
/////////////////////////////////////////////////////////////////////////////
void setup(void) {
	Serial.begin(9600);
	ds.reset();
	Serial.println("Initializing Card");
	pinMode(CS_pin, OUTPUT);
	if (!SD.begin(CS_pin)) {
		Serial.println("Card Failure");
		return;
	}
	Serial.println("Card Ready");
}
/////////////////////////////////////////////////////////////////////////////
void loop(void) {
	byte temp_read = 0;
	unsigned int count_remain = 0;
	unsigned int count_per_c = 0;
	byte configuration_register = 0;
	ds.reset();
	// Start Converting the temperatures  
	ds.write(0xEE);
	do {
		delay(1);
		configuration_register = 0;
		ds.reset();
		ds.write(0xAC);
		configuration_register = readBytes(1); // Read the configuration Register from sthe DS1821
	} while ((configuration_register & (1 << 7)) == 0); // If Bit #8 is 1 then we are finished converting the temp
	// Get Temp
	ds.reset();
	ds.write(0xAA);
	temp_read = readBytes(1); ;
	// Get Count Remaining
	ds.reset();
	ds.write(0xA0);
	count_remain = readBytes(2);
	// Load The Counter to populate the slope accumulator
	ds.reset();
	ds.write(0x41);
	// Read Count Per Deg
	ds.reset();
	ds.write(0xA0);
	count_per_c = readBytes(2);
	if (temp_read >= 200) temp_read -= 256; // If we are reading above the 200 mark then we are below 0 and need to compensate the calculation
	float highResTemp = (float)temp_read - .5 + (((float)count_per_c - (float)count_remain) / (float)count_per_c);
	// Log the temperature value in a new line
	File dataFile = SD.open(fileName + ".txt", FILE_WRITE);
	if (dataFile) {
		dataFile.println(highResTemp);
		dataFile.close();
		Serial.println(highResTemp);
	}
	else {
		Serial.println("Couldn't open log file");
	}
	// Wait for 'delay' milliseconds
	delay(delay);
}