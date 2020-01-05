#ifndef MARKOBJ_H
#define MARKOBJ_H
#include <QPoint>

#pragma pack(push)
#pragma pack(1)

struct MarkObj {
    unsigned char type;
    unsigned char type2;
    unsigned char type3;
    unsigned char state;
    QPoint p0, p1;
    float prob;
};

#pragma pack(pop)

enum {    
    OBJ_AREA,
    OBJ_LINE,
    OBJ_POINT,
    OBJ_PARA,
	OBJ_NONE,
    SELECT_OBJ
};

enum {
    AREA_LEARN,
	AREA_EXTRACT,
    AREA_METAL,
    AREA_CELL,
	AREA_CHECK_ERR
};

enum {
    POINT_NORMAL_VIA0,
    POINT_NORMAL_VIA1,
    POINT_NORMAL_VIA2,
    POINT_NORMAL_VIA3,
	POINT_VIA_AUTO_EXTRACT,
	POINT_VIA_AUTO_EXTRACT1,
	POINT_VIA_AUTO_EXTRACT2,
	POINT_VIA_AUTO_EXTRACT3,
	POINT_NO_VIA,
    POINT_CELL,
	POINT_WORD
};

enum {
    LINE_NORMAL_WIRE0,
    LINE_NORMAL_WIRE1,
    LINE_NORMAL_WIRE2,
    LINE_NORMAL_WIRE3,
	LINE_WIRE_AUTO_EXTRACT,
	LINE_WIRE_AUTO_EXTRACT1,
	LINE_WIRE_AUTO_EXTRACT2,
	LINE_WIRE_AUTO_EXTRACT3,
};

enum {
    PARA_LINE_WIDTH,
    PARA_VIA_RADIUS,
	PARA_EXTRACT_RULE
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

#define OBJ_AREA_MASK	(1<<OBJ_AREA)
#define OBJ_LINE_MASK	(1<<OBJ_LINE)
#define OBJ_POINT_MASK	(1<<OBJ_POINT)
#define OBJ_WORD_MASK	(1<<OBJ_WORD)
#define OBJ_PARA_MASK	(1<<OBJ_PARA)

#define AREA_LEARN_MASK		(1<<AREA_LEARN)
#define AREA_EXTRACT_MASK	(1<<AREA_EXTRACT)
#define AREA_METAL_MASK		(1<<AREA_METAL)
#define AREA_CELL_MASK		(1<<AREA_CELL)
#define AREA_CHECK_ERR_MASK (1<<AREA_CHECK_ERR)

#define POINT_NORMAL_VIA0_MASK	(1<<POINT_NORMAL_VIA0)
#define POINT_NORMAL_VIA1_MASK	(1<<POINT_NORMAL_VIA1)
#define POINT_NORMAL_VIA2_MASK	(1<<POINT_NORMAL_VIA2)
#define POINT_NORMAL_VIA3_MASK	(1<<POINT_NORMAL_VIA3)
#define POINT_VIA_AUTO_EXTRACT_MASK		(1<<POINT_VIA_AUTO_EXTRACT)
#define POINT_VIA_AUTO_EXTRACT1_MASK	(1<<POINT_VIA_AUTO_EXTRACT1)
#define POINT_NO_VIA_MASK		(1<<POINT_NO_VIA)
#define POINT_CELL_MASK			(1<<POINT_CELL)

#define LINE_NORMAL_WIRE0_MASK	(1<<LINE_NORMAL_WIRE0)
#define LINE_NORMAL_WIRE1_MASK	(1<<LINE_NORMAL_WIRE1)
#define LINE_NORMAL_WIRE2_MASK	(1<<LINE_NORMAL_WIRE2)
#define LINE_NORMAL_WIRE3_MASK	(1<<LINE_NORMAL_WIRE3)
#define LINE_WIRE_AUTO_EXTRACT_MASK	(1<<LINE_WIRE_AUTO_EXTRACT)

#define PARA_LINE_WIDTH_MASK	(1<<PARA_LINE_WIDTH)
#define PARA_VIA_RADIUS_MASK	(1<<PARA_VIA_RADIUS)

#define RULE_NO_LOOP			1
#define RULE_NO_UCONN			2
#define RULE_NO_hCONN			4
#define RULE_NO_HCONN			8
#define RULE_NO_FCONN			0x10
#define RULE_NO_fCONN			0x20
#define RULE_NO_TT_CONN			0x40
#define RULE_NO_XT_CONN			0x80
#define RULE_NO_XX_CONN			0x100
#define RULE_NO_X_POINT			0x200
#define RULE_NO_T_POINT			0x400
#define RULE_NO_ADJ_VIA_CONN	0x800
#define RULE_VIA_NO_LCONN		0x1000
#define RULE_POLY_LAYER			0x2000
#define RULE_NO_DUMY_FILLING	0x4000
#define RULE_END_WITH_VIA		0x80000000
#define RULE_EXTEND_VIA_OVERLAP 0x40000000


#define SEARCH_NET_MASK			1
#define SEARCH_DIR_MASK			(1 << 1)
#endif // MARKOBJ_H

