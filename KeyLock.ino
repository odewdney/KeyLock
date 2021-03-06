#include "audit.h"
#include <EEPROM.h>
//#include "CardData.h"
//#include "Keypad.h"
//#include "DoorLock.h"
//#include <sha1.h>
#include <Time.h>
#include <TimeAlarms.h>
//#include <DS1307RTC.h>
#include <DS3232RTC.h>
#include <SCoop.h>
#include "KeyLock.h"
#include "otp.h"
#include "audit.h"

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
#ifdef DS1307
	time_t epoc;
	int16_t secPerDay;
	RTC.get(0x10, epoc);
	RTC.get(0x14, secPerDay);
	//fprintf_P(&uartout, PSTR("Epoc=%ld\n"), epoc);
	if (epoc != 0xffffffff && epoc != 0)
	{
		t += (secPerDay * ((t - epoc)/864)) / 100;
	}
#endif
	return t;
}

void Midnight()
{
	fprintf_P(&uartout, PSTR("alarm-midnight:%ld\n"), now());
}


void Afternoon()
{
	fprintf_P(&uartout, PSTR("alarm-afternoon:%ld\n"), now());
}


FILE uartout = { 0 };

static int uart_putchar(char c, FILE *stream)
{
	Serial.write(c);
	return 0;
}

void setup()
{
//	Serial.begin(115200);
	Serial.begin(9600);
	fdev_setup_stream(&uartout, uart_putchar, NULL, _FDEV_SETUP_WRITE);

	setSyncProvider(getTinyRTCTime);
	setSyncInterval(60);

	audit(F("Boot"));

	//OtpInit();

	//Alarm.alarmRepeat(0, 0, 5, Midnight);
	//Alarm.alarmRepeat(16, 0, 5, Afternoon);

	char *key = "HXDMVJECJJWSRB3HWIZR4IFUGFTMXBOZ";
	uint32_t code = GetTOTPCode(key, now());
	fprintf_P(&uartout, PSTR("Code=%ld\n"), code);

	//audit(F("Code = %06ld"), code);

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
