// Keypad.h

#ifndef _KEYPAD_h
#define _KEYPAD_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif
#include <SCoop.h>

class keypadEventClass : public SCoopEvent
{
public:
	keypadEventClass();
	void setup();
	void run();

	void check();
};


#endif

