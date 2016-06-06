#ifndef ELEMENT_H
#define ELEMENT_H

#include <QLine>
#include <vector>
#include <string.h>
using namespace std;

#define GET_FIELD(var, field) (((var) & field##_MASK) >> field##_SHIFT)
#define SET_FIELD(var, field, num) var = ((unsigned)var & ~(field##_MASK)) | (((unsigned)(num) << field##_SHIFT) & field##_MASK)
#define MAKE_U32(y, x) (((unsigned long) (y) <<16) | (x))
#define MAKE_U64(y, x) (((unsigned long long) (y) <<32) | (x))


enum {
	//following is for line and line relation
	PARALLEL,           // //
	NOINTERSECTION,     // \/
	CENTERINTERSECTION, // X
	BOUNDINTERSECTION1, // T heng
	BOUNDINTERSECTION2, // T heng
	TEXTEND1,           // T shu
	TEXTEND2,           // T shu
	LEXTEND1,           // L
	LEXTEND2,           // L
	INCLUDE,            // [()]
	INCLUDEEDBY,        // ([])
	EXTEND1,            // ([)]
	EXTEND2,            // [(])
	//following is for line and rect relation
	P0INSIDE,
	P1INSIDE,
	BOTHINSIDE,
	CROSSINTERSECTION
};

bool get_cross(const QPoint & u1, const QPoint & u2, const QPoint & v1, const QPoint &v2, double &x, double &y){
	x = u1.x();
	y = u1.y();
	double a = (double)(u1.x() - u2.x())*(v1.y() - v2.y()) - (double)(u1.y() - u2.y())*(v1.x() - v2.x());
	double t = 0;
	if (a != 0)
		t = ((double)(u1.x() - v1.x())*(v1.y() - v2.y()) - (double)(u1.y() - v1.y())*(v1.x() - v2.x())) / a;
	x += (u2.x() - u1.x())*t;
	y += (u2.y() - u1.y())*t;
	return (a != 0);
}

bool online(const QLine &l, const QPoint &p)
{
	double a = (double)(l.x1() - p.x())*(l.y2() - p.y()) - (double)(l.y1() - p.y())*(l.x2() - p.x());
	return (a == 0);
}

