#ifndef ELEMENT_H
#define ELEMENT_H

#include <vector>
#include <string.h>
using namespace std;

#define GET_FIELD(var, field) (((var) & field##_MASK) >> field##_SHIFT)
#define SET_FIELD(var, field, num) var = ((unsigned)var & ~(field##_MASK)) | (((unsigned)(num) << field##_SHIFT) & field##_MASK)
#define MAKE_U32(y, x) (((unsigned) (y) <<16) | (x))


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
};
#pragma pack(pop)

//for DBID
#define TYPE_MASK				0xc0
#define TYPE_SHIFT				6
#define WIRE_TYPE				0
#define INST_TYPE				1
#define META_TYPE				2
#define AREA_WIREVIA_INFO		0

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
#define LAYER_NUM_MASK			0xf8000000
#define LAYER_NUM_SHIFT			27
//if connect to no wire, layer_min = layer_max=0
#define LAYER_MIN_MASK			0x07c00000
#define LAYER_MIN_SHIFT			22
#define LAYER_MAX_MASK			0x003e0000
#define LAYER_MAX_SHIFT			17

class MemPoint {
public:
    unsigned y, x;			//same as DBPoint
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
 * PATCH_CHANGE_NAME name_len(1) name_string
 * PATCH_DELETE_REF ref_index(1)
 * PATCH_INSERT_REF ref_index(1) layer_connectnum(1) dir_scale(1) len(1 to 4) | dir_scale(1), local_id(1)...
 *              or  ref_index(1) layer_connectnum(1) dir_scale(1) dy(2 or 4) dx(2 or 4|, dir_scale(1), local_id(1)...
 *              or  ref_index(1) layer_connectnum(1) dy(2) dx(2) PortID (1 or 2) | dy(2) dx(2) PortID (1 or 2)...
 * PATCH_CHANGE_REF ref_index(1) layer_connectnum(1) dir_scale(1) local_id(1) len(2 or 3)
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

    MemVWPoint(const MemVWPoint &vwp)
    {
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
                            set_layer_min(0);
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


class PointPatch {
	MemPoint * old_point;
	MemPoint * new_point;	
};
#endif // ELEMENT_H

