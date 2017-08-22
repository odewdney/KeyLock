#include <EEPROM.h>
//#include "CardData.h"
//#include "Keypad.h"
//#include "DoorLock.h"
//#include <sha1.h>
#include <Eeprom24C32_64.h>
#include <Time.h>
#include <TimeAlarms.h>
#include <DS1307RTC.h>
#include <SCoop.h>
#include "KeyLock.h"

defineTimerRun(refreshAlarm,  1000)
{
	Alarm.delay(0);
}



/*
void printHash(uint8_t* hash) {
	const static char hex[] = "0123456789abcdef";
	int i;
	for (i = 0; i<20; i++) {
		Serial.print(hex[hash[i] >> 4]);
		Serial.print(hex[hash[i] & 0xf]);
	}
	Serial.println();
}
uint8_t hmacKey1[] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f
};

void test()
{
	unsigned long ms, ms2;
	uint8_t *hash;

	// SHA tests
	Serial.println("Test: FIPS 180-2 C.1 and RFC3174 7.3 TEST1");
	Serial.println("Expect:a9993e364706816aba3e25717850c26c9cd0d89d");
	Serial.print("Result:");
	ms = micros();
	Sha1.init();
	Sha1.print("abc");
	hash = Sha1.result();
	ms2 = micros();
	printHash(hash);
	Serial.print(" Hash took : ");
	Serial.print((ms2 - ms));
	Serial.println(" micros");

	Serial.println("Test: FIPS 198a A.1");
	Serial.println("Expect:4f4ca3d5d68ba7cc0a1208c9c61e9c5da0403c0a");
	Serial.print("Result:");
	ms = micros();
	Sha1.initHmac(hmacKey1, 64);
	Sha1.print("Sample #1");
	hash = Sha1.resultHmac();
	ms2 = micros();
	printHash(hash);
	Serial.print(" Hash took : ");
	Serial.print((ms2 - ms));
	Serial.println(" micros");
}
*/

time_t getTinyRTCTime()
{
	time_t t = RTC.get();
	return t;
}

void Midnight()
{
	Serial.print("alarm-midnight:");
	Serial.println(now());
}
void Morning()
{
	Serial.print("alarm-morning:");
	Serial.println(now());
}
void Afternoon()
{
	Serial.print("alarm-afternoon:");
	Serial.println(now());
}


int16_t fromBase32(char*input, int8_t *out, int16_t len)
{
	int16_t result = 0;
	int16_t buffer;
	int8_t bitsLeft = 0;

	for (int16_t i = 0; result < len; i++)
	{
		byte ch = input[i];
		if (ch == 0)
			break;
		// ignoring some characters: ' ', '\t', '\r', '\n', '='
		if (ch == 0xA0 || ch == 0x09 || ch == 0x0A || ch == 0x0D || ch == 0x3D) continue;

		// recovering mistyped: '0' -> 'O', '1' -> 'L', '8' -> 'B'
		if (ch == 0x30) { ch = 0x4F; }
		else if (ch == 0x31) { ch = 0x4C; }
		else if (ch == 0x38) { ch = 0x42; }


		// look up one base32 symbols: from 'A' to 'Z' or from 'a' to 'z' or from '2' to '7'
		if ((ch >= 0x41 && ch <= 0x5A) || (ch >= 0x61 && ch <= 0x7A)) { ch = ((ch & 0x1F) - 1); }
		else if (ch >= 0x32 && ch <= 0x37) { ch -= (0x32 - 26); }
		else { return 0; }

		buffer <<= 5;
		buffer |= ch;
		bitsLeft += 5;
		if (bitsLeft >= 8)
		{
			out[result] = (unsigned char)((buffer >> (bitsLeft - 8)) & 0xFF);
			result++;
			bitsLeft -= 8;
		}
	}
	return result;
}

char base32StandardAlphabet[] = { "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567" };

int16_t toBase32(int8_t *in, int16_t len, char* out)
{
	int16_t buffer = *in;
	int16_t next = 1;
	int8_t bitsLeft = 8;
	int16_t result = 0;

	while (bitsLeft > 0 || next < len)
	{
		if (bitsLeft < 5)
		{
			if (next < len)
			{
				buffer <<= 8;
				buffer |= in[next] & 0xff;
				next++;
				bitsLeft += 8;
			}
			else
			{
				int8_t pad = 5 - bitsLeft;
				buffer <<= pad;
				bitsLeft += pad;
			}
		}
		int8_t index = (buffer >> (bitsLeft - 5)) & 0x1f;
		bitsLeft -= 5;
		out[result++] = base32StandardAlphabet[index];
	}
	out[result] = 0;
	return result;
}

void setup()
{
	Serial.begin(115200);

	setSyncProvider(getTinyRTCTime);

	Alarm.alarmRepeat(9, 19, 0, Midnight);

	Alarm.alarmRepeat(0, 0, 5, Midnight);
	Alarm.alarmRepeat(10, 0, 5, Morning);
	Alarm.alarmRepeat(16, 0, 5, Afternoon);

	char buf[30];
	char *in = "1234";
	toBase32((int8_t*)in, 4, buf);
	Serial.println(buf);
	char buf2[30];
	int l = fromBase32(buf, (int8_t*)buf2, 30);
	buf2[l] = 0;
	Serial.println(buf2);

  /* add setup code here */
	mySCoop.start();
//	test();
}


void loop()
{
	CheckKeypad();
  /* add main program code here */
	yield();
}
