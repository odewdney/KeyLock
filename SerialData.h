// SerialData.h

#ifndef _SERIALDATA_h
#define _SERIALDATA_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif
#include <SCoop.h>

class SerialDataEvent : public SCoopEvent
{

	char cmd[40];
	byte cmdCnt = 0;
	SCoopDelay logout;

	byte nextToken(byte pos);
	byte ProcessCommand();

public:
	SerialDataEvent();
	void setup();
	void run();
};


#endif

