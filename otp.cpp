// 
// 
// 

#include "otp.h"
#include <sha1.h>

int16_t fromBase32(char*input, uint8_t *out, int16_t len)
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

int16_t toBase32(uint8_t *in, int16_t len, char* out)
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

void printHash(uint8_t* hash, uint8_t len)
{
	const static char hex[] = "0123456789abcdef";
	uint8_t i;
	for (i = 0; i<len; i++) {
		Serial.print(hex[hash[i] >> 4]);
		Serial.print(hex[hash[i] & 0xf]);
		Serial.print("-");
	}
	Serial.println();
}

uint32_t GetTOTPCode(char*key, time_t time)
{
	uint8_t keyBytes[20];
	int16_t len = fromBase32(key, keyBytes, 20);

	return GetTOTPCode(keyBytes, len, time);
}
uint32_t GetTOTPCode(uint8_t* key, uint8_t keylen, time_t time)
{
	time = time / 30;
	//	Serial.print("time=");
	//	Serial.println(time);

	uint8_t data[8];
	*(uint32_t*)data = 0;
	*(uint32_t*)(data + 4) = __builtin_bswap32(time);
	return GetTOTPCode(key, keylen, data, 8);
}

uint32_t GetTOTPCode(uint8_t* key, uint8_t keylen, uint8_t *data, uint8_t datalen)
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

void OtpSave(char*b32)
{
	fromBase32(b32, key, 20);
}

bool OtpCheck(uint32_t code)
{
	if ((code / 100000000) != 6)
		return false;
	code = code % 1000000;
	time_t t = now() - 3600;
	for (int8_t n = -2; n <= 2; n++)
	{
		if (GetTOTPCode(key, 20, t + (30 * n)) == code)
			return true;
	}
	return false;
}