#define SGN(x) (((x)>0) ? 1 : (((x)==0) ? 0 : -1))
int intersect(const QLine & l1, const QLine & l2, QPoint &p)
{
	Q_ASSERT(l1.x1() != l1.x2() || l1.y1() != l1.y2());
	Q_ASSERT(l2.x1() != l2.x2() || l2.y1() != l2.y2());
	double x0, y0;
	bool parallel = (!get_cross(l1.p1(), l1.p2(), l2.p1(), l2.p2(), x0, y0));
	p.setX((int)x0);
	p.setY((int)y0);
	int x11 = SGN(l1.x1() - l2.x1());
	int x12 = SGN(l1.x1() - l2.x2());
	int x21 = SGN(l1.x2() - l2.x1());
	int x22 = SGN(l1.x2() - l2.x2());
	bool same_x = (x11 == x12 && x21 == x22 && x12 == x21);
	if (same_x && x11 != 0)
		return parallel ? PARALLEL : NOINTERSECTION;

	int y11 = SGN(l1.y1() - l2.y1());
	int y12 = SGN(l1.y1() - l2.y2());
	int y21 = SGN(l1.y2() - l2.y1());
	int y22 = SGN(l1.y2() - l2.y2());
	bool same_y = (y11 == y12 && y21 == y22 && y12 == y21);
	if (same_y && y11 != 0)
		return parallel ? PARALLEL : NOINTERSECTION;

	int x01 = SGN(x0 - l2.x1());
	int x02 = SGN(x0 - l2.x2());
	int x10 = SGN(l1.x1() - x0);
	int x20 = SGN(l1.x2() - x0);
	int y01 = SGN(y0 - l2.y1());
	int y02 = SGN(y0 - l2.y2());
	int y10 = SGN(l1.y1() - y0);
	int y20 = SGN(l1.y2() - y0);

	if (l1.x1() == l1.x2()) { //x11==x21, x12==x22
		Q_ASSERT(x0 == l1.x1());
		if (x11 == x12) { //l1.x1()==l1.x2()==l2.x1()==l2.x2()
			Q_ASSERT(x11 == 0); //4 points in one line
			if (y11 != y12 && y21 != y22) //l1.y1 & l1.y2 is inside l2
				return INCLUDE;
			if (y11 == y12 && y21 != y22) //l1.y1 is outside l2, l1.y2 is inside l2
				return EXTEND1;
			if (y11 != y12 && y21 == y22) //l1.y1 is inside l2, l1.y2 is outside l2
				return EXTEND2;

			Q_ASSERT(y12 != y21);
			return INCLUDEEDBY;
		}
		else {
			if (x11 == 0) { //l1.x1() ==l2.x1() = l1.x2(), 3 points in one line
				if (y11 == y21) //l2.y1 is outside l1
					return NOINTERSECTION;
				if (y11 == 0)  //l1.y1()==l2.y1()
					return LEXTEND1;
				if (y21 == 0)  //l1.y2()==l2.y1()
					return LEXTEND2;
				return BOUNDINTERSECTION1;
			}
			if (x12 == 0) { //l1.x1()==l2.x2()=l1.x2()
				if (y12 == y22) //l2.y2 is outside l1
					return NOINTERSECTION;
				if (y12 == 0) //l1.y1()==l2.y2()
					return LEXTEND1;
				if (y22 == 0) //l1.y2()==l2.y2()
					return LEXTEND2;
				return BOUNDINTERSECTION2;
			}
			//x11!=x12 && x11!=0 && x12!=0
			if (y10 == y20)
				return NOINTERSECTION;
			if (y10 == 0)
				return TEXTEND1;
			if (y20 == 0)
				return TEXTEND2;
			return CENTERINTERSECTION;
		}
	}

	if (l1.y1() == l1.y2()) { //y11==y21, y12==y22
		Q_ASSERT(y0 == l1.y1());
		if (y11 == y12) { //l1.y1()==l1.y2()==l2.y1()==l2.y2()
			Q_ASSERT(y11 == 0); //4 points in one line
			if (x11 != x12 && x21 != x22) //l1.x1 & l1.x2 is inside l2
				return INCLUDE;
			if (x11 == x12 && x21 != x22) //l1.x1 is outside l2, l1.x2 is inside l2
				return EXTEND1;
			if (x11 != x12 && x21 == x22) //l1.x1 is inside l2, l1.x2 is outside l2
				return EXTEND2;
			//l1.x1 & l1.x2 is outside l2
			Q_ASSERT(x12 != x21);
			return INCLUDEEDBY;
		}
		else {
			if (y11 == 0) { //l1.y1() ==l2.y1() = l1.y2(), 3 points in one line
				if (x11 == x21) //l2.x1 is outside l1
					return NOINTERSECTION;
				if (x11 == 0) //l1.x1()==l2.x1()
					return LEXTEND1;
				if (x21 == 0) //l1.x2()==l2.x1()
					return LEXTEND2;
				return BOUNDINTERSECTION1;
			}
			if (y12 == 0) { //l1.y1() ==l2.y2() = l1.y2(), 3 points in one line
				if (x12 == x22) //l2.x2 is outside l1
					return NOINTERSECTION;
				if (x12 == 0) //l1.x1()==l2.x2()
					return LEXTEND1;
				if (x22 == 0) //l1.x2()==l2.x2()
					return LEXTEND2;
				return BOUNDINTERSECTION2;
			}
			//y11!=y12 && y11!=0 && y12!=0
			if (x10 == x20) //cross point is outside l1
				return NOINTERSECTION;
			if (x10 == 0)
				return TEXTEND1;
			if (x20 == 0)
				return TEXTEND2;
			return CENTERINTERSECTION;
		}
	}
	/*判断交点是否在线段内还是在线段外
	水平线,   必有 y01==y02==0
	若点在线段外, x01==x02
	若点在线段内, x01!=x02
	垂直线,   必有 x01==x02==0
	若点在线段外, y01==y02
	若点在线段内, y01!=y02
	斜线,       若点在线段外, x01==x02 && y01==y02
	若点在线段内, x01!=x02 && y01!=y02

	交点在线段外 <==> x01==x02 && y01==y02
	交点在线段内 <==> x01!=x02 || y01!=y02
	*/

	if (!parallel && (x10 == x20 && y10 == y20)) //cross point is outside l1
		return NOINTERSECTION;

	if (online(l1, l2.p1())) {
		if (parallel) {//4 points in one line
			if (x11 != x12 && x21 != x22) //l1.x1 & l1.x2 is inside l2
				return INCLUDE;
			if (x11 == x12 && x21 != x22) //l1.x1 is outside l2, l1.x2 is inside l2
				return EXTEND1;
			if (x11 != x12 && x21 == x22) //l1.x1 is inside l2, l1.x2 is outside l2
				return EXTEND2;
			//l1.x1 & l1.x2 is outside l2
			Q_ASSERT(x12 != x21);
			return INCLUDEEDBY;
		}
		else {//3 points in one line
			/*
			if (x11==x21) //l2.x1 is outside l1
			return NOINTERSECTION;*/
			Q_ASSERT(x11 != x21); //because x11==x10, x21==x20
			if (x11 == 0) //l1.x1()==l2.x1()
				return LEXTEND1;
			if (x21 == 0) //l1.x2()==l2.x1()
				return LEXTEND2;
			return BOUNDINTERSECTION1;
		}
	}

	if (online(l1, l2.p2())) { //3 points in one line
		Q_ASSERT(!parallel);
		/*if (x12==x22) //l2.x2 is outside l1
		return NOINTERSECTION;*/
		Q_ASSERT(x12 != x22); //because x12==x10, x22==x20
		if (x12 == 0) //l1.x1()==l2.x2()
			return LEXTEND1;
		if (x22 == 0) //l1.x2()==l2.x2()
			return LEXTEND2;
		return BOUNDINTERSECTION2;
	}

	if (parallel)
		return PARALLEL;
	//cross point is inside l1, and l1 is not 0 90 line
	// !parallel && x10!=x20 && y10!=y20
	if (x01 == x02 && y01 == y02)
		return NOINTERSECTION;
	if (online(l2, l1.p1()))
		return TEXTEND1;
	if (online(l2, l1.p2()))
		return TEXTEND2;
	return CENTERINTERSECTION;
}

bool cross_rect(const QRect & rect, const QLine & line)
{
	int rc;
	QPoint p;
	QLine up(rect.topLeft(), rect.bottomRight());
	rc = intersect(up, line, p);
	if (rc != PARALLEL && rc != NOINTERSECTION)
		return true;
	QLine right(rect.topRight(), rect.bottomLeft());
	rc = intersect(right, line, p);
	if (rc != PARALLEL && rc != NOINTERSECTION)
		return true;
	return false;
}

int intersect(const QRect & rect, const QLine & line)
{
	bool in0, in1;
	in0 = rect.contains(line.p1());
	in1 = rect.contains(line.p2());
	if (in0 & in1)
		return BOTHINSIDE;
	if (in0)
		return P0INSIDE;
	if (in1)
		return P1INSIDE;
	if (cross_rect(rect, line))
		return CROSSINTERSECTION;
	else
		return NOINTERSECTION;
}

#pragma pack(push)
#pragma pack(1)

