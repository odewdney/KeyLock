// otp.h

#ifndef _OTP_h
#define _OTP_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include <Time.h>

void printHash(const uint8_t* hash, uint8_t len);
int16_t toBase32(const uint8_t *in, int16_t len, char* out);
int16_t fromBase32(const char*input, uint8_t *out, int16_t len);
uint32_t GetTOTPCode(const char*key, time_t time);
uint32_t GetTOTPCode(const uint8_t* key, uint8_t keylen, time_t time);
uint32_t GetTOTPCode(const uint8_t* key, uint8_t keylen, const uint8_t *data, uint8_t datalen);

void OtpSave(const char*b32);
bool OtpCheck(uint32_t code);
void OtpInit();

bool SummerTime(time_t t);
bool SummerTime(const tmElements_t &e);

#endif

