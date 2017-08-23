// otp.h

#ifndef _OTP_h
#define _OTP_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include <Time.h>

void printHash(uint8_t* hash, uint8_t len);
int16_t toBase32(uint8_t *in, int16_t len, char* out);
int16_t fromBase32(char*input, uint8_t *out, int16_t len);
uint32_t GetTOTPCode(char*key, time_t time);
uint32_t GetTOTPCode(uint8_t* key, uint8_t keylen, time_t time);
uint32_t GetTOTPCode(uint8_t* key, uint8_t keylen, uint8_t *data, uint8_t datalen);

void OtpSave(char*b32);
bool OtpCheck(uint32_t code);

#endif

