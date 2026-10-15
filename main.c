#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "LPC24xx.h"
#include "glcd.h"
#include "system_init.h"
#include "mfrc522.h"


static void to_hex2(uint8_t value, char *output)
{
    static const char hex_digits[] = "0123456789ABCDEF";

    output[0] = hex_digits[(value >> 4) & 0x0F];
    output[1] = hex_digits[value & 0x0F];
    output[2] = '\0';
}


static void show_uid(const MFRC522_UID *uid)
{
    char row[64];
    char value_text[12];
    uint8_t i;

    /*
     * Display UID in hexadecimal format.
     * Example: A3 5F 21 9C
     */
    strcpy(row, "HEX:");

    for (i = 0; i < uid->size; i++)
    {
        strcat(row, "  ");

        to_hex2(uid->uidByte[i], value_text);

        strcat(row, value_text);
    }

    GLCD_RowWriteMargin(4, row);


    /*
     * Display each UID byte in decimal format.
     * Example: 163 95 33 156
     */
    strcpy(row, "DEC:");

    for (i = 0; i < uid->size; i++)
    {
        snprintf(
            value_text,
            sizeof(value_text),
            " %u",
            (unsigned int)uid->uidByte[i]
        );

        strcat(row, value_text);
    }

    GLCD_RowWriteMargin(5, row);


    /*
     * Display SAK in hexadecimal format.
     */
    to_hex2(uid->sak, value_text);

    strcpy(row, "SAK: 0x");
    strcat(row, value_text);

   //GLCD_RowWriteMargin(6, row);

    GLCD_RowWriteMargin(7, "    CARD READ OK    ");
}


int main(){


    PLL_Init();
    system_Init();

    GLCD_Init();
    GLCD_Clear();

    GLCD_RowWriteMargin(0, "WELCOME TO AKADEMIKA");
    GLCD_RowWriteMargin(2, "   RFID READER      ");
    GLCD_RowWriteMargin(3, "   TAP A CARD       ");

    PCD_Init();
    MFRC522_UID uid;

    uint8_t last_uid[10] = {0};
    uint8_t last_uid_size = 0;
    uint8_t have_last = 0;
    uint8_t i;
    uint8_t same_uid;
    for (;;)
    {
        if (!PICC_IsNewCardPresent())
        {
            continue;
        }

        if (!PICC_ReadCardSerial(&uid))
        {
            continue;
        }

        same_uid = 1;

        if (!have_last || uid.size != last_uid_size)
        {
            same_uid = 0;
        }
        else
        {
            for (i = 0; i < uid.size; i++)
            {
                if (uid.uidByte[i] != last_uid[i])
                {
                    same_uid = 0;
                    break;
                }
            }
        }

        if (!same_uid)
        {
            show_uid(&uid);

            for (i = 0; i < uid.size && i < sizeof(last_uid); i++)
            {
                last_uid[i] = uid.uidByte[i];
            }

            last_uid_size = uid.size;
            have_last = 1;
        }

        PICC_HaltA();
    }
}

