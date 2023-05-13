#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9

#define DOOR_PUSH_PIN 7
#define DOOR_PULL_PIN 6

#define FAN_ROLE_PIN 5
#define FAN_BUTTON_PIN 4

String masterCard = "321953936";
String backupMasterCard = "21020213228";
String gameCardIds[] = {
    "2272813922",
    "19510723820",
    "322414536",
    "6721018522"};

MFRC522 mfrc522[1];

// Function prototypes
String waitUntilCardRead();
String convertByteArrayToString(byte *buffer, byte bufferSize);
bool isGameCard(String cardId);

void setFanActive(bool active);
void openDoor(bool active);

String getCardIdString();

void setup()
{
#ifdef DEBUG
    Serial.begin(9600);
    Serial.println("Start");
#endif

    pinMode(DOOR_PUSH_PIN, OUTPUT);
    pinMode(DOOR_PULL_PIN, OUTPUT);
    pinMode(FAN_ROLE_PIN, OUTPUT);

    SPI.begin();
    mfrc522[0].PCD_Init(SS_PIN, RST_PIN);
    // mfrc522[0].PCD_DumpVersionToSerial();
}

void loop()
{
#ifdef DEBUG
    Serial.print(F("Waiting for card...\n"));
#endif
    String cardId = waitUntilCardRead();
#ifdef DEBUG
    Serial.print(F("Card ID: "));
    Serial.println(cardId);
#endif
    if (cardId == masterCard || cardId == backupMasterCard)
    {
#ifdef DEBUG
        Serial.print(F("Master card detected\n"));
#endif
        setFanActive(true);
        openDoor(true);

        while (!digitalRead(FAN_BUTTON_PIN))
        {
#ifdef DEBUG
            Serial.print(F("Button state: "));
            Serial.println(digitalRead(FAN_BUTTON_PIN));
#endif
            if (mfrc522[0].PICC_IsNewCardPresent() && mfrc522[0].PICC_ReadCardSerial())
            {
                // print "new card readed"
                String new_card = getCardIdString();
#ifdef DEBUG
                Serial.print(F("New card read: "));
                Serial.println(new_card);
#endif
                bool cond1 = isGameCard(new_card);
                bool cond2 = (new_card == masterCard);
                bool cond3 = (new_card == backupMasterCard);
// print all conditions in one line
#ifdef DEBUG
                Serial.print(F("cond1: "));
                Serial.print(cond1);
                Serial.print(F(" cond2: "));
                Serial.print(cond2);
                Serial.print(F(" cond3: "));
                Serial.println(cond3);
                delay(1000);
#endif
                if (!cond1 && !cond2 && !cond3)
                {
#ifdef DEBUG
                    Serial.println(F("Admin card"));
#endif
                    openDoor(false);
                    break;
                }
            }
            else
            {
#ifdef DEBUG
                Serial.println(F("No new card"));
#endif
            }
            delay(100);
        } // end of while
#ifdef DEBUG
        Serial.println(F("Button released"));
#endif
        setFanActive(false);
    }
    else if (isGameCard(cardId))
    {
#ifdef DEBUG
        Serial.println(F("Wrong card"));
#endif
    }
    else
    {
#ifdef DEBUG
        Serial.println(F("Admin card"));
#endif
        setFanActive(false);
        openDoor(false);
    }
    delay(100);
}

String waitUntilCardRead()
{
    while (true)
    {
        if (mfrc522[0].PICC_IsNewCardPresent() && mfrc522[0].PICC_ReadCardSerial())
        {
            String cardData = convertByteArrayToString(mfrc522[0].uid.uidByte, mfrc522[0].uid.size);
            mfrc522[0].PICC_HaltA();
            mfrc522[0].PCD_StopCrypto1();
            return cardData;
        }
        delay(100);
    }
}

String convertByteArrayToString(byte *buffer, byte bufferSize)
{
    String data = "";
    for (byte i = 0; i < bufferSize; i++)
    {
        data += (String)buffer[i];
    }
    return data;
}

bool isGameCard(String cardId)
{
    // Check if the card is a game card
    for (int i = 0; i < sizeof(gameCardIds) / sizeof(gameCardIds[0]); i++)
    {
        if (cardId == gameCardIds[i])
        {
            return true;
        }
    }
    return false;
}

void setFanActive(bool active)
{
    // Activate or deactivate the fan
    digitalWrite(FAN_ROLE_PIN, active ? HIGH : LOW);
}

void openDoor(bool active)
{
    // Open or close the door
    if (active)
    {
        digitalWrite(DOOR_PUSH_PIN, HIGH);
        digitalWrite(DOOR_PULL_PIN, LOW);
        delay(1000);
        digitalWrite(DOOR_PUSH_PIN, LOW);
    }
    else
    {
        digitalWrite(DOOR_PUSH_PIN, LOW);
        digitalWrite(DOOR_PULL_PIN, HIGH);
        delay(1000);
        digitalWrite(DOOR_PULL_PIN, LOW);
    }
}

String getCardIdString()
{
    return convertByteArrayToString(mfrc522[0].uid.uidByte, mfrc522[0].uid.size);
}
