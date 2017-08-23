// 
// 
// 

#include "SerialData.h"
#include "KeyLock.h"

#include <DallasTemperature.h>
#include <Time.h>
#include <DS1307RTC.h>

#include <DallasTemperature.h>
#include <OneWire.h>

#include "CardData.h"
#include "otp.h"

OneWire oneWire(ONE_WIRE);
DallasTemperature tempSensor(&oneWire);



SerialDataEvent serialDataEvent;


byte SerialDataEvent::nextToken(byte pos)
{
	for (; pos < cmdCnt; pos++)
	{
		if (cmd[pos] == 0)
		{
			break;
		}
		if (cmd[pos] == ' ')
		{
			cmd[pos] = 0;
			pos++;
			break;
		}
	}
	return pos;
}

void PrintMsg(byte b)
{
	switch (b)
	{
	default:
		Serial.println(b);
		break;
	case 0:
		Serial.println(F("OK"));
		break;
	case 1:
		Serial.println(F("Err"));
		break;
	case 2:
		Serial.println(F("Unk"));
		break;

	}
}

byte SerialDataEvent::ProcessCommand()
{
	byte arg, nextarg;
	arg = nextToken(0);

	// login, logout
	if (cmd[0] == 'l')
	{
		if (cmd[1] == 'i')
		{
			if (strcmp(cmd + arg, "123") == 0)
			{
				logout.set(30000);
				return 0;
			}
			else
			{
				return 1;
			}
		}
		else if (cmd[1] == 'o')
		{
			logout.reset();
			return 0;
		}
	}
	if (logout.elapsed())
		return 1;
	logout.set(30000);

	// log - clear, log, display
	// rtc read, set

	if (cmd[0] == 'x')
	{
		if (cmd[1] == 'r')
		{
			tmElements_t x;
			bool f = DS1307RTC::read(x);
			if (f || DS1307RTC::chipPresent)
			{
				if (!f)
					Serial.println(F("Clock stopped"));
				fprintf_P(&uartout, PSTR("%d-%02d-%02d %02d:%02d:%02d\n"), tmYearToCalendar(x.Year), x.Month, x.Day, x.Hour, x.Minute, x.Second);
			}
			else
				Serial.println(F("No clock"));
			return 0;
		}
		else if (cmd[1] == 'n')
		{
			tmElements_t x;
			breakTime(now(), x);
			fprintf_P(&uartout, PSTR("%d-%02d-%02d %02d:%02d:%02d\n"), tmYearToCalendar(x.Year), x.Month, x.Day, x.Hour, x.Minute, x.Second);
			return 0;
		}
		else if (cmd[1] == 'w')
		{
			tmElements_t x;
			nextarg = nextToken(arg);
			x.Year = CalendarYrToTm(atoi(cmd + arg));
			arg = nextarg;
			nextarg = nextToken(arg);
			x.Month = atoi(cmd + arg);
			arg = nextarg;
			nextarg = nextToken(arg);
			x.Day = atoi(cmd + arg);
			arg = nextarg;
			nextarg = nextToken(arg);
			x.Hour = atoi(cmd + arg);
			arg = nextarg;
			nextarg = nextToken(arg);
			x.Minute = atoi(cmd + arg);
			arg = nextarg;
			nextarg = nextToken(arg);
			x.Second = atoi(cmd + arg);
			arg = nextarg;
			DS1307RTC::write(x);
			return 0;
		}
		else if (cmd[1] == 't')
		{
			DeviceAddress deviceAddress;
			tempSensor.getAddress(deviceAddress, 0);
			tempSensor.requestTemperaturesByAddress(deviceAddress);
			int16_t temp = tempSensor.getTemp(deviceAddress);
			int8_t t = temp >> 7;
			//tempSensor.requestTemperaturesByIndex(0);
			//float t = tempSensor.getTempCByIndex(0);

			fprintf_P(&uartout, PSTR("Temp=%d\n"),t);
			return 0;
		}

	}
	else if (cmd[0] == 'd')
	{
		if (cmd[1] == 'o')
		{
			OpenDoor();
			return 0;
		}
	}
	else if (cmd[0] == 'c')
	{
		if (cmd[1] == 'a')
		{
			unsigned long cardId = atol(cmd + arg);
			if (cardId == 0)
				return 1;
			CardStore.AddCode(cardId);
			return 0;
		}
		else if (cmd[1] == 'd')
		{
			unsigned long cardId = atol(cmd + arg);
			if (cardId == 0)
				return 1;
			CardStore.RemoveCode(cardId);
			return 0;
		}
		else if (cmd[1] == 'l')
		{
			uint8_t cnt = CardStore.GetCount();
			fprintf_P(&uartout, PSTR("Count=%d\n"), cnt);
			for (uint8_t n = 0; n < cnt; n++)
			{
				uint32_t cardId = CardStore.GetCode(n);
				Serial.println(cardId);
			}
			return 0;
		}
	}
	else if (cmd[0] == 'k')
	{
		if (cmd[1] == 'a')
		{
			unsigned long cardId = atol(cmd + arg);
			if (cardId == 0)
				return 1;
			int l = strlen(cmd + arg);
			if (l < 4)
				return 1;
			cardId += 100000000 * l;
			KeyStore.AddCode(cardId);
			return 0;
		}
		else if (cmd[1] == 'd')
		{
			unsigned long cardId = atol(cmd + arg);
			if (cardId == 0)
				return 1;
			int l = strlen(cmd + arg);
			if (l < 4)
				return 1;
			cardId += 100000000 * l;
			KeyStore.RemoveCode(cardId);
			return 0;
		}
		else if (cmd[1] == 'l')
		{
			uint8_t cnt = KeyStore.GetCount();
			fprintf_P(&uartout, PSTR("Count=%d\n"), cnt);
			for (uint8_t n = 0; n < cnt; n++)
			{
				uint32_t cardId = KeyStore.GetCode(n);
				Serial.println(cardId);
			}
			return 0;
		}
		else if (cmd[1] == 'c')
		{
			KeyStore.Clear();
			return 0;
		}
	}
	else if (cmd[0] == 'o')
	{
		if (cmd[1] == 's')
		{
			if (strlen(cmd+arg) > 32 ) return 1;
			OtpSave(cmd + arg);
			return 0;
		}
	}
	return 2;
}

SerialDataEvent::SerialDataEvent() : SCoopEvent()
{
	state = SCoopNEW;
}

void SerialDataEvent::setup()
{
	Serial.begin(115200);
	Serial.println(F("sw_init"));
}

void SerialDataEvent::run()
{
	int count;
	if ((count = Serial.available())>0)
	{
		char c = Serial.read();
		if (c == 10 || c == 13)
		{
			if (cmdCnt > 0)
			{
				Serial.println();
				cmd[cmdCnt] = 0;
				byte r = ProcessCommand();
				PrintMsg(r);
				cmdCnt = 0;
			}
		}
		else if (c == 127 || c == 8)
		{
			if (cmdCnt > 0)
			{
				Serial.print(F("\b \b"));
				cmdCnt--;
			}
		}
		else
		{
			if (c >= 32 && c<127)
			{
				if (cmdCnt < sizeof(cmd)-1)
				{
					cmd[cmdCnt++] = c;
					Serial.print((char)c);
				}
			}
		}
	}
}

void serialEvent()
{
	serialDataEvent.set();
}
