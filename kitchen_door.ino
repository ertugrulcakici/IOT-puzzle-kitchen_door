#include <SPI.h>
#include <MFRC522.h>

/*
sda
10 13 11 12 boş gnd 9 3v 
*/



#define SS_PIN 10
#define RST_PIN 9


#define door_push 7
#define door_pull 6

#define fan_role 5
#define fan_button 4

String correctCard = "321953936";
String correctCardBackup = "21020213228";
String gameCards[] = {
    "2272813922",
    "19510723820",
    "322414536",
    "6721018522"};

MFRC522 mfrc522[1];

String wait_until_read();
String dump_byte_array(byte *buffer, byte bufferSize);
bool is_game_card(String cardId);

void fan_active(bool active);
void open_door(bool active);

String get_card_string();

void setup()
{
#ifdef DEBUG
    Serial.begin(9600);
    Serial.println("Start");
#endif

    pinMode(door_push, OUTPUT);
    pinMode(door_pull, OUTPUT);
    pinMode(fan_role, OUTPUT);

    SPI.begin();
    mfrc522[0].PCD_Init(SS_PIN, RST_PIN);
    //mfrc522[0].PCD_DumpVersionToSerial();
}

void loop()
{
#ifdef DEBUG
    Serial.print(F("Waiting for card...\n"));
#endif
    String cardId = wait_until_read();
#ifdef DEBUG
    Serial.print(F("Card ID: "));
    Serial.println(cardId);
#endif
    if (cardId == correctCard || cardId == correctCardBackup)
    {
#ifdef DEBUG
        Serial.print(F("Correct card\n"));
#endif
        fan_active(true);
        open_door(true);

        while (!digitalRead(fan_button))
        {
#ifdef DEBUG
            Serial.print(F("Button state: "));
            Serial.println(digitalRead(fan_button));
#endif
            if (mfrc522[0].PICC_IsNewCardPresent() && mfrc522[0].PICC_ReadCardSerial())
            {
                // print "new card readed"
                String new_card = get_card_string();
                #ifdef DEBUG
                Serial.print(F("New card readed: "));
                Serial.println(new_card);

                #endif
                bool cond1 = is_game_card(new_card);
                bool cond2 = (new_card == correctCard);
                bool cond3 = (new_card == correctCardBackup);
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
                if (cond1 == false && cond2 == false && cond3 == false)
                {
                    #ifdef DEBUG
                    Serial.println(F("Admin card"));
                    #endif
                    open_door(false);
                    break;
                }
            } else 
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
        fan_active(false);
    }
    else if (is_game_card(cardId))
    {
        #ifdef DEBUG
        Serial.println(F("Wrong card"));
        #endif
    } else {
        #ifdef DEBUG
        Serial.println(F("Admin card"));
        #endif
        fan_active(false);
        open_door(false);
    }
    delay(100);
}

String wait_until_read()
{
    while (true)
    {
        if (mfrc522[0].PICC_IsNewCardPresent() && mfrc522[0].PICC_ReadCardSerial())
        {
            return dump_byte_array(mfrc522[0].uid.uidByte, mfrc522[0].uid.size);
            mfrc522[0].PICC_HaltA();
            mfrc522[0].PCD_StopCrypto1();
            break;
        }
        delay(100);
    }
}

String dump_byte_array(byte *buffer, byte bufferSize)
{
    String data = "";
    for (byte i = 0; i < bufferSize; i++)
    {
        data += (String)buffer[i];
    }
    return data;
}

bool is_game_card(String cardId)
{
    for (int i = 0; i < sizeof(gameCards) / sizeof(gameCards[0]); i++)
    {
        if (cardId == gameCards[i])
        {
            return true;
        }
    }
    return false;
}

void fan_active(bool active)
{
    if (active)
    {
        digitalWrite(fan_role, HIGH);
    }
    else
    {
        digitalWrite(fan_role, LOW);
    }
}

void open_door(bool active)
{
    if (active)
    {
        digitalWrite(door_push, HIGH);
        digitalWrite(door_pull, LOW);
        delay(1000);
        digitalWrite(door_push, LOW);
    }
    else
    {
        digitalWrite(door_push, LOW);
        digitalWrite(door_pull, HIGH);
        delay(1000);
        digitalWrite(door_pull, LOW);
    }
}

String get_card_string() {
    return dump_byte_array(mfrc522[0].uid.uidByte, mfrc522[0].uid.size);
}
