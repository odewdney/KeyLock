// audit.h

#ifndef _AUDIT_h
#define _AUDIT_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

void audit(const __FlashStringHelper *szMsg, ...);
void printAudit();
void auditReset();


#endif