typedef unsigned ID_MAIN;
// xx   xx   xx   xx
// id_main
// id_global       |local_id
// id_area      |id_part
// id_main = id_area merge id_part
// id_main = id_globar merge id_local
// id_part = type merge part_yx merge local_id
struct DBID {
    ID_MAIN id_main;   // used as database key,
    DBID() {
        id_main = 0;
    }
    DBID(unsigned x, unsigned y, unsigned char type, unsigned char local) {
        //y(28..17)->main(31..20), x(28..17)->main(19..8), type(1..0)->main(7..6),
        //y(16..15)->main(5..4), x(16..15)->main(3..2), local(1..0)->main(1..0)
        id_main = ((y & 0x1ffe0000) << 3) | ((x & 0x1ffe0000) >> 9) | ((y & 0x18000) >> 11) | ((x & 0x18000) >> 13) |
            ((type & 0x3) << 6) | (local & 0x3);
    }
    DBID(unsigned area, unsigned char type, unsigned part_x, unsigned part_y, unsigned char local) {
        id_main = (area << 8) | ((type & 3) << 6) | ((part_y & 3) << 4) | ((part_x & 3) << 2) | (local & 3);
    }
    DBID(unsigned area, unsigned char type, unsigned char part_yx_local) {
        id_main = (area << 8) | ((type & 3) << 6) | (part_yx_local & 0x3f);
    }

    unsigned get_id_area() { //area is used for display
        //main(31..8) is for area, (7..0) is for part
        return id_main >>8;
    }
    unsigned char get_id_part() {
        return id_main & 0xff;
    }
    void set_id_area(unsigned id_area) {
        id_main = (id_area << 8) | (id_main & 0xff);
    }
    void set_id_part(unsigned char part) {
        id_main = (id_main & 0xffffff00) | part;
    }
    unsigned char get_id_partyx() {
        return (id_main >> 2) & 0xf;
    }
    unsigned char get_id_local() {
        return id_main & 3;
    }
    bool operator==(const DBID & p1) const
    {
        return (id_main == p1.id_main);
    }
    /*
    static unsigned xy2id_global(unsigned x, unsigned y) {
        //y(28..17)->main(31..20), x(28..17)->main(19..8), y(16..15)->main(5..4), x(16..15)->main(3..2)
        return (((y & 0x1ffe0000) << 3) | ((x & 0x1ffe0000) >> 9) | ((y & 0x18000) >> 11) | ((x & 0x18000) >> 13)) >> 2;
    }*/
    static unsigned xy2id_area(unsigned x, unsigned y) {
        return (((y & 0x1ffe0000) << 3) | ((x & 0x1ffe0000) >> 9)) >> 8;
    }
	static unsigned gridx2areax(unsigned x)
	{
		return (x & 0x1ffe0000) >> 17;
	}
	static unsigned gridy2areay(unsigned y)
	{
		return (y & 0x1ffe0000) >> 17;
	}
    static unsigned char grid2part(unsigned x) {
        return (x & 0x18000) >> 15;
    }
    static unsigned short grid2local(unsigned x) {
        return (x & 0x7fff) ;
    }
    unsigned localx2grid(unsigned short local_x) {
        return ((id_main & 0x000fff00) << 9) | ((id_main & 0xc) << 13) | local_x;
    }
    unsigned localy2grid(unsigned short local_y) {
        return ((id_main & 0xfff00000) >>3) | ((id_main & 0x30) << 11) | local_y;
    }
	static void area2gridrect(unsigned areax, unsigned areay, unsigned & left, unsigned & top, unsigned & right, unsigned & bottom) {
		left = areax << 17;
		top = areay << 17;
		right = left + (1 << 17) - 1;
		bottom = top + (1 << 17) - 1;
	}
	static unsigned areaxy2area(unsigned areax, unsigned areay)
	{
		return (areay << 12) | areax;
	}
};
#pragma pack(pop)

//for DBID
#define TYPE_MASK				0xc0
#define TYPE_SHIFT				6
#define WIRE_TYPE				0
#define INST_TYPE				1
#define META_TYPE				2
#define AREA_WIREVIA_INFO		0
#define EXT_WIRE_NUM			2
#define EXT_WIRE_START			1
#define EXT_WIRE_END			(EXT_WIRE_START+EXT_WIRE_NUM-1)

//for flag_in_mem
#define IS_INMEM_MASK			0x80
#define IS_INMEM_SHIFT			7
#define IS_BASE_CHANGE_MASK		0x40
#define IS_BASE_CHANGE_SHIFT	6
#define IS_ATTACH_CHANGE_MASK	0x20
#define IS_ATTACH_CHANGE_SHIFT	5

//for pack_info
#define ATTACH_SIZE_MASK		0x7ff
#define ATTACH_SIZE_SHIFT		0
#define INST_ATTACH_SIZE_MASK	0xfff
#define INST_ATTACH_SIZE_SHIFT	0
//layer_num contain INST_LAYER
#define LAYER_NUM_MASK			0xf8000000
#define LAYER_NUM_SHIFT			27
//if connect to no wire, layer_min = layer_max=0, layer_min, layer_max doesn't contain INST_LAYER
#define LAYER_MIN_MASK			0x07c00000
#define LAYER_MIN_SHIFT			22
#define LAYER_MAX_MASK			0x003e0000
#define LAYER_MAX_SHIFT			17

