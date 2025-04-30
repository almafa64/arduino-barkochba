#include <EEPROM.h>
#include <ctype.h>

// ----- User settings -----

// Uncomment this to clear EEPROM data on start, comment out to keep new questions
#define CLEAR_RUN

// ----- Defines -----

#define LEN(arr) ((sizeof(arr))/(sizeof(arr[0])))

#define BIT(nth_bit)                         (1U << (nth_bit))
#define CHECK_BIT(data, bit)                 ((data) & BIT(bit))
#define GET_WITHOUT_FLAGS(data, flagMask)    ((data) & (flagMask))

// ['\0', yesAddress, noAddress, questionTag]
#define EMPTY_QUESTION_SIZE (sizeof(char) + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint8_t))

// [[<ROM_TEXTS index>, '\0'], yesAddress, noAddress, questionTag]
#define EXIT_QUESTION_SIZE (EMPTY_QUESTION_SIZE + sizeof(char))

#define IS_EEPROM_ADDRESS(address)           CHECK_BIT((address), 15)
#define GET_EEPROM_BASED_ADDRESS(address)    GET_WITHOUT_FLAGS((address), ~BIT(15))
#define GET_ABSOLUTE_ADDRESS(address)        ((address) | BIT(15))
// GET_EXIT_ADDRESS
#define GEA(index)                           (GET_ABSOLUTE_ADDRESS((index) * EXIT_QUESTION_SIZE))

#define IS_A_AN_FLAG_PRESENT(questionType)   CHECK_BIT((questionType), 7)
#define GET_REAL_QUESTION_TYPE(questionType) GET_WITHOUT_FLAGS((questionType), ~BIT(7))
#define GET_WITH_A_AN_FLAG(questionType)     ((questionType) | BIT(7))

// ----- Helper defines for PROGMEM data -----

#define CREATE_QUESTION_TAG(index, text) \
	const PROGMEM char __s##index[] = text

#define CREATE_ROM_TEXT(index, text) \
	const PROGMEM char __ts##index[] = text

#define CREATE_QUESTION(name, text, yes_node, no_node, flags) \
	const PROGMEM char __qs##name[] = text; \
	const PROGMEM QuestionNode __q##name = {(char *)__qs##name, yes_node, no_node, flags}

#define CREATE_EXIT_QUESTION(index, flags) \
	const PROGMEM uint8_t __eqs##index[] = {index + 1, 0}; \
	const PROGMEM QuestionNode __eq##index = {(char *)__eqs##index, 0, 0, flags}

// ----- Types -----

struct QuestionNode {
	char *text;
	uint16_t yesAddress;
	uint16_t noAddress;
	uint8_t questionTag;     // top bit flag for a/an article
};

enum QuestionTag : uint8_t {
	QT_NONE = 0,
	QT_MADE_OF,
	QT_IS_IT,
	QT_CAN_BE,
	QT_CAN_IT,
	QT_DOES_IT,
	QT_IS_THIS_INDIVIDUAL,

	QT_A_AN = 1 << 7,
};

typedef uint16_t EepromAddress;

// ----- Question tags -----
// s = string

CREATE_QUESTION_TAG(0, "");
CREATE_QUESTION_TAG(1, "Is it made of ");
CREATE_QUESTION_TAG(2, "Is it ");
CREATE_QUESTION_TAG(3, "Can it be ");
CREATE_QUESTION_TAG(4, "Can it ");
CREATE_QUESTION_TAG(5, "Does it ");
CREATE_QUESTION_TAG(6, "Is this individual ");

const PROGMEM char *const QUESTION_TAGS[] = {__s0, __s1, __s2, __s3, __s4, __s5, __s6};

// ----- Hardcoded questions -----
// q = question
// s = string
// 1. number = level count
// y/n = yes/no branch
// 2. number = y/n branch count in level

CREATE_QUESTION(1, "living being", 1, 2, QT_IS_IT | QT_A_AN);                     // 0

CREATE_QUESTION(2y1, "animal", 3, 4, QT_IS_IT | QT_A_AN);                         // 1
CREATE_QUESTION(2n1, "intangible", 5, 6, QT_IS_IT);                               // 2

