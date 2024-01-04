#include "Arduino.h"
#include "ft8/ft8_message.h" 
#include "ft8/ft8_constants.h"
#include "ft8/ft8_encode.h"

/* This file holds functions to generate FT8 symbols from a Message

Thanks to YL3JG for the ft8_lib this is all based on, see ft8_ files.

https://github.com/kgoba/ft8_lib/tree/master

*/

extern "C" uint8_t* getFT8SymbolsFromText(const char* message, bool isFreeMessage = false)
{
    ftx_message_t msg;
    ftx_message_rc_t rc;
    if (isFreeMessage)
    {
        rc = ftx_message_encode_free(&msg, message);
    } else {
        rc = ftx_message_encode(&msg, NULL, message);
    }
    
    if (rc != FTX_MESSAGE_RC_OK)
    {
        Serial.println('Cannot parse message!');
        Serial.printf("RC = %d\n", (int)rc);
    }

    Serial.printf("Packed data: ");
    for (int j = 0; j < 10; ++j)
    {
        Serial.printf("%02x ", msg.payload[j]);
    }
    Serial.printf("\n");

    static uint8_t tones[FT8_NN]; // Array of 79 tones (symbols) for FT8
    ft8_encode(msg.payload, tones);

    Serial.printf("FSK tones: ");
    for (int j = 0; j < FT8_NN; ++j)
    {
        Serial.printf("%d", tones[j]);
    }
    Serial.printf("\n");

    return tones;
}