class MemPoint {
public:
    unsigned y, x;			//extend from DBPoint (x,y), DBPoint x,y is the offset, use localx2grid to change
    unsigned char * attach; //may point to memory or database
protected:
    unsigned pack_info;		//same as DBPoint
    unsigned char part_id;
    unsigned char flag_in_mem;
    virtual unsigned get_attach_size() const = 0;
    virtual void alloc_attach_mem(unsigned size) = 0;
public:
    bool get_isinst_point() const
    {
        return (GET_FIELD(part_id, TYPE) == INST_TYPE);
    }
    bool get_isattach_inmem() const
    {
        return (GET_FIELD(flag_in_mem, IS_INMEM) == 1);
    }
    void set_isattach_inmem(bool changed) {
        SET_FIELD(flag_in_mem, IS_INMEM, changed ? 1 : 0);
    }
    bool get_isbase_change() const
    {
        return (GET_FIELD(flag_in_mem, IS_BASE_CHANGE) == 1);
    }
    void set_isbase_change(bool changed)
    {
        SET_FIELD(flag_in_mem, IS_BASE_CHANGE, changed ? 1 : 0);
    }
    bool get_isattach_change() const
    {
        return (GET_FIELD(flag_in_mem, IS_ATTACH_CHANGE) == 1);
    }
    void set_isattach_change(bool changed)
    {
        SET_FIELD(flag_in_mem, IS_ATTACH_CHANGE, changed ? 1 : 0);
    }

    DBID get_mainid() const
    {
        DBID id;
        id.set_id_area(DBID::xy2id_area(x, y));
        id.set_id_part(part_id);
        return id;
    }
    unsigned char get_partid() const
    {
        return part_id;
    }
    void set_partid(unsigned char _part_id)
    {
        part_id = _part_id;
    }

    unsigned get_pack_info() const
    {
        return pack_info;
    }

	void set_pack_info(unsigned pack_info_)
	{
		pack_info = pack_info_;
	}

    /*
        insert: source string point
        insert_num: source string number, if only delete, insert_num=0
        del_begin: delete start point
        del_num: delete number, if only insert, del_num=0
    */
    void modify_attach(unsigned char *insert, unsigned insert_num, unsigned short del_begin, unsigned del_num)
    {
        unsigned old_size = get_attach_size();
        unsigned new_size = old_size + insert_num - del_num;

        Q_ASSERT(old_size >= del_num + del_begin);
        unsigned move_size = old_size - del_begin - del_num;
        alloc_attach_mem(new_size + del_num); //alloc enough space first
        if (insert_num != 0 || del_num!=0)
            set_isattach_change(true);

        memmove(attach+ del_begin + insert_num, attach+ del_begin + del_num, move_size);
        memcpy(attach+ del_begin, insert, insert_num);
        alloc_attach_mem(new_size); //set real space
    }
    virtual ~MemPoint() {};
};

struct MemPointCmp {
	bool operator ()(const MemPoint *lhs, const MemPoint *rhs) const
	{
		if (lhs->y != rhs->y)
			return lhs->y > rhs->y;
		if (lhs->x != rhs->x)
			return lhs->x > rhs->x;
		return lhs->get_pack_info() > rhs->get_pack_info();
	}
};

//for layer_connectnum
#define LAYER_MASK			0xf8
#define LAYER_SHIFT			3
#define CONNECT_NUM_MASK	0x7
#define CONNECT_NUM_SHIFT	0

#define ANY_ANGLE_MASK		0x20
#define ANY_ANGLE_SHIFT		5
#define DY_SIGN_MASK		0x10
#define DY_SIGN_SHIFT		4
#define DY_SCALE_MASK		0x8
#define DY_SCALE_SHIFT		3
#define DX_SIGN_MASK		0x4
#define DX_SIGN_SHIFT		2
#define DX_SCALE_MASK		0x2
#define DX_SCALE_SHIFT		1
#define DIR_MASK			0x1c
#define DIR_SHIFT			2
#define DSCALE_MASK			0x3
#define DSCALE_SHIFT		0

//INST LAYER must be 0
#define INST_LAYER 0

typedef union {
    struct {
        unsigned x, y;
        unsigned char dir, size;
    } wp;
    struct {
        unsigned x, y;
        unsigned short port;
        unsigned char size;
    } ip;
} PairPoint;

/*
 * size(1) layer_connectnum(1) dir_scale(1) len(1 to 4) | dir_scale(1), len(1 to 4)...
 * size(1) layer_connectnum(1) dir_scale(1) dy(2 or 4) dx(2 or 4) | dir_scale(1), dy(2 or 4) dx(2 or 4)..
 * size(1) layer_connetnum(1) dy(2) dx(2) PortID (1 or 2) | dy(2) dx(2) PortID (1 or 2)...
 * size(1) name_string
 *
 */
class MemVWPoint : public MemPoint {
public:
    void alloc_attach_mem(unsigned size)
    {
        unsigned short cap;
        unsigned old_size = get_attach_size();
        unsigned char * new_attach;

        Q_ASSERT(size < 0x2000);
        set_attach_size(size);
        if (!get_isattach_inmem() || attach == NULL) {
            set_isattach_inmem(true);
            cap = 32;
            while (cap < size + 2 || cap < old_size + 2)
                cap <<= 1;
            new_attach = (unsigned char *)malloc(cap);
            Q_ASSERT(new_attach != NULL);
            *((unsigned short*)new_attach) = cap;
            new_attach += 2;
            if (attach != NULL)
                memcpy(new_attach, attach, old_size);
            attach = new_attach;
        }

        cap = *((unsigned short*)(attach - 2));
        if (cap < size + 2) {
            Q_ASSERT(old_size < size);
            while (cap < size + 2)
                cap <<= 1;
            new_attach = (unsigned char *)malloc(cap);
            *((unsigned short*)new_attach) = cap;
            new_attach += 2;
            memcpy(new_attach, attach, old_size);
            free(attach - 2);
            attach = new_attach;
        }
    }
public:
    MemVWPoint()
    {
        attach = NULL;
        pack_info = 0;
        flag_in_mem = 0;
		part_id = 0;
		SET_FIELD(part_id, TYPE, WIRE_TYPE);
    }

    //use this to create MemVWPoint point to database
    MemVWPoint(unsigned x_, unsigned y_, unsigned pack_info_, unsigned char part_id_, unsigned char * attach_)
    {
        y = y_;
        x = x_;
        pack_info = pack_info_;
        part_id = part_id_;
        attach = attach_;
        flag_in_mem = 0;
    }