CREATE_QUESTION(3y1, "mammal", 7, 8, QT_IS_IT | QT_A_AN);                         // 3
CREATE_QUESTION(3n1, "type of flower", 9, 10, QT_IS_IT | QT_A_AN);                // 4
CREATE_QUESTION(3y2, "form of entertainment", 11, 12, QT_IS_IT | QT_A_AN);        // 5
CREATE_QUESTION(3n2, "small", 13, 14, QT_IS_IT);                                  // 6

CREATE_QUESTION(4y1, "human", 15, 16, QT_IS_IT | QT_A_AN);                        // 7
CREATE_QUESTION(4n1, "reptile", 17, 18, QT_IS_IT | QT_A_AN);                      // 8
CREATE_QUESTION(4y2, "yellow", 19, 20, QT_IS_IT);                                 // 9
CREATE_QUESTION(4n2, "fruit", 21, 22, QT_IS_IT | QT_A_AN);                        // 10
CREATE_QUESTION(4y3, "digital", 23, 24, QT_IS_IT);                                // 11
CREATE_QUESTION(4n3, "school subject", 25, 26, QT_IS_IT | QT_A_AN);               // 12
CREATE_QUESTION(4y4, "paper", 27, 28, QT_MADE_OF);                                // 13
CREATE_QUESTION(4n4, "used for living", 29, 30, QT_CAN_BE);                       // 14

CREATE_QUESTION(5y1, "famous", GEA(0), GEA(1), QT_IS_THIS_INDIVIDUAL);            // 15 -> y: Hatalyak Balint, n: Elon Musk
CREATE_QUESTION(5n1, "carnivore", GEA(2), GEA(3), QT_IS_IT | QT_A_AN);            // 16 -> y: Wolf, n: Cow
CREATE_QUESTION(5y2, "have legs", GEA(4), GEA(5), QT_DOES_IT);                    // 17 -> y: Lizard, n: Snake
CREATE_QUESTION(5n2, "have beak", GEA(6), GEA(7), QT_DOES_IT | QT_A_AN);          // 18 -> y: Parrot, n: Fish
CREATE_QUESTION(5y3, "look up to the sun", GEA(8), GEA(9), QT_DOES_IT);           // 19 -> y: Sunflower, n: Daffodils
CREATE_QUESTION(5n3, "small", GEA(10), GEA(11), QT_IS_IT);                        // 20 -> y: Lobelia, n: Bamboo
CREATE_QUESTION(5y4, "red", GEA(12), GEA(13), QT_IS_IT);                          // 21 -> y: Apple, n: Banana
CREATE_QUESTION(5n4, "spicy", GEA(14), GEA(15), QT_IS_IT);                        // 22 -> y: Chili, n: Potato
CREATE_QUESTION(5y5, "video game", 31, 32, QT_IS_IT | QT_A_AN);                   // 23
CREATE_QUESTION(5n5, "laughed about", GEA(16), GEA(17), QT_CAN_BE);               // 24 -> y: USA, n: Tragic show
CREATE_QUESTION(5y6, "fun", GEA(18), GEA(19), QT_IS_IT);                          // 25 -> y: IT, n: Physics
CREATE_QUESTION(5n6, "natural disaster", GEA(20), GEA(21), QT_IS_IT | QT_A_AN);   // 26 -> y: Vulcano eruption, n: Your mom
CREATE_QUESTION(5y7, "hold infromation", GEA(22), GEA(23), QT_DOES_IT);           // 27 -> y: Photograph, n: White clear paper
CREATE_QUESTION(5n7, "heavy", GEA(24), GEA(25), QT_IS_IT);                        // 28 -> y: Tungsten cube, n: Smartphone
CREATE_QUESTION(5y8, "move", GEA(26), GEA(27), QT_CAN_IT);                        // 29 -> y: Car, n: House
CREATE_QUESTION(5n8, "fly", GEA(28), GEA(29), QT_CAN_IT);                         // 30 -> y: Rocket, n: Antenna

CREATE_QUESTION(6y1, "scifi", 33, 34, QT_IS_IT);                                  // 31
CREATE_QUESTION(6n1, "music", GEA(30), GEA(31), QT_IS_IT);                        // 32 -> y: SKIDS, n: SKIDS

