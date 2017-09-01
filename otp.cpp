// 
// 
// 

#include "otp.h"
#include <sha1.h>
#include <EEPROM.h>
#include "KeyLock.h"

int16_t fromBase32(const char*input, uint8_t *out, int16_t len)
{
	int16_t result = 0;
	uint16_t buffer;
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

//const char base32StandardAlphabet[] = { "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567" };

int16_t toBase32(const uint8_t *in, int16_t len, char* out)
{
	uint16_t buffer = *in;
	uint16_t next = 1;
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
		out[result++] = index < 26 ? 'A' + index : '2' + (index - 26);// base32StandardAlphabet[index];
	}
	out[result] = 0;
	return result;
}

void printHash(const uint8_t* hash, uint8_t len)
{
	//const static char hex[] = "0123456789abcdef";
	uint8_t i;
	for (i = 0; i<len; i++) {
		if (i > 0) fputc('-', &uartout);
		fprintf_P(&uartout, "%02x", hash[i]);
		//Serial.print(hex[hash[i] >> 4]);
		//Serial.print(hex[hash[i] & 0xf]);
		//Serial.print("-");
	}
	fputc('\n', &uartout);
}

uint32_t GetTOTPCode(const char*key, time_t time)
{
	uint8_t keyBytes[20];
	int16_t len = fromBase32(key, keyBytes, 20);

	return GetTOTPCode(keyBytes, len, time);
}
uint32_t GetTOTPCode(const uint8_t* key, uint8_t keylen, time_t time)
{
	time = time / 30;
	//	Serial.print("time=");
	//	Serial.println(time);

	uint8_t data[8];
	*(uint32_t*)data = 0;
	*(uint32_t*)(data + 4) = __builtin_bswap32(time);
	return GetTOTPCode(key, keylen, data, 8);
}

uint32_t GetTOTPCode(const uint8_t* key, uint8_t keylen, const uint8_t *data, uint8_t datalen)
{
	Sha1.initHmac(key, keylen);
	Sha1.write(data, datalen);
	uint8_t *hash = Sha1.resultHmac();
	uint8_t offset = hash[19] & 0xf;
	uint32_t code = __builtin_bswap32(*(uint32_t*)(hash+offset) ) & 0x7fffffff;

	/*
	Serial.print("hex code:");
	Serial.println(code, HEX);
	Serial.print("dec code:");
	Serial.println(code, 10);
	*/
	code = code % 1000000L;
	return code;
}

uint8_t key[20];

void OtpSave(const char*b32)
{
	fromBase32(b32, key, 20);
	EEPROM.put(90, key);
}


struct timeWindow
{
	time_t start;
	time_t end;
};

bool SummerTime(time_t t)
{
	tmElements_t e;
	breakTime(t, e);
	return SummerTime(e);
}

bool SummerTime(const tmElements_t &e)
{
	if (e.Month <3 || e.Month >10)
		return false;
	if (e.Month > 3 && e.Month < 10)
		return true;
	const static uint8_t tzHours = 0;
	if (e.Month == 3 && (e.Hour + 24 * e.Day) >= (1 + tzHours + 24 * (31 - (5 * e.Year / 4 + 4) % 7)))
		return true;
	if (e.Month == 10 && (e.Hour + 24 * e.Day)<(1 + tzHours + 24 * (31 - (5 * e.Year / 4 + 1) % 7)))
		return true;
	return false;
}

void OtpInit()
{
	EEPROM.get(90, key);

	time_t t = now();
	time_t tday = previousMidnight(t);

	struct timeWindow w;
	// check daily code
	w.start = tday;
	if (SummerTime(w.start)) // convert to gmt
		w.start -= 3600;
	w.end = w.start + SECS_PER_DAY;
	uint32_t code = GetTOTPCode(key, 20, (uint8_t*)&w, 8);
	fprintf_P(&uartout, PSTR("Daily=%lu %lu %lu\n"), w.start, w.end, code);

	// check nights code
	for (int8_t s = 0; s >= -14; s--)
	{
		w.start = tday + s * SECS_PER_DAY + 15 * SECS_PER_HOUR;
		if (SummerTime(w.start)) // convert to gmt
			w.start -= 3600;
		if (w.start > t)
			continue;
		for (int8_t e = 0; (e - s) <= 14; e++)
		{
			w.end = tday + e * SECS_PER_DAY + 11 * SECS_PER_HOUR;
			if (SummerTime(w.end))
				w.end -= 3600;
			if (w.end < t)
				continue;
			uint32_t OTPcode = GetTOTPCode(key, 20, (uint8_t*)&w, 8);
			fprintf_P(&uartout, PSTR("%lu %lu %lu\n"), w.start, w.end, OTPcode);
		}
	}
}

struct timeWindow cacheWindow = { 0, 0 };
uint32_t cacheCode = 0;

bool OtpCheck(uint32_t code)
{
	if ((code / 100000000) != 6)
		return false;
	code = code % 1000000;

	EEPROM.get(90, key);

	// check one time code
	time_t t = now();
	for (int8_t n = -2; n <= 2; n++)
	{
		if (GetTOTPCode(key, 20, t + (30 * n)) == code)
			return true;
	}

	if (t > cacheWindow.start && t < cacheWindow.end && code == cacheCode)
		return true;

	//fprintf_P(&uartout, PSTR("looking for code %ld"), code);

	time_t tday = previousMidnight(t);

	struct timeWindow w;
	
	// check daily code
	w.start = tday;
	if (SummerTime(w.start)) // convert to gmt
		w.start -= 3600;
	w.end = w.start + SECS_PER_DAY;
	if ( GetTOTPCode(key, 20, (uint8_t*)&w, 8) == code )
	{
		return true;
	}
	
	// check nights code
	for (int8_t s = 0; s >= -14; s--)
	{
		w.start = tday + s * SECS_PER_DAY + 15 * SECS_PER_HOUR;
		if (SummerTime(w.start)) // convert to gmt
			w.start -= 3600;
		if ( w.start > t )
			continue;
		for (int8_t e = 0; (e - s) <= 14; e++)
		{
			w.end = tday + e * SECS_PER_DAY + 11 * SECS_PER_HOUR;
			if (SummerTime(w.end))
				w.end -= 3600;
			if ( w.end < t )
				continue;
			uint32_t OTPcode = GetTOTPCode(key, 20, (uint8_t*)&w, 8);
			//fprintf_P(&uartout, PSTR("%d %d %ld\n"), s, e, OTPcode);
			if (OTPcode == code)
			{
				fputs_P(PSTR("Found"), &uartout);
				cacheWindow = w;
				cacheCode = code;
				return true;
			}
		}
	}
	return false;
}