	//use this to create MemVWPoint in one layer
	MemVWPoint(unsigned x_, unsigned y_)
	{
		y = y_;
		x = x_;
		pack_info = 0;
		part_id = 0;
		flag_in_mem = 0;
		attach = NULL;
	}
	
	MemVWPoint(unsigned x_, unsigned y_, unsigned char layer)
	{
		y = y_;
		x = x_;
		pack_info = 0;		
		set_layer_num(1);
		if (layer != INST_LAYER) {
			set_layer_min(layer);
			set_layer_max(layer);
		}		
		part_id = 0;
		SET_FIELD(part_id, TYPE, WIRE_TYPE);
		flag_in_mem = 0;
		attach = NULL;
	}

    MemVWPoint(const MemVWPoint &vwp)
    {
        y = vwp.y;
        x = vwp.x;
        pack_info = vwp.pack_info;
        part_id = vwp.part_id;
        flag_in_mem = 0;
		attach = NULL;
        //if attach point to database, shallow copy else deep copy
        if (vwp.get_isattach_inmem()) {
            int attach_size = vwp.get_attach_size();
            alloc_attach_mem(attach_size);
            memcpy(attach, vwp.attach, attach_size);
        }
        else
            attach = vwp.attach;
    }

    ~MemVWPoint()
    {
        if (get_isattach_inmem())
            free(attach - 2);
    }

    MemVWPoint & operator=(const MemVWPoint & vwp)
    {
        if (get_isattach_inmem())
            free(attach - 2);
        y = vwp.y;
        x = vwp.x;
        pack_info = vwp.pack_info;
        part_id = vwp.part_id;
        flag_in_mem = 0;

        //if attach point to database, shallow copy else deep copy
        if (vwp.get_isattach_inmem()) {
            int attach_size = vwp.get_attach_size();
            alloc_attach_mem(attach_size);
            memcpy(attach, vwp.attach, attach_size);
        }
        else
            attach = vwp.attach;
    }

	bool operator==(const MemVWPoint &vwp) const
	{
		if (x != vwp.x || y != vwp.y || pack_info != vwp.pack_info)
			return false;

		return (memcmp(attach, vwp.attach, get_attach_size()) == 0);			 
	}

	bool operator!=(const MemVWPoint &vwp) const
	{
		if (x != vwp.x || y != vwp.y || pack_info != vwp.pack_info)
			return true;

		return (memcmp(attach, vwp.attach, get_attach_size()) != 0);
	}

    unsigned get_attach_size() const
    {
        return GET_FIELD(pack_info, ATTACH_SIZE);
    }

    void set_attach_size(unsigned size)
    {
        Q_ASSERT(size <= 1020);
        SET_FIELD(pack_info, ATTACH_SIZE, size);
    }

    unsigned short get_attach_cap()
    {
        return *((unsigned short*)(attach - 2));
    }

    bool get_isto_inst() const
    {
		if (attach != NULL)
			return (GET_FIELD(attach[1], LAYER) == INST_LAYER && get_layer_num() > 0);
        else
            return false;
    }

    int get_layer_num() const
    {
        return GET_FIELD(pack_info, LAYER_NUM);
    }

    int set_layer_num(unsigned char layer_num)
    {
        Q_ASSERT(layer_num <= 31);
        SET_FIELD(pack_info, LAYER_NUM, layer_num);
        return 0;
    }

    unsigned char get_layer_min() const
    {
        return GET_FIELD(pack_info, LAYER_MIN);
    }

    int set_layer_min(unsigned char layer)
    {
        Q_ASSERT(layer <= 31);
        SET_FIELD(pack_info, LAYER_MIN, layer);
        return 0;
    }

    unsigned char get_layer_max() const
    {
        return GET_FIELD(pack_info, LAYER_MAX);
    }

    int set_layer_max(unsigned char layer)
    {
        Q_ASSERT(layer <= 31);
        SET_FIELD(pack_info, LAYER_MAX, layer);
        return 0;
    }

    bool get_isvia() const
    {
        bool isvia = true;
        unsigned layer_num = get_layer_num();
        if (layer_num <= 1 || (layer_num == 2 && get_isto_inst()))
            isvia = false;
        return isvia;
    }
    //return all connected layers, maybe zero
    void get_layers(vector<unsigned char> & layers) const
    {
        layers.resize(get_layer_num());
		if (get_layer_num() == 0)
			return;
        unsigned char * ref = attach;
        unsigned limit = get_attach_size();
        for (int i = 0; i < layers.size(); i++) {
            layers[i] = GET_FIELD(ref[1], LAYER);
            ref += ref[0];
            Q_ASSERT(ref <= attach + limit);
        }
		if (layers.size()>1 || layers[0] != INST_LAYER) {
			Q_ASSERT(layers.back() == get_layer_max());
			if (layers[0]==INST_LAYER)
				Q_ASSERT(layers[1] == get_layer_min());
			else
				Q_ASSERT(layers[0] == get_layer_min());
		}
			
    }