CREATE_QUESTION(7y1, "focus on empire management", GEA(32), GEA(33), QT_DOES_IT); // 33 -> y: Stellaris, n: Space engineers
CREATE_QUESTION(7n1, "involve stealth", GEA(34), GEA(35), QT_DOES_IT);            // 34 -> y: Assassin's Creed, n: South park: The stick of truth

const PROGMEM QuestionNode ROM_QUESTIONS[] = {
	__q1,
	__q2y1, __q2n1,
	__q3y1, __q3n1, __q3y2, __q3n2,
	__q4y1, __q4n1, __q4y2, __q4n2, __q4y3, __q4n3, __q4y4, __q4n4,
	__q5y1, __q5n1, __q5y2, __q5n2, __q5y3, __q5n3, __q5y4, __q5n4, __q5y5, __q5n5, __q5y6, __q5n6, __q5y7, __q5n7, __q5y8, __q5n8,
	__q6y1, __q6n1,
	__q7y1, __q7n1,
};

// ----- Hardcoded texts for repeated/exit texts to save EEPROM space -----
// ts = text string

CREATE_ROM_TEXT( 0, "");
CREATE_ROM_TEXT( 1, "Michael reeves");
CREATE_ROM_TEXT( 2, "Elon Musk");
CREATE_ROM_TEXT( 3, "wolf");
CREATE_ROM_TEXT( 4, "cow");
CREATE_ROM_TEXT( 5, "lizard");
CREATE_ROM_TEXT( 6, "snake");
CREATE_ROM_TEXT( 7, "parrot");
CREATE_ROM_TEXT( 8, "fish");
CREATE_ROM_TEXT( 9, "sunflower");
CREATE_ROM_TEXT(10, "daffodils");
CREATE_ROM_TEXT(11, "lobelia");
CREATE_ROM_TEXT(12, "bamboo");
CREATE_ROM_TEXT(13, "apple");
CREATE_ROM_TEXT(14, "banana");
CREATE_ROM_TEXT(15, "chili");
CREATE_ROM_TEXT(16, "potato");
CREATE_ROM_TEXT(17, "the USA");
CREATE_ROM_TEXT(18, "tragic show");
CREATE_ROM_TEXT(19, "IT");
CREATE_ROM_TEXT(20, "Physics");
CREATE_ROM_TEXT(21, "vulcano eruption");
CREATE_ROM_TEXT(22, "your mom");
CREATE_ROM_TEXT(23, "photograph");
CREATE_ROM_TEXT(24, "clear white paper");
CREATE_ROM_TEXT(25, "tungsten cube");
CREATE_ROM_TEXT(26, "smartphone");
CREATE_ROM_TEXT(27, "car");
CREATE_ROM_TEXT(28, "house");
CREATE_ROM_TEXT(29, "rocket");
CREATE_ROM_TEXT(30, "antenna");
CREATE_ROM_TEXT(31, "SKIDS (by Rare Americans)");
CREATE_ROM_TEXT(32, "SKIDS (by Rare Americans)");
CREATE_ROM_TEXT(33, "Stellaris");
CREATE_ROM_TEXT(34, "Space engineers");
CREATE_ROM_TEXT(35, "Assassin's Creed");
CREATE_ROM_TEXT(36, "South park: The stick of truth");

const PROGMEM char *const ROM_TEXTS[] = {
	__ts0, __ts1, __ts2, __ts3, __ts4, __ts5, __ts6, __ts7, __ts8, __ts9, __ts10,
	__ts11, __ts12, __ts13, __ts14, __ts15, __ts16, __ts17, __ts18, __ts19, __ts20,
	__ts21, __ts22, __ts23, __ts24, __ts25, __ts26, __ts27, __ts28, __ts29, __ts30,
	__ts31, __ts32, __ts33, __ts34, __ts35, __ts36
};

// ----- Question exits to store in EEPROM -----

