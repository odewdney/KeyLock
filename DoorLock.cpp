// 
// 
// 

#include "DoorLock.h"
#include "KeyLock.h"

doorLock::doorLock() : SCoopTimer()
{
	state = SCoopNEW;
}

void doorLock::setup()
{
	Serial.println(F("doorlock Setup"));

	pinMode(DOORLOCK, OUTPUT);
	digitalWrite(DOORLOCK, DOORLOCK_LOCK);
	pinMode(DOORLOCK_LED, INPUT_PULLUP);
	digitalWrite(DOORLOCK_LED, HIGH);
}

void doorLock::run()
{
	if (cnt > 0)
	{
//		Serial.println(F("Door led"));
		cnt--;
		digitalWrite(DOORLOCK_LED, LOW);
		pinMode(DOORLOCK_LED, OUTPUT);
		return;
	}
	// switch off relay
	digitalWrite(DOORLOCK, DOORLOCK_LOCK);
	pinMode(DOORLOCK_LED, INPUT_PULLUP);
	digitalWrite(DOORLOCK_LED, HIGH);
	//	Serial.println(F("Door Locked"));
	schedule(1000, 0);
}

void doorLock::open()
{
	cnt = 1;
	digitalWrite(DOORLOCK, DOORLOCK_UNLOCK);
//	Serial.println(F("Door Unlocked"));
	schedule(1000);
}

doorLock doorLock;

void OpenDoor()
{
	doorLock.open();
}