    void get_layer_wire(unsigned char layer, vector <PairPoint> & rp) const
    {
        rp.clear();
        unsigned short ref = 0;
        unsigned limit = get_attach_size();
        int layer_num = get_layer_num();

        for (int i = 0; i < layer_num; i++) {
            if (GET_FIELD(attach[ref+1], LAYER) == layer) {
				rp.resize(GET_FIELD(attach[ref + 1], CONNECT_NUM) + 1);
                ref += 2;
                for (int j = 0; j < rp.size(); j++) {
                    if (layer == INST_LAYER) { //ref point to dy(2) dx(2) PortID (1 or 2)
                        short dy, dx;
                        int newy, newx;
						dy = *((short*)(attach + ref));
						dx = *((short*)(attach + ref + 2));
                        newx = (int)x + dx;
                        newy = (int)y + dy;
                        Q_ASSERT(newx >= 0 && newy >= 0); //more check
                        rp[j].ip.x = newx;
                        rp[j].ip.y = newy;
                        ref += 4; //ref point to PortID (1 or 2)
                        rp[j].ip.size = 4;
						if (attach[ref] >= 128) {
							rp[j].ip.port = ((unsigned short)(attach[ref] & 0x7f) << 8) | attach[ref + 1];
                            ref += 2;
                            rp[j].ip.size += 2;
                        }
                        else {
                            rp[j].ip.port = attach[ref];
                            rp[j].ip.size++;
                            ref++;
                        }
                    }
                    else { //ref point to dir_scale(1), dy(2 or 4) dx(2 or 4)
						rp[j].wp.dir = attach[ref];
                        if (rp[j].wp.dir & ANY_ANGLE_MASK) { //ref[1] point to dy(2 or 4) dx(2 or 4)
                            unsigned dx, dy;
                            if (rp[j].wp.dir & DY_SCALE_MASK) {
								dy = *(unsigned*)(attach + ref + 1);
                                ref += 5;
                                rp[j].wp.size = 5;
                            }
                            else {
								dy = *(unsigned short*)(attach + ref + 1);
                                ref += 3;
                                rp[j].wp.size = 3;
                            }
                            //ref point to dx(2 or 4)
                            if (rp[j].wp.dir & DX_SCALE_MASK) {
								dx = *(unsigned*)(attach + ref);
                                ref += 4;
                                rp[j].wp.size += 4;
                            }
                            else {
								dx = *(unsigned short *)(attach + ref);
                                ref += 2;
                                rp[j].wp.size += 2;
                            }
                            if (rp[j].wp.dir & DX_SIGN_MASK) {
                                Q_ASSERT(x >= dx);
                                rp[j].wp.x = x - dx;
                            }
                            else {
                                //TODO: add some check
                                rp[j].wp.x = x + dx;
                            }
                            if (rp[j].wp.dir & DY_SIGN_MASK) {
                                Q_ASSERT(y >= dy);
                                rp[j].wp.y = y - dy;
                            }
                            else {
                                //TODO: add some check
                                rp[j].wp.y = y + dy;
                            }
                        }
                        else { //ref[1] point to len(1 to 4)
                            unsigned d;
                            switch (GET_FIELD(rp[j].wp.dir, DSCALE)) {
                            case 0:
								d = attach[ref + 1];
                                ref += 2;
                                rp[j].wp.size = 2;
                                break;
                            case 1:
								d = *(unsigned short*)(attach + ref + 1);
                                ref += 3;
                                rp[j].wp.size = 3;
                                break;
                            case 2:
								d = attach[ref + 3];
								d = (d << 8) | attach[ref + 2];
								d = (d << 8) | attach[ref + 1];
                                ref += 4;
                                rp[j].wp.size = 4;
                                break;
                            case 3:
								d = *(unsigned*)(attach + ref + 1);
                                ref += 5;
                                rp[j].wp.size = 5;
                                break;
                            }
                            rp[j].wp.x = x;
                            rp[j].wp.y = y;
                            switch (GET_FIELD(rp[j].wp.dir, DIR)) {
                            case 0:
                                Q_ASSERT(y >= d);
                                rp[j].wp.y = y - d;
                                break;
                            case 1:
                                Q_ASSERT(y >= d);
                                rp[j].wp.x = x + d;
                                rp[j].wp.y = y - d;
                                break;
                            case 2:
                                rp[j].wp.x = x + d;
                                break;
                            case 3:
                                rp[j].wp.x = x + d;
                                rp[j].wp.y = y + d;
                                break;
                            case 4:
                                rp[j].wp.y = y + d;
                                break;
                            case 5:
                                Q_ASSERT(x >= d);
                                rp[j].wp.x = x - d;
                                rp[j].wp.y = y + d;
                                break;
                            case 6:
                                Q_ASSERT(x >= d);
                                rp[j].wp.x = x - d;
                                break;
                            case 7:
                                Q_ASSERT(x >= d && y >= d);
                                rp[j].wp.x = x - d;
                                rp[j].wp.y = y - d;
                                break;
                            }
                        }
                    }
                }
                break;
            }
            ref += attach[ref];
            Q_ASSERT(ref <= limit);
        }
    }

	int intersect(const QLine & wire, unsigned char layer, bool exclude_p0, bool exclude_p1)
	{
		int rc;
		if (layer<get_layer_min() || layer>get_layer_max())
			return false;
		vector <PairPoint> rp;
		get_layer_wire(layer, rp);

		if (exclude_p0 && x == wire.x1() && y == wire.y1())
			return NOINTERSECTION;
		if (exclude_p1 && x == wire.x2() && y == wire.y2())
			return NOINTERSECTION;
		QPoint org(x, y);
		QPoint pp;
		for (int i = 0; i < rp.size(); i++) {
			if (exclude_p0 && rp[i].wp.x == wire.x1() && rp[i].wp.y == wire.y1())
				continue;
			if (exclude_p1 && rp[i].wp.x == wire.x2() && rp[i].wp.y == wire.y2())
				continue;
			rc = ::intersect(QLine(org, QPoint(rp[i].wp.x, rp[i].wp.y)), wire, pp);
			if (rc != NOINTERSECTION && rc != PARALLEL)
				return rc;
		}
		return NOINTERSECTION;
	}

