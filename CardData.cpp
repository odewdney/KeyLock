// 
// 
// 

#include "CardData.h"
#include <EEPROM.h>

#define CARD_OFFSET 1
#define CARD_DATA 2
#define CARD_MAX 10

// cards 2 - ((10*4)+2)=42

#define KEY_OFFSET 42
#define KEY_DATA 43
#define KEY_MAX 10

// max key = 42 - 83

// passwd 86
#define PASSWD 86
// key - 90

void SavePass(uint32_t code)
{
	EEPROM.put(PASSWD, code);
}

bool CheckPass(uint32_t code)
{
	uint32_t pass;
	EEPROM.get(PASSWD, pass);
	if (pass == 0 || pass == 0xffffffff)
		pass = 123;
	if (pass == code)
		return true;
	return false;
}

CodeStore CardStore(CARD_OFFSET, CARD_MAX);
CodeStore KeyStore(KEY_OFFSET, KEY_MAX);


CodeStore::CodeStore(uint16_t offset, uint8_t maxCode) : offset(offset), maxCode(maxCode)
{

}

uint8_t CodeStore::GetCount()
{
	uint8_t count;

	EEPROM.get(offset, count);
	if (count == 0xff)
		return 0;
	return count;
}

void CodeStore::SetCount(uint8_t count)
{
	EEPROM.put(offset, count);
}

void CodeStore::Clear()
{
	SetCount(0);
}


uint32_t CodeStore::GetCode(uint8_t card)
{
	uint32_t cardId;

	EEPROM.get(offset + 1 + card*sizeof(uint32_t), cardId);
	return cardId;
}

void CodeStore::PutCode(uint8_t card, uint32_t cardId)
{
	EEPROM.put(offset + 1 + card*sizeof(uint32_t), cardId);
}

uint8_t CodeStore::GetCodeIndex(uint32_t cardId)
{
	uint8_t index = 0xff;
	uint8_t cardCount = GetCount();
	for (uint8_t n = 0; n < cardCount; n++)
	{
		uint32_t card = GetCode(n);

		if (card == cardId)
		{
			index = n;
		}
	}
	return index;
}

void CodeStore::AddCode(unsigned long cardId)
{
	uint8_t count = GetCount();
	uint8_t n;
	n = GetCodeIndex(cardId);
	if (n != 0xff)
		return;
	for (n = 0; n < count; n++)
	{
		uint32_t cardIdN = GetCode(n);
		if (cardIdN == 0 || cardIdN == 0xffffffffU)
		{
			PutCode(n, cardId);
			return;
		}
	}
	if (count < CARD_MAX)
	{
		PutCode(count, cardId);
		SetCount(count + 1);
	}
}

void CodeStore::RemoveCode(unsigned long cardId)
{
	uint8_t index = GetCodeIndex(cardId);
	if (index != 0xff)
	{
		uint8_t count = GetCount();
		count--;
		if (count > 0 && index != count)
		{
			unsigned long endCode = GetCode(count);
			PutCode(index, endCode);
		}
		SetCount(count);
	}
}

bool CodeStore::CheckCode(unsigned long cardId)
{
	bool ret = false;
	if (cardId != 0)
	{
		uint8_t index = GetCodeIndex(cardId);
		if (index != 0xff)
			ret = true;
	}
	return ret;
}


