#ifndef MARKOBJ_H
#define MARKOBJ_H
#include <QPoint>

#pragma pack(push)
#pragma pack(1)

struct MarkObj {
    unsigned char type;
    unsigned char type2;
    unsigned char type3;
    unsigned char select_state;
    QPoint p0, p1;
    float prob;
};

#pragma pack(pop)

enum {
    OBJ_NONE=0,
    OBJ_AREA,
    OBJ_WIRE,
    OBJ_VIA,
    SELECT_OBJ
};

enum {
    AREA_LEARN=0,
    AREA_METAL,
    AREA_CELL
};

#define POWER_UP_L      1
#define POWER_UP_R      2
#define POWER_DOWN_L    4
#define POWER_DOWN_R    8
#define POWER_LEFT_U    16
#define POWER_LEFT_D    32
#define POWER_RIGHT_U   64
#define POWER_RIGHT_D   128

#define	POWER_UP		(POWER_UP_L | POWER_UP_R)
#define POWER_DOWN		(POWER_DOWN_L | POWER_DOWN_R)
#define	POWER_LEFT		(POWER_LEFT_U | POWER_LEFT_D)
#define	POWER_RIGHT		(POWER_RIGHT_U | POWER_RIGHT_D)


#endif // MARKOBJ_H