CREATE_EXIT_QUESTION( 0, QT_IS_IT);
CREATE_EXIT_QUESTION( 1, QT_IS_IT);
CREATE_EXIT_QUESTION( 2, QT_IS_IT | QT_A_AN);
CREATE_EXIT_QUESTION( 3, QT_IS_IT | QT_A_AN);
CREATE_EXIT_QUESTION( 4, QT_IS_IT | QT_A_AN);
CREATE_EXIT_QUESTION( 5, QT_IS_IT | QT_A_AN);
CREATE_EXIT_QUESTION( 6, QT_IS_IT | QT_A_AN);
CREATE_EXIT_QUESTION( 7, QT_IS_IT | QT_A_AN);
CREATE_EXIT_QUESTION( 8, QT_IS_IT | QT_A_AN);
CREATE_EXIT_QUESTION( 9, QT_IS_IT | QT_A_AN);
CREATE_EXIT_QUESTION(10, QT_IS_IT | QT_A_AN);
CREATE_EXIT_QUESTION(11, QT_IS_IT | QT_A_AN);
CREATE_EXIT_QUESTION(12, QT_IS_IT | QT_A_AN);
CREATE_EXIT_QUESTION(13, QT_IS_IT | QT_A_AN);
CREATE_EXIT_QUESTION(14, QT_IS_IT | QT_A_AN);
CREATE_EXIT_QUESTION(15, QT_IS_IT | QT_A_AN);
CREATE_EXIT_QUESTION(16, QT_IS_IT);
CREATE_EXIT_QUESTION(17, QT_IS_IT | QT_A_AN);
CREATE_EXIT_QUESTION(18, QT_IS_IT);
CREATE_EXIT_QUESTION(19, QT_IS_IT);
CREATE_EXIT_QUESTION(20, QT_IS_IT | QT_A_AN);
CREATE_EXIT_QUESTION(21, QT_IS_IT);
CREATE_EXIT_QUESTION(22, QT_IS_IT | QT_A_AN);
CREATE_EXIT_QUESTION(23, QT_IS_IT | QT_A_AN);
CREATE_EXIT_QUESTION(24, QT_IS_IT | QT_A_AN);
CREATE_EXIT_QUESTION(25, QT_IS_IT | QT_A_AN);
CREATE_EXIT_QUESTION(26, QT_IS_IT | QT_A_AN);
CREATE_EXIT_QUESTION(27, QT_IS_IT | QT_A_AN);
CREATE_EXIT_QUESTION(28, QT_IS_IT | QT_A_AN);
CREATE_EXIT_QUESTION(29, QT_IS_IT | QT_A_AN);
CREATE_EXIT_QUESTION(30, QT_IS_IT);
CREATE_EXIT_QUESTION(31, QT_IS_IT);
CREATE_EXIT_QUESTION(32, QT_IS_IT);
CREATE_EXIT_QUESTION(33, QT_IS_IT);
CREATE_EXIT_QUESTION(34, QT_IS_IT);
CREATE_EXIT_QUESTION(35, QT_IS_IT);

const PROGMEM QuestionNode ROM_EXIT_QUESTIONS[] = {
	__eq0, __eq1, __eq2, __eq3, __eq4, __eq5, __eq6, __eq7, __eq8, __eq9, __eq10,
	__eq11, __eq12, __eq13, __eq14, __eq15, __eq16, __eq17, __eq18, __eq19, __eq20,
	__eq21, __eq22, __eq23, __eq24, __eq25, __eq26, __eq27, __eq28, __eq29, __eq30,
	__eq31, __eq32, __eq33, __eq34, __eq35
};

// ----- GLobal variables -----

uint16_t eepromWriteAddress;
uint16_t currentAddress;

uint8_t currentQuestion;

bool run = true;

// ----- Functions -----

/**
 * @brief Checks if chr is a vowel
 * @param chr character to check
 * @return 1 if vowel (a, e, i, o, u), 0 otherwise
 */
uint8_t isVowel(char chr) {
	switch (tolower(chr)) {
		case 'a':
		case 'e':
		case 'i':
		case 'o':
		case 'u': return 1;
		default: return 0;
	}
}

/**
 * @brief Checks if a string starts with any of the QUESTION_TAGS (case-insensitive)
 * @param str text to compare
 * @param realText pointer to receive the address where the actual text (after tag + optional "a"/"an") starts
 * @return index of matched tag in QUESTION_TAGS, or -1 if no match
 * @remark made with ai with some touch ups
 */