    //if success return 0, else return -1
    int delete_layer_wire(unsigned char layer, unsigned x, unsigned y, unsigned short port=0)
    {
        unsigned layer_num = get_layer_num();
        unsigned short ref = 0;
        vector <PairPoint> rp;
        int del_idx = -1;
        get_layer_wire(layer, rp);
        for (int i = 0; i < rp.size(); i++) {
            if (layer == INST_LAYER) {
                if (rp[i].ip.x == x && rp[i].ip.y ==y && rp[i].ip.port == port)
                    del_idx = i;
            }
            else {
                if (rp[i].wp.x == x && rp[i].wp.y == y)
                    del_idx = i;
            }
        }
        if (del_idx == -1)
            return -1;
        unsigned del_size = (layer == INST_LAYER) ? rp[del_idx].ip.size : rp[del_idx].wp.size;
		unsigned char last_layer = 0;
        for (unsigned i = 0; i < layer_num; i++) {
            if (GET_FIELD(attach[ref+1], LAYER) == layer) { //ref point to size(1) layer_connectnum(1)
                if (rp.size() == 1) {
                    Q_ASSERT(attach[ref] == del_size + 2);
                    modify_attach(NULL, 0, ref, attach[ref]); //delete whole layer connection
                    set_layer_num(layer_num - 1);
                    if (layer == get_layer_max())
                        set_layer_max(last_layer);
                    if (layer == get_layer_min()) {
						Q_ASSERT(i == 0 || i == 1);
                        if (layer_num == i + 1)
                            set_layer_min(0); //no wire layer, only inst layer left
                        else                             
							set_layer_min(GET_FIELD(attach[ref + 1], LAYER));                        
                    }
                }
                else {
                    unsigned size = 0;
                    for (int j = 0; j < del_idx; j++)
                        size += (layer == INST_LAYER) ? rp[j].ip.size : rp[j].wp.size;
                    modify_attach(NULL, 0, ref + 2 + size, del_size); //delete one layer wire
					Q_ASSERT(attach[ref] > del_size && (GET_FIELD(attach[ref + 1], CONNECT_NUM) + 1 == rp.size()));
                    attach[ref] -= del_size;
					SET_FIELD(attach[ref + 1], CONNECT_NUM, rp.size() - 2);
                }
				return 0;
            }
			last_layer = GET_FIELD(attach[ref + 1], LAYER);
            ref += attach[ref];
        }
		return -1;
    }

	//add connection to other point
	//if success return 0, else return -1
    int add_noninst_wire(unsigned char layer, unsigned wx, unsigned wy)
    {
        unsigned layer_num = get_layer_num();
		unsigned short ref = 0;
        unsigned i, dx, dy;

        Q_ASSERT(layer <= 31 && layer != INST_LAYER);
        for (i = 0; i < layer_num; i++) {
			if (GET_FIELD(attach[ref + 1], LAYER) >= layer)
				break;
			ref += attach[ref];
        }
        unsigned char insert[30]; //insert point to size(1) layer_connectnum(1)  dir_scale(1) local_id(1)
        unsigned insert_len = 0;
        insert[2] = 0;
        Q_ASSERT(x != wx || y != wy);
        dx = (x >= wx) ? x - wx : wx - x;
        dy = (y >= wy) ? y - wy : wy - y;
        if (dx == 0) {
            if (y > wy)
                SET_FIELD(insert[2], DIR, 0);
            else
                SET_FIELD(insert[2], DIR, 4);
        }
        if (dy == 0) {
            if (x > wx)
                SET_FIELD(insert[2], DIR, 6);
            else
                SET_FIELD(insert[2], DIR, 2);
        }
        if (dx == dy) {
            if (x > wx)
                if (y > wy)
                    SET_FIELD(insert[2], DIR, 7);
                else
                    SET_FIELD(insert[2], DIR, 5);
            else
                if (y > wy)
                    SET_FIELD(insert[2], DIR, 1);
                else
                    SET_FIELD(insert[2], DIR, 3);
        }
        if (dx == 0 || dy == 0 || dx == dy) {
            SET_FIELD(insert[2], ANY_ANGLE, 0);
            unsigned d = (dx > dy) ? dx : dy;
            if (d < 0x100) {
                SET_FIELD(insert[2], DSCALE, 0);
                insert[3] = d;
                insert_len = 4;
            }
            else
                if (d < 0x10000) {
                    SET_FIELD(insert[2], DSCALE, 1);
                    *(unsigned short*)(&insert[3]) = d;
                    insert_len = 5;
                }
                else
                    if (d < 0x1000000) {
                        SET_FIELD(insert[2], DSCALE, 2);
                        insert[3] = d & 0xff;
                        insert[4] = (d >> 8) & 0xff;
                        insert[5] = (d >> 16);
                        insert_len = 6;
                    }
                    else {
                        SET_FIELD(insert[2], DSCALE, 3);
                        *(unsigned*)(&insert[3]) = d;
                        insert_len = 7;
                    }
        }
        else {
            SET_FIELD(insert[2], ANY_ANGLE, 1);
            if (x>wx)
                SET_FIELD(insert[2], DX_SIGN, 1);
            else
                SET_FIELD(insert[2], DX_SIGN, 0);
            if (y>wy)
                SET_FIELD(insert[2], DY_SIGN, 1);
            else
                SET_FIELD(insert[2], DY_SIGN, 0);
            if (dy < 0x10000) {
                SET_FIELD(insert[2], DY_SCALE, 0);
                *(unsigned short*)(&insert[3]) = dy;
                insert_len = 5;
            }
            else {
                SET_FIELD(insert[2], DY_SCALE, 1);
                *(unsigned*)(&insert[3]) = dy;
                insert_len = 7;
            }
            if (dx < 0x10000) {
                SET_FIELD(insert[2], DX_SCALE, 0);
                *(unsigned short*)(&insert[insert_len]) = dx;
                insert_len += 2;
            }
            else {
                SET_FIELD(insert[2], DX_SCALE, 1);
                *(unsigned*)(&insert[insert_len]) = dx;
                insert_len += 4;
            }
        }

        if (layer_num==i || GET_FIELD(attach[ref+1], LAYER) > layer) {	//add new layer
            //insert size(1) layer_connectnum(1) dir_scale(1) len(1 to 4) or
            //insert size(1) layer_connectnum(1) dir_scale(1) dy(2 or 4) dx(2 or 4)
            insert[0] = insert_len;
            insert[1] = 0;
            SET_FIELD(insert[1], LAYER, layer);
            SET_FIELD(insert[1], CONNECT_NUM, 0);
            set_layer_num(layer_num + 1);
            modify_attach(insert, insert_len, ref, 0); //insert new layer
            if (layer > get_layer_max())
                set_layer_max(layer);
            if (layer < get_layer_min() || get_layer_min() == 0)
                set_layer_min(layer);
        }
        else {
            //insert dir_scale(1) len(1 to 4) or
            //insert dir_scale(1) dy(2 or 4) dx(2 or 4)
			unsigned char cnum = GET_FIELD(attach[ref + 1], CONNECT_NUM);
            if (cnum==7)
                return -1;
            modify_attach(insert + 2, insert_len - 2, ref+2, 0); //insert existing layer new connection
            attach[ref] += insert_len - 2;
			SET_FIELD(attach[ref + 1], CONNECT_NUM, cnum + 1);
        }
		return 0;
    }

