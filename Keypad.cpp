// 
// 
// 

#include "Keypad.h"
#include <Wiegand.h>
#include "KeyLock.h"
#include "CardData.h"
#include "otp.h"
#include "audit.h"
WIEGAND wg;


keypadEventClass::keypadEventClass()
{
	state = SCoopNEW;
}

void keypadEventClass::setup()
{
	wg.begin();

}

int32_t code = 0;
int8_t codecnt = 0;
unsigned long lastkey = 0;

void keypadEventClass::run()
{
	/*
	Serial.print("Wiegand HEX = ");
	Serial.print(wg.getCode(), HEX);
	Serial.print(", DECIMAL = ");
	Serial.print(wg.getCode());
	Serial.print(", Type W");
	Serial.println(wg.getWiegandType());
	*/
	if (wg.getWiegandType() == 26)
	{
		unsigned long fob = wg.getCode();
		if (CardStore.CheckCode(fob))
		{
			audit(F("FobOpen %ld"), fob);
			OpenDoor();
		}
	}
	else if (wg.getWiegandType() == 4)
	{
		unsigned long nowmilli = millis();
		if ((nowmilli - lastkey) > 6000)
		{
			code = 0;
			codecnt = 0;
		}
		lastkey = nowmilli;
		int8_t key = wg.getCode();
		if (key >= 0 && key <= 9)
		{
			code = ((code * 10) + key) % 100000000;
			if (codecnt < 8)
				codecnt++;
		}
		else if (key == 13 || key == 27) // # or *
		{
			if (codecnt >= 4)
			{
				code += codecnt * 100000000;
				if (KeyStore.CheckCode(code))
				{
					audit(F("KeyOpen %ld"), code);
					OpenDoor();
				}
				else if (OtpCheck(code))
				{
					audit(F("OtpOpen %ld"), code);
					OpenDoor();
				}
			}
			code = 0;
			codecnt = 0;
		}
	}
}

void keypadEventClass::check()
{
	if (wg.available())
		set();
}

keypadEventClass keypadEvent;

void CheckKeypad()
{
	keypadEvent.check();
}
