#ifndef COMMUNICATION_H
#define COMMUNICATION_H
#include "MessageIdentifiers.h"

#define IMAGE_MAX_SCALE			4
#define SERVER_PORT				13399
#define ENABLE_CHECK_SUM_BKIMG	1
#define ENABLE_CHECK_SUM		0
#define BKIMAGE_STREAM			0
#define ELEMENT_STREAM			1

typedef unsigned long long CHECKSUM_TYPE;
enum GameMessages
{
    ID_REQUIRE_BG_IMG = ID_USER_PACKET_ENUM,
    ID_RESPONSE_BG_IMG = ID_USER_PACKET_ENUM + 1,
    ID_REQUIRE_IMG_INFO = ID_USER_PACKET_ENUM + 2,
    ID_RESPONSE_IMG_INFO = ID_USER_PACKET_ENUM + 3,
    ID_REQUIRE_OBJ_SEARCH = ID_USER_PACKET_ENUM + 4,
    ID_RESPONSE_OBJ_SEARCH = ID_USER_PACKET_ENUM + 5
};

enum SEARCH_COMMAND
{
    CELL_TRAIN,
    CELL_EXTRACT,
    VW_EXTRACT
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
#if ENABLE_CHECK_SUM_BKIMG & ENABLE_CHECK_SUM
    CHECKSUM_TYPE check_sum;
#endif
} RspBkImgPkt;

typedef struct {
    unsigned char typeId;
} ReqImgInfoPkt;

typedef struct {
    unsigned char typeId;
    int img_block_w;
    int img_block_h;
    int num_block_x;
    int num_block_y;
    int num_layer;
} RspImgInfoPkt;

typedef struct {
    unsigned short opt;
    int x0, y0;
    int x1, y1;
    float prob;
} Location;

typedef struct {
    int parami[8];
    float paramf[5];
    Location loc[1];
} ReqSearchParam;

typedef struct {
    unsigned char typeId;
    unsigned char command;
    unsigned short req_search_num;
    ReqSearchParam params[0];
} ReqSearchPkt;

typedef struct {
    unsigned char typeId;
    unsigned char command;
    unsigned rsp_search_num;
    Location result[0];
} RspSearchPkt;

#pragma pack(pop)
#endif // COMMUNICATION_H