    int add_inst_wire(unsigned ix, unsigned iy, unsigned short port)
    {
        unsigned char insert[30]; //insert point to size(1) layer_connetnum(1) dy(2) dx(2) PortID (1 or 2)
        unsigned insert_len = 0;

        if (port >= 0x8000)
            return -1;
        short dx, dy;
        dx = (int)ix - (int)x;
        dy = (int)iy - (int)y;
        *(short *) (&insert[2]) = dy;
        *(short *) (&insert[4]) = dx;
        insert_len = 6;

        if (port < 128) {
            insert[insert_len] = port;
            insert_len++;
        }
        else {
			insert[insert_len] = (port >> 8) | 0x80;
            insert[insert_len + 1] = port & 0xff;
            insert_len += 2;
        }

        if (!get_isto_inst()) {
            insert[0] = insert_len;
            insert[1] = 0;
            SET_FIELD(insert[1], LAYER, INST_LAYER);
            SET_FIELD(insert[1], CONNECT_NUM, 0);
            set_layer_num(get_layer_num() + 1);
            modify_attach(insert, insert_len, 0, 0);
        }
        else {
            //insert dy(2) dx(2) PortID (1 or 2)
            unsigned char cnum = GET_FIELD(attach[1], CONNECT_NUM);
            if (cnum == 7)
                return -1;
            modify_attach(insert + 2, insert_len - 2, 2, 0);
            attach[0] += insert_len - 2;
            SET_FIELD(attach[1], CONNECT_NUM, cnum + 1);
        }
		return 0;
    }
};

struct MemVWPointCmp {
	bool operator ()(const MemVWPoint *lhs, const MemVWPoint *rhs) const
	{		
		if (lhs->y != rhs->y)
			return lhs->y > rhs->y;
		if (lhs->x != rhs->x)
			return lhs->x > rhs->x;
		return  lhs->get_layer_min() > rhs->get_layer_max();
	}
};

#define INST_CONNECT_NUM_MASK	0x7
#define INST_CONNECT_NUM_SHIFT	0

/*
* connectnum(1) dy(2) dx(2)  | dy(2) dx(2) ...
* size(1) name_string
*/
class MemInstPoint : MemPoint {
public:
    MemInstPoint()
    {
        attach = NULL;
        pack_info = 0;
        flag_in_mem = 0;
    }

    ~MemInstPoint()
    {
        if (get_isattach_inmem())
            free(attach - 2);
    }

    unsigned get_attach_size() const
    {
        return GET_FIELD(pack_info, INST_ATTACH_SIZE);
    }

    void set_attach_size(unsigned size)
    {
        Q_ASSERT(size <= 2000);
        SET_FIELD(pack_info, INST_ATTACH_SIZE, size);
    }

    unsigned short get_attach_cap()
    {
        return *((unsigned short*)(attach - 2));
    }

    void alloc_attach_mem(unsigned size)
    {
        unsigned short cap;
        unsigned old_size = get_attach_size();
        unsigned char * new_attach;

        set_attach_size(size);
        if (!get_isattach_inmem() || attach == NULL) {
            set_isattach_inmem(true);
            cap = 64;
            while (cap < size + 2)
                cap <<= 1;
            new_attach = (unsigned char *)malloc(cap);
            *((unsigned short*)new_attach) = cap;
            new_attach += 2;
            if (attach != NULL)
                memcpy(new_attach, attach, old_size);
            attach = new_attach;
        }

        cap = *((unsigned short*)(attach - 2));
        if (cap < size + 2) {
            while (cap < size + 2)
                cap <<= 1;
            new_attach = (unsigned char *)malloc(cap);
            *((unsigned short*)new_attach) = cap;
            new_attach += 2;
            memcpy(new_attach, attach, old_size);
            free(attach - 2);
            attach = new_attach;
        }
    }

    void get_port_wire(unsigned char port, vector <PairPoint> & rp) const
    {

    }

    int delete_wire(unsigned x, unsigned y, unsigned short port = 0)
    {

    }
};

#define ADDLINE_MODIFY_P0 1
#define ADDLINE_CREATE_P0 2
#define ADDLINE_MODIFY_P1 4
#define ADDLINE_CREATE_P1 8
#define DELLINE_MODIFY_P0 0x10
#define DELLINE_DELETE_P0 0x20
#define DELLINE_MODIFY_P1 0x40
#define DELLINE_DELETE_P1 0x80

class PointPatch {
public:
	vector<MemPoint *> old_points;
	vector<MemPoint *> new_points;
	int action;
public:
	PointPatch() {
		action = 0;
	}
	~PointPatch() {
		for (int i = 0; i < old_points.size(); i++)
			delete old_points[i];
		for (int i = 0; i < new_points.size(); i++)
			delete new_points[i];
	}	
};
#endif // ELEMENT_H

