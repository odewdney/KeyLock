// 
// 
// 

#include "audit.h"
#include "Time.h"
#include <Eeprom24C32_64.h>

Eeprom24C32_64 eeprom(0x50);
extern FILE uartout;

struct auditState
{
	char buf[32];
	uint16_t size;
	uint16_t len;
	uint16_t pos;
};

//#define FDEV_SETUP_STREAMx(p,g,f) {auditbuf, 0, f, sizeof(auditbuf), 0, p, g, NULL}
auditState state = {"",32,0,0};
#define FDEV_SETUP_STREAMx(p,g,f) {0, 0, f, 0, 0, p, g, &state}

int audit_putchar(char c, FILE* file);

FILE auditout = FDEV_SETUP_STREAMx(audit_putchar, NULL, _FDEV_SETUP_WRITE);


void audit_flushbuffer(FILE *file)
{
//	uint16_t pos = (uint16_t)fdev_get_udata(file);
	uint16_t pos = state.pos;
	if (pos == 0)
	{ // init
		eeprom.readBytes(0, 2, (byte*)&pos);
		if (pos == 0 || pos > 4096)
			pos = 4;
	}
//	byte *buf = (byte*)file->buf;
	byte *buf = (byte*)state.buf;
	while (state.len > 0)
	{
		uint16_t l = state.len;
		if (pos + l > 4096)
			l = 4096 - pos;
		eeprom.writeBytes(pos, l, buf);
		state.len -= l;
		buf += l;
		pos += l;
		if (pos >= 4096)
			pos = 4;
	}
	//fdev_set_udata(file, (void*)pos);
	state.pos = pos;
}

int audit_putchar(char c, FILE* file)
{
//	file->buf[file->len++] = c;
	state.buf[state.len++] = c;
//	if (file->len == file->size)
	if (state.len == state.size)
	{
		// write buf
		audit_flushbuffer(file);
	}
	Serial.write(c);
	return 0;
}

void audit_flush(FILE *file)
{
	audit_flushbuffer(file);
//	uint16_t pos = (uint16_t)fdev_get_udata(file);
	uint16_t pos = state.pos;
	eeprom.writeBytes(0, 2, (byte*)&pos);
}

void audit(const __FlashStringHelper *szMsg, ...)
{
	tmElements_t x;
	breakTime(now(), x);
	fprintf_P(&auditout, PSTR("%d-%02d-%02d %02d:%02d:%02d: "), tmYearToCalendar(x.Year), x.Month, x.Day, x.Hour, x.Minute, x.Second);
	va_list va;
	va_start(va, szMsg);
	vfprintf_P(&auditout, (const char *)szMsg, va);
	va_end(va);
	fputs_P(PSTR("\r\n"), &auditout);
	audit_flush(&auditout);
}

void printAudit()
{
	uint16_t pos, n;
	eeprom.readBytes(0, 2, (byte*)&pos);
	if (pos > 4096) pos = 4096;
	for (n = 4; n < pos; n++)
	{
		byte c = eeprom.readByte(n);
		if (c > 0 && c < 0xff)
			Serial.print((char)c);
	}
}

void auditReset()
{
	uint16_t pos = 4;
	eeprom.writeBytes(0, 2, (byte*)&pos);
	state.len = 0;
	state.pos = pos;
}