int8_t matchQuestionTag(char *str, const char **realText) {
	*realText = str;

	for (uint8_t i = 1; i < LEN(QUESTION_TAGS); ++i) {
		const char *tag = (const char *)pgm_read_ptr_near(&QUESTION_TAGS[i]);

		// compare all tag character to str
		uint8_t j = 0;
		while (true) {
			char a = str[j];
			char b = pgm_read_byte_near(tag + j);

			// End of tag -> match found
			if (b == '\0') {
				const char *after = str + j;
				// Skip any spaces
				while (*after == ' ') ++after;

				// Check for "a " or "an "
				char c0 = after[0], c1 = after[1], c2 = after[2];
				if (c0 >= 'A' && c0 <= 'Z') c0 += 'a' - 'A';
				if (c1 >= 'A' && c1 <= 'Z') c1 += 'a' - 'A';

				if (c0 == 'a' && c1 == ' ') {
					*realText = after + 2;
					return GET_WITH_A_AN_FLAG(i);
				}
				if (c0 == 'a' && c1 == 'n' && c2 == ' ') {
					*realText = after + 3;
					return GET_WITH_A_AN_FLAG(i);
				}

				*realText = after;

				return i;
			}

			// End of input string or mismatch
			if (a == '\0') break;

			// Lowercase both if they are letters
			if (a >= 'A' && a <= 'Z') a += 'a' - 'A';
			if (b >= 'A' && b <= 'Z') b += 'a' - 'A';

			if (a != b) break;
			++j;
		}
	}
	return -1; // No match
}

/**
 * @brief Copies string from PROGMEM to heap
 * @param str PROGMEM string pointer
 * @return heap allocated string, or NULL on malloc error
 */
char *copyStrFromPROGMEM(const char *str) {
	char *buf = (char *)malloc(strlen_P(str) + sizeof(char));
	if(!buf) return NULL;
	return strcpy_P(buf, str);
}

/**
 * @brief Gets length of EEPROM string
 * @param address EEPROM based address
 * @return length, or 0 on error
 */
size_t strLenEEPROM(EepromAddress address) {
	if(address >= EEPROM.length()) return 0;

	size_t len = 0;
	for(EepromAddress i = address, n = EEPROM.length(); i < n; ++i) {
		if(EEPROM[i] == 0) break;
		++len;
	}

	if(len + address >= EEPROM.length()) return 0;
	return len;
}

/**
 * @brief Copies string (until len and ends string with '\0') from EEPROM to heap
 * @param address EEPROM based address
 * @return heap allocated string, or NULL on malloc error
 */
char *copyStrFromEEPROM(EepromAddress address, size_t len) {
	if ((uint32_t)address + len >= EEPROM.length()) return NULL;

	char *str = static_cast<char *>(malloc(len + sizeof(char)));
	if(!str) return NULL;

	for(EepromAddress i = 0; i < len; ++i) {
		str[i] = EEPROM[address + i];
	}

	str[len] = '\0';

	return str;
}

/**
 * @brief Writes string (until len, then writes '\0') to EEPROM
 * @param address EEPROM based address
 * @return 1 on success, or 0 on error
 */
uint8_t writeStrToEEPROM(char * str, EepromAddress address, size_t len) {
	if ((uint32_t)address + len >= EEPROM.length()) return 0;

	// write until len, so \0 gets copied too (hopefully)
	for(EepromAddress i = 0; i < len; ++i) {
		EEPROM.update(address + i, str[i]);
	}

	EEPROM.update(address + len, '\0');

	return 1;
}

/**
 * @brief Writes question into EEPROM at address
 * @param address EEPROM based address
 * @param question question to write
 * @return written bytes or 0 on error
 */
uint16_t writeQuestionToEEPROM(EepromAddress address, QuestionNode question) {
	uint16_t len = strlen(question.text);
	uint16_t size = EMPTY_QUESTION_SIZE + len;
	EepromAddress curAddress = address;

	if((uint32_t)curAddress + size >= EEPROM.length()) return 0;

	writeStrToEEPROM(question.text, curAddress, len); // dont need to check return -> already returned 0 if out of space
	curAddress += len + sizeof(char);

	EEPROM.put(curAddress, question.yesAddress);
	curAddress += sizeof(question.yesAddress);

	EEPROM.put(curAddress, question.noAddress);
	curAddress += sizeof(question.noAddress);

	EEPROM.put(curAddress, question.questionTag);
	curAddress += sizeof(question.questionTag);

	return curAddress - address;
}

