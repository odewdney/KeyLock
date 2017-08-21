// CardData.h

#ifndef _CARDDATA_h
#define _CARDDATA_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

class CodeStore
{
	uint16_t offset;
	uint8_t maxCode;
public:
	CodeStore(uint16_t offset, uint8_t maxCode);
	uint8_t GetCount();
	void SetCount(uint8_t count);
	void Clear();

	uint32_t GetCode(uint8_t card);
	void PutCode(uint8_t card, uint32_t cardId);
	uint8_t GetCodeIndex(uint32_t cardId);

	void AddCode(unsigned long cardId);
	void RemoveCode(unsigned long cardId);
	bool CheckCode(unsigned long cardId);

};

extern CodeStore CardStore;
extern CodeStore KeyStore;

#endif

