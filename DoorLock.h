// DoorLock.h

#ifndef _DOORLOCK_h
#define _DOORLOCK_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif
#include <SCoop.h>

class doorLock : public SCoopTimer
{
	int8_t cnt;
public:
	doorLock();
	void setup();
	void run();

	void open();
};

#endif