/**
 * @brief Writes ROM_EXIT_QUESTIONS into start of EEPROM
 * @return 1 on success, or 0 on error
 */
uint8_t configureEEPROM() {
	Serial.println(F("Configuring EEPROM."));

	for(EepromAddress i = 0, n = LEN(ROM_EXIT_QUESTIONS); i < n; ++i) {
		EepromAddress address = i * EXIT_QUESTION_SIZE;
		if(address + EXIT_QUESTION_SIZE > EEPROM.length()) {
			Serial.print(i);
			Serial.println(F(". Question doesn't fit into EEPROM."));
			return 0;
		}

		QuestionNode progmemQuestion;
		memcpy_P(&progmemQuestion, &ROM_EXIT_QUESTIONS[i], sizeof(QuestionNode));

		progmemQuestion.text = copyStrFromPROGMEM(progmemQuestion.text);
		if(!progmemQuestion.text) {
			Serial.println(F("Out of memory! Please turn off to avoid corruption."));
			return 0;
		}

		uint16_t writtenBytes = writeQuestionToEEPROM(address, progmemQuestion);
		if(writtenBytes != EXIT_QUESTION_SIZE) {
			Serial.println(F("Question size mismatch! Please consult code."));
			return 0;
		}

		free(progmemQuestion.text);
	}

	return 1;
}

/**
 * @brief Searches EEPROM for free space (0xff == free)
 * @param searchFrom EEPROM based address
 * @param blockLength How much free byte is needed
 * @return address of block or 0xffff on error
 */
EepromAddress getFirstFreeSpace(EepromAddress searchFrom, uint16_t blockLength) {
	if((uint32_t)searchFrom + blockLength >= EEPROM.length()) return 0xffff;
	if(blockLength == 0) return searchFrom;

	uint16_t cellCounter = 0;
	for(EepromAddress address = searchFrom, n = EEPROM.length(); address < n; ++address) {
		if(EEPROM[address] != 0xff) {
			cellCounter = 0;
			continue;
		}

		++cellCounter;
		if(cellCounter == blockLength) {
			return address - cellCounter + 1; // get address of block start
		}
	}

	return 0xffff;
}

/**
 * @brief Gets question from PROGMEM (text stored on heap)
 * @param address ROM_QUESTIONS index
 * @return question
 */
QuestionNode fetchROMQuestion(uint16_t address) {
	QuestionNode question;

	QuestionNode progmemQuestion;
	memcpy_P(&progmemQuestion, &ROM_QUESTIONS[address], sizeof(QuestionNode));

	question.yesAddress = progmemQuestion.yesAddress;
	question.noAddress = progmemQuestion.noAddress;
	question.questionTag = progmemQuestion.questionTag;

	question.text = copyStrFromPROGMEM(progmemQuestion.text);

	return question;
}

/**
 * @brief Gets question from EEPROM. Gets text from ROM_TEXTS if len(text) == 1
 * @param address EEPROM based address
 * @return question. text is NULL on error
 */
QuestionNode fetchEEPROMQuestion(EepromAddress address) {
	QuestionNode question;

	size_t len = strLenEEPROM(address);
	question.text = copyStrFromEEPROM(address, len);
	if(question.text == NULL) return question;

	if(len == 1) {
		uint8_t index = question.text[0];
		free(question.text);
		question.text = copyStrFromPROGMEM((const char *)pgm_read_ptr_near(&ROM_TEXTS[index]));
	}

	EepromAddress offset = address + len + sizeof(char);
	EEPROM.get(offset + 0, question.yesAddress);
	EEPROM.get(offset + 2, question.noAddress);
	EEPROM.get(offset + 4, question.questionTag);

	return question;
}

/**
 * @brief Abstracted question getter
 * @param address absolute address
 * @return question, text is NULL on error
 */
QuestionNode fetchQuestion(uint16_t address) {
	if(IS_EEPROM_ADDRESS(address))
		return fetchEEPROMQuestion(GET_EEPROM_BASED_ADDRESS(address));
	else
		return fetchROMQuestion(address);
}

