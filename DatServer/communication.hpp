#ifndef COMMUNICATION_H
#define COMMUNICATION_H
#include "MessageIdentifiers.h"

#define SERVER_PORT 13399
#define ENABLE_CHECK_SUM 1
typedef unsigned long long CHECKSUM_TYPE;
enum GameMessages
{
    ID_REQUIRE_BG_IMG = ID_USER_PACKET_ENUM,
    ID_RESPONSE_BG_IMG = ID_USER_PACKET_ENUM+1
};

#pragma pack(push)
#pragma pack(1)
typedef struct {
    unsigned char typeId;
    unsigned char scale;
    unsigned char layer;
    unsigned char priority;
    unsigned short x, y;    
} ReqBkImgPkt;

typedef struct {
    unsigned char typeId;
    unsigned char scale;
    unsigned short x, y;
    unsigned int len;
    unsigned char layer;
#if ENABLE_CHECK_SUM & 1
    CHECKSUM_TYPE check_sum;
#endif
} RspBkImgPkt;
#pragma pack(pop)
#endif // COMMUNICATION_H

