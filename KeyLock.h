#define WIEGAND_0 2
#define WIEGAND_1 3

#define DOORLOCK 5
#define DOORLOCK_LOCK HIGH
#define DOORLOCK_UNLOCK LOW
#define DOORLOCK_LED 6

#define ONE_WIRE 4

void OpenDoor();
void CheckKeypad();

void ClearCards();
void AddCard(unsigned long cardId);
void RemoveCard(unsigned long cardId);
bool CheckCard(unsigned long cardId);
unsigned long GetCard(uint8_t index);
uint8_t GetCardCount();


bool CheckCode(int32_t code);