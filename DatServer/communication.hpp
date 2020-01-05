#ifndef COMMUNICATION_H
#define COMMUNICATION_H
#include "MessageIdentifiers.h"

#define IMAGE_MAX_SCALE			4
#define SERVER_PORT				13399
#define ENABLE_CHECK_SUM_BKIMG	1
#define ENABLE_CHECK_SUM		0
#define BKIMAGE_STREAM			0
#define ELEMENT_STREAM			1
#define SINGLE_WIRE_STREAM      2

typedef unsigned long long CHECKSUM_TYPE;
enum GameMessages
{
    ID_REQUIRE_OBJ_SEARCH = ID_USER_PACKET_ENUM,
    ID_RESPONSE_OBJ_SEARCH = ID_USER_PACKET_ENUM + 1
};

enum SEARCH_COMMAND
{
    CELL_TRAIN,
    CELL_EXTRACT,
    VW_EXTRACT,
    SINGLE_WIRE_EXTRACT,
	VWML_TRAIN,
	VWML_EXTRACT,
	SHUT_DOWN
};

#pragma pack(push)
#pragma pack(1)
typedef struct {
    unsigned short opt;
    int x0, y0;
    int x1, y1;
    float prob;
} Location;

typedef struct {
    int parami[11];
    float paramf;
    Location loc[1];
} ReqSearchParam;

typedef struct {	
    unsigned char typeId;
    unsigned char command;
    unsigned char prj_file[256];
	unsigned int token;
    unsigned short req_search_num;
    ReqSearchParam params[0];
} ReqSearchPkt;

typedef struct {
    unsigned char typeId;
    unsigned char command;
	unsigned int token;
    unsigned rsp_search_num;
    Location result[0];
} RspSearchPkt;

#pragma pack(pop)
#endif // COMMUNICATION_H

