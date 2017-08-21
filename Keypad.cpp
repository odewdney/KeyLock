// 
// 
// 

#include "Keypad.h"
#include <Wiegand.h>
#include "KeyLock.h"
#include "CardData.h"

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
		if (CardStore.CheckCode(wg.getCode()))
			OpenDoor();
	}
	else if (wg.getWiegandType() == 4)
	{
		int8_t key = wg.getCode();
		if (key >= 0 && key <= 9)
		{
			if (codecnt < 6)
			{
				code = (code*10) + key;
			}
		}
		else if (key == 13)
		{
			if ( KeyStore.CheckCode(code) )
				OpenDoor();
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