/**
 * @brief Prints question with help of QUESTION_TAG and currentQuestion
 */
void printQuestion(QuestionNode question) {
	Serial.print('[');
	Serial.print(currentQuestion);
	Serial.print(F("]: "));

	if(question.questionTag != 0) {
		uint8_t questionTag = GET_REAL_QUESTION_TYPE(question.questionTag);
		Serial.print(reinterpret_cast<const __FlashStringHelper *>(pgm_read_ptr_near(&QUESTION_TAGS[questionTag])));
		if(IS_A_AN_FLAG_PRESENT(question.questionTag))
			Serial.print(isVowel(question.text[0]) ? F("an ") : F("a "));
	}

	Serial.print(question.text);

	Serial.print('?');
	Serial.print(' ');
}

/**
 * @brief Clears Serial's input buffer, with delays to wait for not yet sent bytes
 */
void clearInputBuffer() {
	uint8_t retries = 0;

	while(++retries <= 3) {
		delay(50);
		while(Serial.available()) { Serial.read(); }
	}
}

/**
 * @brief Gets "yes" or "no" (case-insensitive) answer from user. Prompts until successful. (Dont print new line before this)
 * @return 0 == "no", 1 == "yes"
 */
uint8_t getAnswer() {
	uint8_t answer;

	while (true) {
		while(!Serial.available()) {}

		switch(Serial.read()) {
			case 'y':
			case 'Y':
				Serial.println(F("yes"));
				answer = 1;
				break;
			case 'N':
			case 'n':
				Serial.println(F("no"));
				answer = 0;
				break;
			case 'a':
			case 'A':
				Serial.println(F("\nDear friend\nHappy birthday!\nHope you'll enjoy this little thingamajig.\nYou can find the code here: 'https://github.com/almafa64/arduino-barkochba'."));
				break;
		}

		clearInputBuffer();

		if(answer != 0xff) {
			return answer;
		}
		Serial.println(F("\nOnly 'yes' or 'no'!"));
	}
}

/**
 * @brief Gets input from user through serial to buf. Reads until '\\n' or '\\r'. Retries when input exceeds buffer size
 * @param buf buffer to write into
 * @param size size of buffer (not length)
 * @param minCharacters at least this many characters needed
 * @return buf or NULL on error
 */
char *getInput(char *buf, uint16_t size, uint16_t minCharacters) {
	if(size == 0) return NULL;
	if(minCharacters >= size) return NULL;

	uint16_t count = 0;

	while(true) {
		while(Serial.available() > 0) {
			buf[count] = '\0';

			char chr = Serial.read();

			if(chr == '\n' || chr == '\r') {
				Serial.println();
				clearInputBuffer();

				if(count < minCharacters) {
					count = 0;
					Serial.print(F("At least "));
					Serial.print(minCharacters);
					Serial.println(F(" character(s) needed! Please make it longer."));
					break;
				} else {
					return buf;
				}
			}

			if(chr == '\b' or chr == 127) {
				if(count != 0) {
					Serial.print(F("\b \b")); // go back, delete character, go back again
					--count;
				}
				continue;
			}

			Serial.print(chr);
			buf[count] = chr;

			++count;
			if(count >= size) {
				clearInputBuffer();
				count = 0;
				Serial.print(F("Maximum input is "));
				Serial.print(size - 1);
				Serial.println(F(" character(s)! Please shorten it."));
				break;
			}
		}
	}
}

/**
 * @brief Makes new question and appends it to EEPROM
 * @param currentQuestion current question (needed to modify its address)
 * @param branchToUpdate updates noAddress (false) or yesAddress (true)
 */
