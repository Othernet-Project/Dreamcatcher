#ifndef _PLAGEOJ_URLENCODE_H
#define _PLAGEOJ_URLENCODE_H

#include <Arduino.h>

/**
 * Percent-encodes a string.
 * @param msg UTF-8 string to encode.
 * @returns Percent-encoded string.
 */
inline String urlEncode(const char *msg)
{
    const char *hex = "0123456789ABCDEF";
    String encodedMsg = "";

    while (*msg != '\0')
    {
        if (('a' <= *msg && *msg <= 'z') || ('A' <= *msg && *msg <= 'Z') || ('0' <= *msg && *msg <= '9') || *msg == '-' || *msg == '_' || *msg == '.' || *msg == '~')
        {
            encodedMsg += *msg;
        }
        else
        {
            encodedMsg += '%';
            encodedMsg += hex[*msg >> 4];
            encodedMsg += hex[*msg & 0xf];
        }
        msg++;
    }
    return encodedMsg;
}
inline String urlEncode(String msg)
{
    return urlEncode(msg.c_str());
}

#endif