void makeNewQuestionNode(QuestionNode currentQuestion, bool branchToUpdate) {
	if(eepromWriteAddress == 0xffff) {
		Serial.print(F("EEPROM is out of space."));
		Serial.println(F(" Skipping making new question."));
		return;
	}

	if(!IS_EEPROM_ADDRESS(currentAddress)) {
		Serial.print(F("Tried to modify ROM question at address: "));
		Serial.println(currentAddress);
		Serial.println(F(" Skipping making new question."));
		return;
	}

	Serial.println(F("To save some space there are hardcoded question starters. Please use these if you can:"));
	for(uint8_t i = 1, n = LEN(QUESTION_TAGS); i < n; ++i) {
		char *tag = copyStrFromPROGMEM((char *)pgm_read_ptr_near(&QUESTION_TAGS[i]));
		Serial.print(tag);
		Serial.println(F(" [a/an]"));
		free(tag);
	}

	char str[51];
	char *strippedStr;
	uint8_t index;

	while(true) {
		Serial.print("How would you ask a question about it: ");

		getInput(str, sizeof(str), 2);

		index = matchQuestionTag(str, &strippedStr);

		size_t len = strlen(strippedStr);
		if(len < 2) {
			Serial.println(F("Too short. Please write more characters."));
			continue;
		}

		if(strippedStr[len - 1] == '?')
			strippedStr[len - 1] = '\0';

		break;
	}

	QuestionNode newQuestion;
	newQuestion.questionTag = (index == (uint8_t)-1) ? 0 : index;
	newQuestion.text = strippedStr;
	newQuestion.noAddress = 0;
	newQuestion.yesAddress = 0;

	uint16_t writtenBytes = writeQuestionToEEPROM(eepromWriteAddress, newQuestion);
	if(writtenBytes == 0) {
		Serial.print(F("EEPROM is out of space."));
		Serial.println(F(" Skipping making new question."));
		return;
	}

	if(branchToUpdate)
		currentQuestion.yesAddress = GET_ABSOLUTE_ADDRESS(eepromWriteAddress);
	else
		currentQuestion.noAddress = GET_ABSOLUTE_ADDRESS(eepromWriteAddress);

	writeQuestionToEEPROM(GET_EEPROM_BASED_ADDRESS(currentAddress), currentQuestion);

	eepromWriteAddress += writtenBytes;
}

/**
 * @brief repeated part of yesJump and noJump
 * @return 1 if jumped, 0 otherwise
 */
uint8_t generalJump(uint16_t address) {
	if(address != 0) {
		currentAddress = address;
		return 1;
	}

	run = false;
	Serial.println();
	return 0;
}

/**
 * @brief follows yesAddress if not 0, asks for new question otherwise
 */
void yesJump(QuestionNode question) {
	if(generalJump(question.yesAddress)) return;

	Serial.print(F("Did i figure it out your choice? "));
	if(getAnswer()) {
		Serial.print(F("GG! "));
	} else {
		Serial.print(F("Do you want to add it? "));
		if(getAnswer()) makeNewQuestionNode(question, true);
	}

	Serial.print(F("\nDo you want to play again? "));
	if(getAnswer()) setup();
}

/**
 * @brief follows noAddress if not 0, asks for new question otherwise
 */
void noJump(QuestionNode question) {
	if(generalJump(question.noAddress)) return;

	Serial.println(F("I couldn't figure out your choice."));

	Serial.print(F("Do you want to add it? "));
	if(getAnswer()) makeNewQuestionNode(question, false);

	Serial.print(F("Can i try guessing again? "));
	if(getAnswer()) setup();
}

/**
 * @brief Resets all EEPROM data to 0xff
 */
void clearEEPROM() {
	for(EepromAddress i = 0; i < EEPROM.length(); ++i) {
		EEPROM.update(i, 0xff);
	}
}

void setup() {
	// run can only be true here at bootup
	if(run) {
		Serial.begin(9600);
		delay(500);

		#ifdef CLEAR_RUN
		clearEEPROM();
		configureEEPROM();
		#endif

		eepromWriteAddress = getFirstFreeSpace(0, EMPTY_QUESTION_SIZE); // size doesnt really matter, just be bigger than 1
		if(eepromWriteAddress == 0xffff)
			Serial.println(F("EEPROM is out of free space. New question function is turned off."));
	}

	currentAddress = 0;
	currentQuestion = 0;
	run = true;
	Serial.println();
}

void loop() {
	if(!run) return;

	++currentQuestion;

	QuestionNode question = fetchQuestion(currentAddress);
	if(question.text == NULL) {
		Serial.println(F("Can't allocate memory for question string. Stopping."));
		run = false;
		return;
	}

	printQuestion(question);

	if(getAnswer()) {
		yesJump(question);
	} else {
		noJump(question);
	}

	free(question.text);
}