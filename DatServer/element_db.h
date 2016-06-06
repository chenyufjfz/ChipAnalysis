#ifndef ELEMENT_DB_H
#define ELEMENT_DB_H

#include <QRect>
#include "element.h"
#include "lmdb.h"
#include <map>
#include <stdlib.h>

#pragma pack(push)
#pragma pack(1)
typedef struct {    
    unsigned short y, x;
    unsigned pack_info; //layer_num(5), layer_min(5), layer_max(5) score(4), template(13), dir(3), flag(4), attach_ref(11)
} DBPoint;

#define EXTWIRE_LAYER_MASK 0x1f
#define EXTWIRE_LAYER_SHIFT 0

#define ERR_CHECK(expr) CHECK0((rc = (expr)) == MDB_SUCCESS, #expr)
#define ERR_CHECK1(expr, ret) CHECK1((rc = (expr)) == MDB_SUCCESS, #expr, ret)
#define CHECK0(test, msg) do {if (!(test)) { qInfo(msg); qCritical(mdb_strerror(rc)); return;}} while(0)
#define CHECK1(test, msg, ret) do {if (!(test)) { qInfo(msg); qCritical(mdb_strerror(rc)); return ret;}} while(0)

struct DBExtWire {
	unsigned y0, x0, y1, x1;
	unsigned short pack_info;
	DBExtWire(unsigned x0_, unsigned y0_, unsigned x1_, unsigned y1_, unsigned char layer_) {
		x0 = x0_;
		x1 = x1_;
		y0 = y0_;
		y1 = y1_;
		pack_info = 0;
		SET_FIELD(pack_info, EXTWIRE_LAYER, layer_);
	}
	bool operator==(const DBExtWire &dbw) const {
		if (x0 != dbw.x0 || y0 != dbw.y0 || x1 != dbw.x1 || y1 != dbw.y1 || pack_info != dbw.pack_info)
			return false;
		return true;
	}
};

//can be unsigned char?
typedef unsigned short DB_NUM_TYPE;

typedef struct {						
	DB_NUM_TYPE part_point_num[16][4];
	DB_NUM_TYPE ext_wire_num[EXT_WIRE_NUM];
} DBAreaWireVia;

#pragma pack(pop)

struct DBPointCmp {
	bool operator ()(const DBPoint *lhs, const DBPoint *rhs) const {
		if (lhs->y != rhs->y)
			return lhs->y > rhs->y;
		if (lhs->x != rhs->x)
			return lhs->x > rhs->x;
		return lhs->pack_info > rhs->pack_info;
	}
};

struct DBExtWireCmp {
	bool operator ()(const DBExtWire *lhs, const DBExtWire *rhs) const {
		if (lhs->y0 != rhs->y0)
			return lhs->y0 > rhs->y0;
		if (lhs->x0 != rhs->x0)
			return lhs->x0 > rhs->x0;
		if (lhs->y1 != rhs->y1)
			return lhs->y1 > rhs->y1;
		if (lhs->x1 != rhs->x1)
			return lhs->x1 > rhs->x1;
		return lhs->pack_info > rhs->pack_info;
	}
};

#define HASH_SHIFT	9
#define SPLIT_TH	24
#define HASH_SPLIT(x,y) ((((y) >> 14) + ((x) >> 14) + ((x) >> HASH_SHIFT)) & 1)
//Area contain 16(4*4) parts, each part have at most 4 entries, entry may be empty

class MemAreaWireVia {
protected:
	DBAreaWireVia db_info;						//saved in database
	unsigned short part_size[16][4];			//saved as data.size
	unsigned short ext_wire_size[EXT_WIRE_NUM];
	DBPoint * part_points[16][4];					//maybe point to memory or database	
	DBExtWire * ext_wires[EXT_WIRE_NUM];
	unsigned long long point_set_change;
	unsigned ext_wire_change;
	MDB_txn *txn;
	MDB_dbi dbi;
	unsigned area;

	typedef pair<int, int> PII;

protected:
	int _get_point(DBID id, unsigned x, unsigned y, vector<MemVWPoint> & points, vector<PII> &idx, 
		bool clone_point=false, unsigned char layer=0xff)
	{
		points.clear();
		idx.clear();
		unsigned char part_yx = id.get_id_partyx();
		unsigned char local = id.get_id_local();
		unsigned short local_y = DBID::grid2local(y), local_x = DBID::grid2local(x);
		unsigned local_yx = MAKE_U32(local_y, local_x);		

		if (db_info.part_point_num[part_yx][local] == 0)
			return 0;
		DBPoint * search_set = part_points[part_yx][local]; //search_set point to x,y,pack_info
		if (search_set == NULL) {			
			MDB_val key, data;
			int rc;
			key.mv_size = sizeof(id);
			key.mv_data = &id;
			ERR_CHECK1(mdb_get(txn, dbi, &key, &data), -1);
			search_set = (DBPoint *)data.mv_data;
			part_points[part_yx][local] = search_set;
			Q_ASSERT(data.mv_size <= 64000);
			part_size[part_yx][local] = (unsigned short) data.mv_size;
		}
		Q_ASSERT(db_info.part_point_num[part_yx][local] * sizeof(DBPoint) < part_size[part_yx][local]);
		unsigned char * attach = (unsigned char *)&search_set[db_info.part_point_num[part_yx][local]];
		unsigned offset = 0;
		for (unsigned i = 0; i < db_info.part_point_num[part_yx][local]; i++) {
			unsigned yx = MAKE_U32(search_set[i].y, search_set[i].x);			
			if (yx == local_yx) {
				MemVWPoint point(x, y, search_set[i].pack_info, id.get_id_part(), 
					attach + offset);
				if (layer == 0xff || (layer == INST_LAYER && point.get_isto_inst()) 
					|| (layer <= point.get_layer_max() && layer >= point.get_layer_min())) {
					points.push_back(point);
					if (clone_point)
						points[points.size() - 1].alloc_attach_mem(point.get_attach_size());
					PII idx_new = make_pair(i, offset);
					idx.push_back(idx_new);
				}				
			}
			if (yx > local_yx)
				break;
			offset += GET_FIELD(search_set[i].pack_info, ATTACH_SIZE);
		}
		return 0;
	}
	
	//part_points should be mdb_get from DB before call this
	void alloc_part_mem(unsigned char part_id, unsigned size)
	{
		unsigned short cap;
		unsigned char part_yx = part_id >> 2;
		unsigned char local = part_id & 3;
		unsigned old_size = part_size[part_yx][local];
		unsigned char * new_part;

		Q_ASSERT(size <= 64000);
		part_size[part_yx][local] = size;
		if (((point_set_change >> part_id) & 1)==0 || part_points[part_yx][local] == NULL) { //in database or empty
			point_set_change |= (unsigned long long) 1 << part_id;
			cap = 63;
			while (cap < size + 2 || cap < old_size + 2)
				cap <<= 1;
			new_part = (unsigned char *)malloc(cap);
			Q_ASSERT(new_part != NULL);
			*((unsigned short*)new_part) = cap;
			new_part += 2;
			if (part_points[part_yx][local] != NULL)
				memcpy(new_part, part_points[part_yx][local], old_size);
			part_points[part_yx][local] = (DBPoint*)new_part;
		}

		cap = *((unsigned short*)(part_points[part_yx][local]) - 1);
		if (cap < size + 2) {
			Q_ASSERT(old_size < size);
			while (cap < size + 2)
				cap <<= 1;
			new_part = (unsigned char *)malloc(cap);
			Q_ASSERT(new_part != NULL);
			*((unsigned short*)new_part) = cap;
			new_part += 2;
			memcpy(new_part, part_points[part_yx][local], old_size);
			free((unsigned short*)(part_points[part_yx][local]) - 1);
			part_points[part_yx][local] = (DBPoint*) new_part;
		}
	}

	void alloc_ext_mem(unsigned char ext_wires_id, unsigned size)
	{
		unsigned old_size = ext_wire_size[ext_wires_id];
		unsigned char * new_part;
		unsigned short cap;

		Q_ASSERT(size <= 64000 && ext_wires_id < EXT_WIRE_NUM);
		ext_wire_size[ext_wires_id] = size;
		if (((ext_wire_change >> ext_wires_id) & 1) == 0 || ext_wires[ext_wires_id] == NULL)  { //in database or empty
			ext_wire_change |= 1 << ext_wires_id;
			cap = 63;
			while (cap < size + 2 || cap < old_size + 2)
				cap <<= 1;
			new_part = (unsigned char *)malloc(cap);
			Q_ASSERT(new_part != NULL);
			*((unsigned short*)new_part) = cap;
			new_part += 2;
			if (ext_wires[ext_wires_id] != NULL)
				memcpy(new_part, ext_wires[ext_wires_id], old_size);
			ext_wires[ext_wires_id] = (DBExtWire*)new_part;
		}
		cap = *((unsigned short*)(ext_wires[ext_wires_id]) - 1);
		if (cap < size + 2) {
			Q_ASSERT(old_size < size);
			while (cap < size + 2)
				cap <<= 1;
			new_part = (unsigned char *)malloc(cap);
			Q_ASSERT(new_part != NULL);
			*((unsigned short*)new_part) = cap;
			new_part += 2;
			memcpy(new_part, ext_wires[ext_wires_id], old_size);
			free((unsigned short*)(ext_wires[ext_wires_id]) - 1);
			ext_wires[ext_wires_id] = (DBExtWire*)new_part;
		}
	}

	int write_database()
	{
		MDB_val key, data;
		unsigned long long change = point_set_change;
		unsigned ext_change = ext_wire_change;
		int rc;
		Q_ASSERT(txn != NULL);

		if (change == 0 && ext_change == 0)
			return 0;

		DBID id(area, META_TYPE, AREA_WIREVIA_INFO);
		key.mv_size = sizeof(id);
		key.mv_data = &id;
		data.mv_size = sizeof(db_info);
		data.mv_data = &db_info;
		ERR_CHECK1(mdb_put(txn, dbi, &key, &data, 0), -1);
		for (int i = 0; i < 64; i++) {
			if (change & 1) {
				unsigned part_y = i >> 4;
				unsigned part_x = (i >> 2) & 3;
				unsigned part = i >> 2;
				unsigned local = i & 3;
				DBID id(area, 0, part_x, part_y, local);			
				key.mv_size = sizeof(id);
				key.mv_data = &id;
				data.mv_size = part_size[part][local];
				data.mv_data = part_points[part][local];
				if (part_size[part][local] == 0)
					mdb_del(txn, dbi, &key, &data);
				else 
					ERR_CHECK1(mdb_put(txn, dbi, &key, &data, 0), -1);
			}
			change = change >> 1;
		}
		for (int i = 0; i < EXT_WIRE_NUM; i++) {
			if (ext_change & 1) {
				DBID id(area, META_TYPE, EXT_WIRE_START + i);
				key.mv_size = sizeof(id);
				key.mv_data = &id;
				data.mv_size = ext_wire_size[i];
				data.mv_data = ext_wires[i];			
				if (ext_wire_size[i] == 0)
					mdb_del(txn, dbi, &key, &data);
				else
					ERR_CHECK1(mdb_put(txn, dbi, &key, &data, 0), -1);
			}
			ext_change = ext_change >> 1;
		}
		return 0;
	}

	unsigned char get_local(unsigned x, unsigned y)
	{
		unsigned char search_local = HASH_SPLIT(x,y) ? 2 : 0;
		unsigned char part_y = DBID::grid2part(y);
		unsigned char part_x = DBID::grid2part(x);
		unsigned char part_yx = part_y * 4 + part_x;

		if (db_info.part_point_num[part_yx][0] == 0 && db_info.part_point_num[part_yx][2] == 0)
			search_local = 4;  //empty
		else {
			if (db_info.part_point_num[part_yx][0] == 0) {
				if (search_local == 0) //local 0 is empty
					search_local = 4;
			}
			else {
				if (db_info.part_point_num[part_yx][2] == 0) { //local 2 is empty
					Q_ASSERT(db_info.part_point_num[part_yx][3] == 0);
					if (db_info.part_point_num[part_yx][1] == 0)
						search_local = 0;
					else
						if (search_local == 2)
							search_local = 6;
				}
			}
		}

		if (search_local <= 3) {
			if (db_info.part_point_num[part_yx][search_local + 1] != 0) {
				if (HASH_SPLIT(y,x))
					search_local++;
			}
		}

		return search_local;
	}

	unsigned char get_ext_wire_id(unsigned x, unsigned y)
	{
		unsigned char search_id = HASH_SPLIT(x, y) ? 1 : 0;
		if (db_info.ext_wire_num[0] == 0 && db_info.ext_wire_num[1] == 0)
			search_id = EXT_WIRE_NUM;
		else
			if (db_info.ext_wire_num[0] == 0) {
			if (search_id == 0)
				search_id = EXT_WIRE_NUM;
			}
			else {
				if (db_info.ext_wire_num[1] == 0)
					search_id = 0;
			}
			return search_id;
	}

public:
	enum {
		DO_CLONE,
		NOT_CLONE,
		AUTO_CLONE
	};
	MemAreaWireVia(MDB_txn * _txn = NULL, MDB_dbi _dbi = 0, unsigned _area = 0) 
	{	
		point_set_change = 0;
		ext_wire_change = 0;
		renew(_txn, _dbi, _area);
	}

	~MemAreaWireVia()
	{
		close(false);
	}

	unsigned get_area() const
	{
		return area;
	}
	
	int renew(MDB_txn *_txn, MDB_dbi _dbi, unsigned _area)
	{
		MDB_val key, data;
		int rc;
		
		close(false);
		txn = _txn;
		dbi = _dbi;
		area = _area;
		point_set_change = 0;
		ext_wire_change = 0;
		memset(&part_points[0][0], 0, sizeof(part_points));		
		memset(&part_size[0][0], 0, sizeof(part_size));
		memset(&ext_wires, 0, sizeof(ext_wires));
		memset(&ext_wire_size[0], 0, sizeof(ext_wire_size));
		if (_txn == NULL) 
			memset(&db_info, 0, sizeof(db_info));		
		else {
			DBID id(area, META_TYPE, AREA_WIREVIA_INFO);
			key.mv_size = sizeof(id);
			key.mv_data = &id;			
			rc = mdb_get(txn, dbi, &key, &data);
			if (rc == MDB_NOTFOUND) 
				memset(&db_info, 0, sizeof(db_info));			
			else {
				ERR_CHECK1(rc, -1);
				Q_ASSERT(data.mv_size == sizeof(db_info));
				memcpy(&db_info, data.mv_data, sizeof(db_info));
			}		
		}
		return 0;
	}

	int close(bool submit)
	{
		if (point_set_change == 0 && ext_wire_change == 0)
			return 0;

		if (submit)
			if (write_database()!=0)
				return -1;

		for (int i = 0; i < 64; i++) {
			unsigned part = i >> 2;
			unsigned local = i & 3;
			if (point_set_change & 1)
				free((unsigned short*)(part_points[part][local]) - 1);
			point_set_change = point_set_change >> 1;
		}
		point_set_change = 0;
		
		for (int i = 0; i < EXT_WIRE_NUM; i++) {
			if (ext_wire_change & 1)
				free((unsigned short*)(ext_wires[i]) - 1);
			ext_wire_change = ext_wire_change >> 1;
		}
		ext_wire_change = 0;
		return 0;
	}

	//points is valid until database item contain points is changed, caller can read/write return points
	//if part_points is in database, return points is clone-on-write, if it is in memory, return points is already cloned
	int get_point(unsigned x, unsigned y, vector<MemVWPoint> & points, unsigned char layer = 0xff, int need_clone = AUTO_CLONE)
	{
		Q_ASSERT(DBID::xy2id_area(x, y) == area);
		unsigned char search_local = get_local(x, y);
		vector<PII> idx;
		bool clone = false;

		points.clear();
		if (search_local > 3)
			return 0;
		DBID id(x, y, WIRE_TYPE, search_local);
		if ((((point_set_change >> id.get_id_part()) & 1) && need_clone == AUTO_CLONE) ||
			need_clone == DO_CLONE)
			clone = true; //in memory, need clone
		return _get_point(id, x, y, points, idx, clone, layer);		
	}

	//caller can read/write return points
	//if part_points is in database, return points is clone-on-write, if it is in memory, return points is already cloned
	int get_points_all(vector<MemVWPoint> & points, bool append = false, unsigned char layer = 0xff, unsigned int need_clone = AUTO_CLONE)
	{
		if (!append)
			points.clear();
		if (layer != 0xff)
			Q_ASSERT(layer < 32);
		for (unsigned char part_yx = 0; part_yx < 16; part_yx++)
			for (unsigned char local = 0; local <= 3; local++) {
				if (db_info.part_point_num[part_yx][local] == 0)
					continue;
				DBID id(area, WIRE_TYPE, ((part_yx << 2) | local));
				DBPoint * search_set = part_points[part_yx][local];
				if (search_set == NULL) {
					MDB_val key, data;
					int rc;
					key.mv_size = sizeof(id);
					key.mv_data = &id;
					ERR_CHECK1(mdb_get(txn, dbi, &key, &data), -1);
					search_set = (DBPoint *)data.mv_data;
					part_points[part_yx][local] = search_set;
					Q_ASSERT(data.mv_size <= 64000);
					part_size[part_yx][local] = (unsigned short)data.mv_size;
				}
				Q_ASSERT(db_info.part_point_num[part_yx][local] * sizeof(DBPoint) < part_size[part_yx][local]);
				unsigned char * attach = (unsigned char *)&search_set[db_info.part_point_num[part_yx][local]];
				unsigned offset = 0;
				for (unsigned i = 0; i < db_info.part_point_num[part_yx][local]; i++) {					
					unsigned y = id.localy2grid(search_set[i].y);
					unsigned x = id.localx2grid(search_set[i].x);
				
					MemVWPoint point(x, y, search_set[i].pack_info, id.get_id_part(),
						attach + offset);
					if (layer == 0xff || (layer == INST_LAYER && point.get_isto_inst())
						|| (layer <= point.get_layer_max() && layer >= point.get_layer_min())) {
						points.push_back(point);
						if ((((point_set_change >> id.get_id_part()) & 1) && need_clone == AUTO_CLONE) ||
							need_clone == DO_CLONE)
							points[points.size() - 1].alloc_attach_mem(point.get_attach_size()); //clone
					}
					offset += GET_FIELD(search_set[i].pack_info, ATTACH_SIZE);
				}
			}
		
		return 0;
	}

	//if success return 0, else return nonzero
	int del_point(const MemVWPoint & p)
	{
		return del_point(p.x, p.y, p.get_pack_info());
	}

	//if success return 0, else return nonzero
	int del_point(unsigned x, unsigned y, unsigned pack_info)
	{
		unsigned char search_local = get_local(x, y);
		vector<MemVWPoint> points;
		vector<PII> idx;

		if (search_local > 3)
			return -2;
		DBID id(x, y, WIRE_TYPE, search_local);
		if (_get_point(id, x, y, points, idx, false, 0xff) < 0)
			return -1;
		/*I       O           Attach            p             q
		  first   first + 1     0            second      second + size*/
		unsigned char part_yx = id.get_id_part() >> 2;
		for (int i = 0; i < points.size(); i++) {
			if (points[i].get_pack_info() == pack_info) {				
				alloc_part_mem(id.get_id_part(), part_size[part_yx][search_local]);
				memmove(part_points[part_yx][search_local] + idx[i].first,
					part_points[part_yx][search_local] + idx[i].first + 1,
					(db_info.part_point_num[part_yx][search_local] - idx[i].first - 1)*sizeof(DBPoint) + idx[i].second);
				Q_ASSERT(part_size[part_yx][search_local] >= idx[i].second +points[i].get_attach_size() +
					db_info.part_point_num[part_yx][search_local] * sizeof(DBPoint));

				unsigned tail_size = part_size[part_yx][search_local] - idx[i].second - points[i].get_attach_size() -
					db_info.part_point_num[part_yx][search_local] * sizeof(DBPoint);
				unsigned char * point_t = (unsigned char *)(part_points[part_yx][search_local] + db_info.part_point_num[part_yx][search_local] - 1);
				unsigned char * point_ot = (unsigned char *)(part_points[part_yx][search_local] + db_info.part_point_num[part_yx][search_local]);
				memmove(point_t + idx[i].second, point_ot + idx[i].second + points[i].get_attach_size(), tail_size);	
				part_size[part_yx][search_local] -= (unsigned short) (sizeof(DBPoint) + points[i].get_attach_size());
				db_info.part_point_num[part_yx][search_local]--;				
				return 0;
			}
		}

		return -2;
	}

	//if success return 0, else return nonzero
	//input x, y, layer, if layer==0xff, delete (x,y) point without compare layer
	int del_point(unsigned x, unsigned y, unsigned char layer)
	{
		unsigned char search_local = get_local(x, y);
		vector<MemVWPoint> points;
		vector<PII> idx;

		if (search_local > 3)
			return -2;
		DBID id(x, y, WIRE_TYPE, search_local);
		if (_get_point(id, x, y, points, idx, false, 0xff) < 0)
			return -1;
		/*I       O           Attach            p             q
		first   first + 1     0            second      second + size*/
		unsigned char part_yx = id.get_id_part() >> 2;
		for (int i = 0; i < points.size(); i++) {
			if (layer == 0xff || (layer == INST_LAYER && points[i].get_isto_inst())
				|| (layer <= points[i].get_layer_max() && layer >= points[i].get_layer_min()))
			{
				alloc_part_mem(id.get_id_part(), part_size[part_yx][search_local]);
				memmove(part_points[part_yx][search_local] + idx[i].first,
					part_points[part_yx][search_local] + idx[i].first + 1,
					(db_info.part_point_num[part_yx][search_local] - idx[i].first - 1)*sizeof(DBPoint) + idx[i].second);
				Q_ASSERT(part_size[part_yx][search_local] >= idx[i].second + points[i].get_attach_size() +
					db_info.part_point_num[part_yx][search_local] * sizeof(DBPoint));

				unsigned tail_size = part_size[part_yx][search_local] - idx[i].second - points[i].get_attach_size() -
					db_info.part_point_num[part_yx][search_local] * sizeof(DBPoint);
				unsigned char * point_t = (unsigned char *)(part_points[part_yx][search_local] + db_info.part_point_num[part_yx][search_local] - 1);
				unsigned char * point_ot = (unsigned char *)(part_points[part_yx][search_local] + db_info.part_point_num[part_yx][search_local]);
				memmove(point_t + idx[i].second, point_ot + idx[i].second + points[i].get_attach_size(), tail_size);
				part_size[part_yx][search_local] -= (unsigned short)(sizeof(DBPoint) + points[i].get_attach_size());
				db_info.part_point_num[part_yx][search_local]--;
				return 0;
			}
		}

		return -2;
	}

	void split02(DBID id)
	{				
		unsigned char part_yx = id.get_id_part() >> 2;
		Q_ASSERT((id.get_id_part() & 3) == 0 && part_points[part_yx][0] != NULL && db_info.part_point_num[part_yx][2] == 0);
		DBPoint * points2 = (DBPoint*) malloc(db_info.part_point_num[part_yx][0] * sizeof(DBPoint));
		unsigned point2_num = 0;
		unsigned char * attach2 = (unsigned char *)malloc(part_size[part_yx][0] - db_info.part_point_num[part_yx][0] * sizeof(DBPoint));
		unsigned offset2 = 0;
		bool del = true;
		while (del) {
			del = false;
			unsigned offset0 = 0;
			unsigned char * attach0 = (unsigned char *)&part_points[part_yx][0][db_info.part_point_num[part_yx][0]];
			for (int i = 0; i < db_info.part_point_num[part_yx][0]; i++) {
				unsigned short size = GET_FIELD(part_points[part_yx][0][i].pack_info, ATTACH_SIZE);
				if (HASH_SPLIT(part_points[part_yx][0][i].x, part_points[part_yx][0][i].y)) {
					points2[point2_num++] = part_points[part_yx][0][i];
					memcpy(attach2 + offset2, attach0 + offset0, size);
					offset2 += size;
					del_point(id.localx2grid(part_points[part_yx][0][i].x), 
						id.localy2grid(part_points[part_yx][0][i].y), part_points[part_yx][0][i].pack_info);
					del = true;
					break;
				}
				offset0 += size;
			}
		}
		if (point2_num != 0) {
			alloc_part_mem(id.get_id_part() | 2, point2_num*sizeof(DBPoint) + offset2);
			memcpy(part_points[part_yx][2], points2, point2_num*sizeof(DBPoint));
			memcpy(part_points[part_yx][2] + point2_num, attach2, offset2);
			db_info.part_point_num[part_yx][2] = point2_num;
		}		
		free(points2);
		free(attach2);
	}

	void split01(DBID id)
	{
		unsigned char part_yx = id.get_id_part() >> 2;
		unsigned char local = id.get_id_part() & 2;
		Q_ASSERT((id.get_id_part() & 1) == 0 && part_points[part_yx][local] != NULL && db_info.part_point_num[part_yx][local + 1] == 0);
		DBPoint * points1 = (DBPoint*)malloc(db_info.part_point_num[part_yx][local] * sizeof(DBPoint));
		unsigned point1_num = 0;
		unsigned char * attach1 = (unsigned char *)malloc(part_size[part_yx][local] - db_info.part_point_num[part_yx][local] * sizeof(DBPoint));
		unsigned offset1 = 0;
		bool del = true;
		while (del) {
			del = false;
			unsigned offset0 = 0;
			unsigned char * attach0 = (unsigned char *)&part_points[part_yx][local][db_info.part_point_num[part_yx][local]];
			for (int i = 0; i < db_info.part_point_num[part_yx][local]; i++) {
				unsigned short size = GET_FIELD(part_points[part_yx][local][i].pack_info, ATTACH_SIZE);
				if (HASH_SPLIT(part_points[part_yx][local][i].y, part_points[part_yx][local][i].x)) {
					points1[point1_num++] = part_points[part_yx][local][i];
					memcpy(attach1 + offset1, attach0 + offset0, size);
					offset1 += size;
					del_point(id.localx2grid(part_points[part_yx][local][i].x), 
						id.localy2grid(part_points[part_yx][local][i].y), 
						part_points[part_yx][local][i].pack_info);
					del = true;
					break;
				}
				offset0 += size;
			}
		}
		if (point1_num != 0) {
			alloc_part_mem(id.get_id_part() | 1, point1_num*sizeof(DBPoint) + offset1);
			memcpy(part_points[part_yx][local + 1], points1, point1_num*sizeof(DBPoint));
			memcpy(part_points[part_yx][local + 1] + point1_num, attach1, offset1);
			db_info.part_point_num[part_yx][local + 1] = point1_num;
		}		
		free(points1);
		free(attach1);
	}

	//if success, return 0
	int add_point(const MemVWPoint & p)
	{
		unsigned char local = get_local(p.x, p.y);
		vector<MemVWPoint> points;		
		unsigned short local_y = DBID::grid2local(p.y), local_x = DBID::grid2local(p.x);
		unsigned local_yx =  MAKE_U32(local_y, local_x);
		
		if (local > 3)
			local -= 4;
		
		DBID id(p.x, p.y, WIRE_TYPE, local);
		unsigned char part_yx = id.get_id_part() >> 2;
		
		if (db_info.part_point_num[part_yx][local] != 0) {
			DBPoint * search_set = part_points[part_yx][local];
			if (search_set == NULL) {
				MDB_val key, data;
				int rc;
				key.mv_size = sizeof(id);
				key.mv_data = &id;
				ERR_CHECK1(mdb_get(txn, dbi, &key, &data), -1);
				search_set = (DBPoint *)data.mv_data;
				part_points[part_yx][local] = search_set;
				Q_ASSERT(data.mv_size <= 64000);
				part_size[part_yx][local] = (unsigned short)data.mv_size;
			}
		}
		else
			Q_ASSERT(part_size[part_yx][local] == 0);
		unsigned short old_size = part_size[part_yx][local];
		alloc_part_mem(id.get_id_part(), part_size[part_yx][local] + p.get_attach_size() + sizeof(DBPoint));
		
		DBPoint * search_set = part_points[part_yx][local];		
		unsigned offset = 0;
		unsigned i;
		for (i = 0; i < db_info.part_point_num[part_yx][local]; i++) {
			unsigned yx = MAKE_U32(search_set[i].y, search_set[i].x);

			if (yx == local_yx) {
				unsigned char omin = GET_FIELD(search_set[i].pack_info, LAYER_MIN);
				unsigned char omax = GET_FIELD(search_set[i].pack_info, LAYER_MAX);
				unsigned char nmin = p.get_layer_min();
				unsigned char nmax = p.get_layer_max();
				if (omax != 0 && nmax != 0) {
					if ((omin >= nmin && omin <= nmax) || (omax >= nmin && omax <= nmax) ||
						(nmin >= omin && nmin <= omax) || (nmax >= omin && nmax <= omax)) {
						part_size[part_yx][local] = old_size; //TODO: avoid unnecessary point change
						return -2;
					}
						
				}
			}
			if (yx > local_yx) 
				break;
			
			offset += GET_FIELD(search_set[i].pack_info, ATTACH_SIZE);
		}
		
		unsigned tail_size = old_size - offset - db_info.part_point_num[part_yx][local] * sizeof(DBPoint);
		unsigned char * point_ot = (unsigned char *)(part_points[part_yx][local] + db_info.part_point_num[part_yx][local]);
		memmove(point_ot + offset + p.get_attach_size() + sizeof(DBPoint), point_ot + offset, tail_size);

		memmove(part_points[part_yx][local] + i + 1,
			part_points[part_yx][local] + i,
			(db_info.part_point_num[part_yx][local] - i)*sizeof(DBPoint) + offset);

		part_points[part_yx][local][i].x = local_x;
		part_points[part_yx][local][i].y = local_y;
		part_points[part_yx][local][i].pack_info = p.get_pack_info();

		memcpy(point_ot + offset + sizeof(DBPoint), p.attach, p.get_attach_size());		
		db_info.part_point_num[part_yx][local]++;
		if (local == 0 && db_info.part_point_num[part_yx][2] == 0 && db_info.part_point_num[part_yx][local] >= SPLIT_TH)
			split02(id);
		if ((local & 1) == 0 && db_info.part_point_num[part_yx][local + 1] == 0 &&
			db_info.part_point_num[part_yx][local] >= SPLIT_TH)
			split01(id);
		return 0;
	}

	int get_ext_wire_all(vector<DBExtWire> & ext_wire_set)
	{
		ext_wire_set.clear();
		for (int i = 0; i < EXT_WIRE_NUM; i++) {
			if (db_info.ext_wire_num[i] != 0 && ext_wires[i] == NULL) {
				DBID id(area, META_TYPE, EXT_WIRE_START + i);
				MDB_val key, data;
				int rc;
				key.mv_size = sizeof(id);
				key.mv_data = &id;
				ERR_CHECK1(mdb_get(txn, dbi, &key, &data), -1);
				Q_ASSERT(data.mv_size <= 64000 && data.mv_size == db_info.ext_wire_num[i] * sizeof(DBExtWire));
				ext_wires[i] = (DBExtWire *)data.mv_data;
				ext_wire_size[i] = (unsigned short)data.mv_size;
			}
			ext_wire_set.insert(ext_wire_set.end(), ext_wires[i], ext_wires[i] + db_info.ext_wire_num[i]);
		}
		return 0;
	}

	int del_ext_wire(unsigned x0, unsigned y0, unsigned x1, unsigned y1, unsigned char layer)
	{
		unsigned char search_id = get_ext_wire_id(x0, y0);
		if (search_id >= EXT_WIRE_NUM)
			return -1;
		if (db_info.ext_wire_num[search_id] == 0)
			return -1;
		DBID id(area, META_TYPE, EXT_WIRE_START + search_id);
		DBExtWire * search_set = ext_wires[search_id];
		if (search_set == NULL) {
			MDB_val key, data;
			int rc;
			key.mv_size = sizeof(id);
			key.mv_data = &id;
			ERR_CHECK1(mdb_get(txn, dbi, &key, &data), -1);
			search_set = (DBExtWire *)data.mv_data;
			ext_wires[search_id] = search_set;
			Q_ASSERT(data.mv_size <= 64000 && data.mv_size == db_info.ext_wire_num[search_id] * sizeof(DBExtWire));
			ext_wire_size[search_id] = (unsigned short)data.mv_size;
		}

		unsigned long long yx0 = MAKE_U64(y0, x0);
		unsigned long long yx1 = MAKE_U64(y1, x1);
		unsigned i;
		for (i = 0; i < db_info.ext_wire_num[search_id]; i++) {
			unsigned long long search_yx0 = MAKE_U64(search_set[i].y0, search_set[i].x0);

			if (search_yx0 == yx0) {
				unsigned long long search_yx1 = MAKE_U64(search_set[i].y1, search_set[i].x1);
				if (search_yx1 == yx1) {
					if (GET_FIELD(search_set[i].pack_info, EXTWIRE_LAYER) == layer)
						break;
				}
				if (search_yx1 > yx1)
					i = db_info.ext_wire_num[search_id];
			}
			if (search_yx0 > yx0)
				i = db_info.ext_wire_num[search_id];
		}

		if (i < db_info.ext_wire_num[search_id]) {
			db_info.ext_wire_num[search_id]--;
			alloc_ext_mem(search_id, db_info.ext_wire_num[search_id] * sizeof(DBExtWire));
			memmove(&ext_wires[search_id][i], &ext_wires[search_id][i + 1],
				(db_info.ext_wire_num[search_id] - i) * sizeof(DBExtWire));
			return 0;
		}
		else
			return -1;
	}

	int del_ext_wire(DBExtWire &ext_wire)
	{
		return del_ext_wire(ext_wire.x0, ext_wire.y0, ext_wire.x1, ext_wire.y1, GET_FIELD(ext_wire.pack_info, EXTWIRE_LAYER));
	}
	
	void split_ext_wire()
	{
		DBExtWire * ext_wire2 = (DBExtWire *)malloc(db_info.ext_wire_num[0] * sizeof(DBExtWire));
		unsigned ext_num2 = 0;
		bool del = true;
		while (del) {
			del = false;
			for (int i = 0; i < db_info.ext_wire_num[0]; i++) {
				if (HASH_SPLIT(ext_wires[0][i].x0, ext_wires[0][i].y0)) {
					ext_wire2[ext_num2++] = ext_wires[0][i];
					del_ext_wire(ext_wires[0][i]);
					del = true;
					break;
				}
			}
		}
		db_info.ext_wire_num[1] = ext_num2;
		if (ext_num2 != 0) {
			alloc_ext_mem(1, ext_num2*sizeof(DBExtWire));
			memcpy(ext_wires[1], ext_wire2, ext_num2 * sizeof(DBExtWire));
		}
		free(ext_wire2);
	}
	
	//if success, return 0, else return non-zero
	//internal set is order by (y0,x0), so better let (y0,x0)<(y1,x1) faster seaerch speed
	int add_ext_wire(unsigned x0, unsigned y0, unsigned x1, unsigned y1, unsigned char layer)
	{
		Q_ASSERT(DBID::xy2id_area(x0, y0) != area && DBID::xy2id_area(x1, y1) != area);
		unsigned char search_id = get_ext_wire_id(x0, y0);
		if (search_id >= EXT_WIRE_NUM)
			search_id -= EXT_WIRE_NUM;
		DBID id(area, META_TYPE, EXT_WIRE_START + search_id);	
		if (db_info.ext_wire_num[search_id] != 0) {
			DBExtWire * search_set = ext_wires[search_id];
			if (search_set == NULL) {
				MDB_val key, data;
				int rc;
				key.mv_size = sizeof(id);
				key.mv_data = &id;
				ERR_CHECK1(mdb_get(txn, dbi, &key, &data), -1);
				search_set = (DBExtWire *)data.mv_data;
				ext_wires[search_id] = search_set;
				Q_ASSERT(data.mv_size <= 64000 && data.mv_size == db_info.ext_wire_num[search_id] * sizeof(DBExtWire));
				ext_wire_size[search_id] = (unsigned short)data.mv_size;
			}
		}
		else
			Q_ASSERT(ext_wire_size[search_id] == 0);
				
		DBExtWire * search_set = ext_wires[search_id];
		unsigned long long yx0 = MAKE_U64(y0, x0);
		unsigned long long yx1 = MAKE_U64(y1, x1);
		unsigned i;
		for (i = 0; i < db_info.ext_wire_num[search_id]; i++) {
			unsigned long long search_yx0 = MAKE_U64(search_set[i].y0, search_set[i].x0);

			if (search_yx0 == yx0) {
				unsigned long long search_yx1 = MAKE_U64(search_set[i].y1, search_set[i].x1);
				if (search_yx1 == yx1) {
					if (GET_FIELD(search_set[i].pack_info, EXTWIRE_LAYER) == layer) {
						qCritical("add existing external line");
						return -2;
					}						
					else
						break;
				}
				if (search_yx1 > yx1)
					break;
			}
			if (search_yx0 > yx0)
				break;
		}

		db_info.ext_wire_num[search_id]++;
		alloc_ext_mem(search_id, db_info.ext_wire_num[search_id] * sizeof(DBExtWire));
		memmove(&ext_wires[search_id][i + 1], &ext_wires[search_id][i], (db_info.ext_wire_num[search_id] - i-1) * sizeof(DBExtWire));
		ext_wires[search_id][i].x0 = x0;
		ext_wires[search_id][i].y0 = y0;
		ext_wires[search_id][i].x1 = x1;
		ext_wires[search_id][i].y1 = y1;
		ext_wires[search_id][i].pack_info = 0;
		SET_FIELD(ext_wires[search_id][i].pack_info, EXTWIRE_LAYER, layer);

		if (search_id == 0 && db_info.ext_wire_num[1] == 0 && db_info.ext_wire_num[0] > SPLIT_TH)
			split_ext_wire();
		return 0;
	}

	int add_ext_wire(DBExtWire & dbw)
	{
		return add_ext_wire(dbw.x0, dbw.y0, dbw.x1, dbw.y1, GET_FIELD(dbw.pack_info, EXTWIRE_LAYER));
	}


	int intersect(const QLine & wire, unsigned char layer, bool exclude_p0, bool exclude_p1)
	{
		vector<MemVWPoint> points;
		int rc;
		get_points_all(points, false, layer, NOT_CLONE);
		for (int i = 0; i < points.size(); i++) {
			if (exclude_p0 && points[i].x == wire.x1() && points[i].y == wire.y1())
				continue;
			if (exclude_p1 && points[i].x == wire.x2() && points[i].y == wire.y2())
				continue;
			if ((rc = points[i].intersect(wire, layer, exclude_p0, exclude_p1)) != NOINTERSECTION)
				return rc;
		}
		for (int i = 0; i < EXT_WIRE_NUM; i++) {
			if (db_info.ext_wire_num[i] != 0 && ext_wires[i] == NULL) {
				DBID id(area, META_TYPE, EXT_WIRE_START + i);
				MDB_val key, data;
				key.mv_size = sizeof(id);
				key.mv_data = &id;
				ERR_CHECK1(mdb_get(txn, dbi, &key, &data), -1);
				Q_ASSERT(data.mv_size <= 64000 && data.mv_size == db_info.ext_wire_num[i] * sizeof(DBExtWire));
				ext_wires[i] = (DBExtWire *)data.mv_data;
				ext_wire_size[i] = (unsigned short)data.mv_size;
			}
			for (int j = 0; j < db_info.ext_wire_num[i]; j++) 
				if (layer == GET_FIELD(ext_wires[i][j].pack_info, EXTWIRE_LAYER)) {
					if (exclude_p0 && (ext_wires[i][j].x0 == wire.x1() && ext_wires[i][j].y0 == wire.y1() ||
						ext_wires[i][j].x1 == wire.x1() && ext_wires[i][j].y1 == wire.y1()))
						continue;
					if (exclude_p1 && (ext_wires[i][j].x0 == wire.x2() && ext_wires[i][j].y0 == wire.y2() ||
						ext_wires[i][j].x1 == wire.x2() && ext_wires[i][j].y1 == wire.y2()))
						continue;
					QPoint pp;
					rc = ::intersect(QLine(ext_wires[i][j].x0, ext_wires[i][j].y0, ext_wires[i][j].x1, ext_wires[i][j].y1), wire, pp);
					if (rc != NOINTERSECTION && rc != PARALLEL)
						return rc;
				}
		}
		return NOINTERSECTION;
	}
};

struct MemAreaWireViaCmp {
	bool operator ()(const MemAreaWireVia *lhs, const MemAreaWireVia *rhs) const {
		return lhs->get_area() > rhs->get_area();
	}
};

//return move distance, 0,1, 2
int move_toward(int src_x, int src_y, int dst_x, int dst_y, unsigned & area_x, unsigned & area_y)
{	
	int a_x = area_x, a_y = area_y;
	int dx = SGN((int)DBID::gridx2areax(dst_x) - a_x);
	int dy = SGN((int)DBID::gridy2areay(dst_y) - a_y);
	int ret = 1;
	if (dx == 0 && dy == 0)
		return 0;
	if (dx == 0)
		a_y += dy;
	else
		if (dy == 0)
			a_x += dx;
		else {
			unsigned left, top, right, bottom;
			DBID::area2gridrect(a_x + dx, a_y, left, top, right, bottom);
			if (cross_rect(QRect(left, top, right - left + 1, bottom - top + 1),
				QLine(src_x, src_y, dst_x, dst_y)))
				a_x += dx;
			else {
				DBID::area2gridrect(a_x, a_y + dy, left, top, right, bottom);
				if (cross_rect(QRect(left, top, right - left + 1, bottom - top + 1),
					QLine(src_x, src_y, dst_x, dst_y)))
					a_y += dy;
				else {
					DBID::area2gridrect(a_x + dx, a_y + dy, left, top, right, bottom);
					Q_ASSERT(cross_rect(QRect(left, top, right - left + 1, bottom - top + 1),
						QLine(src_x, src_y, dst_x, dst_y)));
					a_x += dx;
					ret = 1;
				}
			}

		}
	area_x = a_x;
	area_y = a_y;
	return ret;
}

enum {
	Success=0,
	ERR_INVALIDPARAM,
	ERR_PointSame,
	ERR_PointP0NotExist,
	ERR_PointP0Exist,
	ERR_PointP1NotExist,
	ERR_PointP1Exist,
	ERR_PointP2NotExist,
	ERR_PointP3NotExist,
	ERR_PointP0LineFull,
	ERR_PointP1LineFull,
	ERR_DelLineNotExist,
	ERR_AddLineExist,
	ERR_AddLineIntersect,
	ERR_PointAddFail,
	ERR_Internal,
};

class DBProject {
protected:
	map<unsigned, MemAreaWireVia *> modify_areas;
	MDB_env * env;
	MDB_txn * txn;
	MDB_dbi dbi;
	bool write_txn_active;
protected:
	MemAreaWireVia * get_area(unsigned area)
	{
		MemAreaWireVia * areavw;
		map<unsigned, MemAreaWireVia *>::iterator find_area = modify_areas.find(area);
		if (find_area == modify_areas.end()) {
			areavw = new MemAreaWireVia(txn, dbi, area);
			modify_areas[area] = areavw;
		}
		else
			areavw = find_area->second;
		return areavw;
	}
public:
	DBProject(MDB_env *env_, char * db_name)
	{
		int rc = MDB_SUCCESS;
		env = env_;
		ERR_CHECK(mdb_txn_begin(env, NULL, 0, &txn));
		if (sizeof(DBID) == 4)
			rc = mdb_dbi_open(txn, db_name, MDB_INTEGERKEY, &dbi);
		else
			rc = mdb_dbi_open(txn, db_name, 0, &dbi);
		if (rc != MDB_SUCCESS) {
			mdb_txn_abort(txn);
			qCritical(mdb_strerror(rc));
			return;
		}
		ERR_CHECK(mdb_txn_commit(txn));
	}

	~DBProject()
	{
		if (write_txn_active) {
			close_write_txn(false);
			mdb_txn_abort(txn);
		}		
	}
	int new_write_txn()
	{
		int rc;
		ERR_CHECK1(mdb_txn_begin(env, NULL, 0, &txn), -1);
		write_txn_active = true;
		return 0;
	}

	int close_write_txn(bool submit)
	{
		int rc;
		write_txn_active = false;
		for (map<unsigned, MemAreaWireVia *>::iterator it = modify_areas.begin(); it != modify_areas.end(); it++) {
			it->second->close(submit);
			delete it->second;
		}
		modify_areas.clear();
		if (submit) 
			ERR_CHECK1(mdb_txn_commit(txn), -1);
		else
			mdb_txn_abort(txn);		
		return 0;
	}

	//low level add wire, not checking wire intersection with other wire
	//if success, return 0, else return non-zero
	//input, vwp0, vwp1, these two point will be delete-modified-readd in database
	//input layer
	//output patch, old_points contain vwp0, vwp1, new_points contain vwp0, vwp1 after modified
	int add_wire_nocheck(const MemVWPoint & vwp0, const MemVWPoint & vwp1, unsigned char layer, vector<PointPatch *> & patches)
	{		
		if (vwp0.x == vwp1.x && vwp0.y == vwp1.y)
			return ERR_PointSame;
		
		MemVWPoint *p0, *p1, *new_p0, *new_p1;
		if (vwp0.y < vwp1.y || (vwp0.y == vwp1.y && vwp0.x < vwp1.x)) {
			p0 = new MemVWPoint(vwp0);
			p1 = new MemVWPoint(vwp1);
		}
		else {
			p0 = new MemVWPoint(vwp1);
			p1 = new MemVWPoint(vwp0);
		}

		unsigned area0_x, area0_y, area0, area1_x, area1_y, area1;
		int rc;
		area0_x = DBID::gridx2areax(p0->x);
		area0_y = DBID::gridy2areay(p0->y);
		area0 = DBID::xy2id_area(p0->x, p0->y);
		area1_x = DBID::gridx2areax(p1->x);
		area1_y = DBID::gridy2areay(p1->y);
		area1 = DBID::xy2id_area(p1->x, p1->y);

		MemAreaWireVia * areavw0 = get_area(area0);
		if (p0->get_pack_info() != 0)
			if ((rc=areavw0->del_point(*p0)) != 0) {
				qCritical("vw0 del point error %d", rc);
				delete p0;
				delete p1;
				return ERR_PointP0NotExist;
			}

		MemAreaWireVia * areavw1 = get_area(area1);		
		if (p1->get_pack_info() != 0) 
			if ((rc=areavw1->del_point(*p1)) != 0) {
				qCritical("vw1 del point error %d", rc);
				delete p0;
				delete p1;
				return ERR_PointP1NotExist;
			}

		new_p0 = new MemVWPoint(*p0);		
		if ((rc = new_p0->add_noninst_wire(layer, p1->x, p1->y)) != 0) {
			qCritical("Pointp0 add wire error %d", rc);
			delete new_p0;
			delete p0;
			delete p1;
			return ERR_PointP0LineFull;
		}
		if ((rc = areavw0->add_point(*new_p0)) != 0) {
			qCritical("areavw0 add wire error %d", rc);
			delete new_p0;
			delete p0;
			delete p1;
			return ERR_PointAddFail;
		}

		new_p1 = new MemVWPoint(*p1);
		if ((rc=new_p1->add_noninst_wire(layer, p0->x, p0->y)) != 0) {
			qCritical("Pointp1 add wire error %d", rc);
			delete new_p0;
			delete new_p1;
			delete p0;
			delete p1;
			return ERR_PointP1LineFull;
		}
		if ((rc = areavw1->add_point(*new_p1)) != 0) {
			qCritical("areavw1 add wire error %d", rc);
			delete new_p0;
			delete new_p1;
			delete p0;
			delete p1;
			return ERR_PointAddFail;
		}

		if (area0 != area1) {
			move_toward(p0->x, p0->y, p1->x, p1->y, area0_x, area0_y);
			while (area0_x != area1_x || area0_y != area1_y) {
				MemAreaWireVia * areavw;
				unsigned area = DBID::areaxy2area(area0_x, area0_y);
				areavw = get_area(area);					
				if ((rc = areavw->add_ext_wire(p0->x, p0->y, p1->x, p1->y, layer)) != 0) {
					qCritical("areavw add external wire error %d", rc);
					delete new_p0;
					delete new_p1;
					delete p0;
					delete p1;
					return ERR_AddLineExist;
				}					
				move_toward(p0->x, p0->y, p1->x, p1->y, area0_x, area0_y);
			}	
		}
		PointPatch * patch = new PointPatch();
		if (p0->get_pack_info() != 0) {
			patch->old_points.push_back(p0);
			patch->action |= ADDLINE_MODIFY_P0;
		}			
		else {
			patch->action |= ADDLINE_CREATE_P0;
			delete p0;
		}			
		if (p1->get_pack_info() != 0) {
			patch->old_points.push_back(p1);
			patch->action |= ADDLINE_MODIFY_P1;
		}			
		else {
			patch->action |= ADDLINE_CREATE_P1;
			delete p1;
		}			
		patch->new_points.push_back(new_p0);
		patch->new_points.push_back(new_p1);
		patches.push_back(patch);
		return Success;
	}

	//low level delete wire, not checking wire intersection with other wire
	//if success, return 0, else return non-zero
	//input, vwp0, vwp1, these two point will be delete in database
	//output patch
	int del_wire_nocheck(const MemVWPoint & vwp0, const MemVWPoint & vwp1, unsigned char layer, vector<PointPatch*> & patches)
	{		
		if (vwp0.x == vwp1.x && vwp0.y == vwp1.y)
			return -10;
		
		MemVWPoint *p0, *p1, *new_p0, *new_p1;
		if (vwp0.y < vwp1.y || (vwp0.y == vwp1.y && vwp0.x < vwp1.x)) {
			p0 = new MemVWPoint(vwp0);
			p1 = new MemVWPoint(vwp1);
		}
		else {
			p0 = new MemVWPoint(vwp1);
			p1 = new MemVWPoint(vwp0);
		}
				
		unsigned area0_x, area0_y, area0, area1_x, area1_y, area1;
		int rc;
		area0_x = DBID::gridx2areax(p0->x);
		area0_y = DBID::gridy2areay(p0->y);
		area0 = DBID::xy2id_area(p0->x, p0->y);
		area1_x = DBID::gridx2areax(p1->x);
		area1_y = DBID::gridy2areay(p1->y);
		area1 = DBID::xy2id_area(p1->x, p1->y);

		MemAreaWireVia * areavw0 = get_area(area0);
		Q_ASSERT(p0->get_pack_info() != 0);
		if (areavw0->del_point(*p0) != 0) {
			delete p0;
			delete p1;
			return ERR_PointP0NotExist;
		}

		MemAreaWireVia * areavw1 = get_area(area1);
		Q_ASSERT(p1->get_pack_info() != 0);
		if (areavw1->del_point(*p1) != 0) {
			delete p0;
			delete p1;
			return ERR_PointP1NotExist;
		}

		new_p0 = new MemVWPoint(*p0);		
		if ((rc = new_p0->delete_layer_wire(layer, p1->x, p1->y)) != 0) {
			qCritical("PointP0 delete wire error=%d", rc);
			delete new_p0;
			delete p0;
			delete p1;
			return ERR_DelLineNotExist;
		}
		if (new_p0->get_pack_info() != 0)
			if ((rc = areavw0->add_point(*new_p0)) != 0) {
				qCritical("areavw %d delete wire error=%d", area0, rc);
				delete new_p0;
				delete p0;
				delete p1;
				return ERR_PointAddFail;
			}

		new_p1 = new MemVWPoint(*p1);		
		if ((rc = new_p1->delete_layer_wire(layer, p0->x, p0->y)) != 0) {
			qCritical("PointP1 delete wire error=%d", rc);
			delete new_p0;
			delete new_p1;
			delete p0;
			delete p1;
			return ERR_DelLineNotExist;
		}
		if (new_p1->get_pack_info() != 0)
			if ((rc = areavw1->add_point(*new_p1)) != 0) {
				qCritical("areavw %d delete wire error=%d", area1, rc);
				delete new_p0;
				delete new_p1;
				delete p0;
				delete p1;
				return ERR_PointAddFail;
			}
		if (area0 != area1) {
			move_toward(p0->x, p0->y, p1->x, p1->y, area0_x, area0_y);
			while (area0_x != area1_x || area0_y != area1_y) {				
				MemAreaWireVia * areavw;
				unsigned area = DBID::areaxy2area(area0_x, area0_y);
				areavw = get_area(area);
				if ((rc = areavw->del_ext_wire(p0->x, p0->y, p1->x, p1->y, layer)) != 0) {
					qCritical("areavw %d del external wire error %d", area, rc);
					delete new_p0;
					delete new_p1;
					delete p0;
					delete p1;
					return ERR_Internal;
				}
				move_toward(p0->x, p0->y, p1->x, p1->y, area0_x, area0_y);
			}
		}
		PointPatch * patch = new PointPatch();
		patch->old_points.push_back(p0);
		patch->old_points.push_back(p1);
		if (new_p0->get_pack_info() != 0) {
			patch->new_points.push_back(new_p0);
			patch->action |= DELLINE_MODIFY_P0;
		}			
		else {
			patch->action |= DELLINE_DELETE_P0;
			delete new_p0;
		}
			
		if (new_p1->get_pack_info() != 0) {
			patch->action |= DELLINE_MODIFY_P1;
			patch->new_points.push_back(new_p1);
		}			
		else {
			patch->action |= DELLINE_DELETE_P1;
			delete new_p1;
		}			
		patches.push_back(patch);
		return Success;
	}

	int get_internal_points(unsigned area, unsigned get_method, vector<MemVWPoint> & points, bool append, bool from_database = true)
	{
		MemAreaWireVia * areavw;
		if (from_database) {
			int rc;
			MDB_txn * txn_rd;
			ERR_CHECK1(mdb_txn_begin(env, NULL, MDB_RDONLY, &txn_rd), -1);			
			areavw = new MemAreaWireVia(txn_rd, dbi, area);
			areavw->get_points_all(points, append, 0xff, MemAreaWireVia::AUTO_CLONE);
			delete areavw;
			mdb_txn_abort(txn_rd);
		}
		else {
			areavw = get_area(area);
			areavw->get_points_all(points, append, 0xff, MemAreaWireVia::AUTO_CLONE);
		}
		return 0;
	}

	void free_patch(vector<PointPatch*> & patch)
	{
		for (int i = 0; i < patch.size(); i++)
			delete patch[i];
		patch.clear();
	}

	//if success return 0, else return nonzero
	int add_wire(const QLine & wire, unsigned char layer, bool p0_is_new, bool p1_is_new, vector<PointPatch*> & patch)
	{
		if (wire.x1() == wire.x2() && wire.y1() == wire.y2())
			return ERR_INVALIDPARAM;
		unsigned area0_x, area0_y, area0, area1_x, area1_y, area1;
		int rc;
		area0_x = DBID::gridx2areax(wire.x1());
		area0_y = DBID::gridy2areay(wire.y1());
		area0 = DBID::xy2id_area(wire.x1(), wire.y1());
		area1_x = DBID::gridx2areax(wire.x2());
		area1_y = DBID::gridy2areay(wire.y2());
		area1 = DBID::xy2id_area(wire.x2(), wire.y2());

		vector<MemVWPoint> p0, p1;
		MemAreaWireVia * areavw0 = get_area(area0);
		areavw0->get_point(wire.x1(), wire.y1(), p0, layer, MemAreaWireVia::DO_CLONE);
		if (p0_is_new) {			
			if (!p0.empty())
				return ERR_PointP0Exist;
			p0.push_back(MemVWPoint(wire.x1(), wire.y1()));
		}
		else
			if (p0.empty())
				return ERR_PointP0NotExist;
		Q_ASSERT(p0.size() == 1);
		MemAreaWireVia * areavw1 = get_area(area1);
		areavw1->get_point(wire.x2(), wire.y2(), p1, layer, MemAreaWireVia::DO_CLONE);
		if (p1_is_new) {
			if (!p1.empty())
				return ERR_PointP1Exist;
			p1.push_back(MemVWPoint(wire.x2(), wire.y2()));
		}
		else
			if (p1.empty())
				return ERR_PointP1NotExist;
		Q_ASSERT(p1.size() == 1);

		if ((rc = areavw0->intersect(wire, layer, !p0_is_new, !p1_is_new)) != NOINTERSECTION) {
			//qWarning("add line intersect with existing line");
			return ERR_AddLineIntersect;
		}

		while (area0_x != area1_x || area0_y != area1_y) {
			move_toward(wire.x1(), wire.y1(), wire.x2(), wire.y2(), area0_x, area0_y);
			MemAreaWireVia * areavw;
			unsigned area = DBID::areaxy2area(area0_x, area0_y);
			areavw = get_area(area);
			if ((rc = areavw->intersect(wire, layer, !p0_is_new, !p1_is_new)) != NOINTERSECTION) {
				//qWarning("add line intersect with existing line");
				return ERR_AddLineIntersect;
			}
		} 

		return add_wire_nocheck(p0[0], p1[0], layer, patch);
	}

	int make_point_on_line(const QLine & wire, QPoint & point, unsigned char layer, vector<PointPatch *> & patches)
	{
		if (wire.x1() == wire.x2() && wire.y1() == wire.y2())
			return ERR_INVALIDPARAM;

		unsigned area0, area1;
		int rc;
		area0 = DBID::xy2id_area(wire.x1(), wire.y1());
		area1 = DBID::xy2id_area(wire.x2(), wire.y2());

		vector<MemVWPoint> p0, p1;
		MemAreaWireVia * areavw0 = get_area(area0);
		areavw0->get_point(wire.x1(), wire.y1(), p0, layer, MemAreaWireVia::DO_CLONE);
		if (p0.empty())
			return ERR_PointP0NotExist;
		Q_ASSERT(p0.size() == 1);

		MemAreaWireVia * areavw1 = get_area(area1);
		areavw1->get_point(wire.x2(), wire.y2(), p1, layer, MemAreaWireVia::DO_CLONE);
		if (p1.empty())
			return ERR_PointP1NotExist;
		Q_ASSERT(p1.size() == 1);
				
		rc = del_wire_nocheck(p0[0], p1[0], layer, patches);
		if (rc != Success) {
			free_patch(patches);
			return rc;
		}
		bool p0_is_new = ((patches.back()->action & DELLINE_DELETE_P0) != 0);
		bool p1_is_new = ((patches.back()->action & DELLINE_DELETE_P1) != 0);
		MemVWPoint old_p0(wire.x1(), wire.y1()), old_p1(wire.x2(), wire.y2());
		MemVWPoint * old_pp0, *old_pp1;
		if (patches.back()->action & DELLINE_DELETE_P0)
			old_pp0 = &old_p0;
		else
			old_pp0 = dynamic_cast <MemVWPoint *> (patches.back()->new_points[0]);
		if (patches.back()->action & DELLINE_DELETE_P1)
			old_pp1 = &old_p1;
		else
			old_pp1 = dynamic_cast <MemVWPoint *> (patches.back()->new_points.back());

		if (!online(wire, point)) {				
			rc = add_wire(QLine(wire.x1(), wire.y1(), point.x(), point.y()), layer, p0_is_new, true, patches);
			if (rc != Success) {
				free_patch(patches);
				return rc;
			}			
			rc = add_wire(QLine(wire.x2(), wire.y2(), point.x(), point.y()), layer, p1_is_new, false, patches);
			if (rc != Success) {
				free_patch(patches);
				return rc;
			}			
		}
		else {	
			MemVWPoint *new_pp;
			rc = add_wire_nocheck(*old_pp0, MemVWPoint(point.x(), point.y()), layer, patches);
			if (rc != Success) {
				free_patch(patches);
				return rc;
			}			
			new_pp = dynamic_cast <MemVWPoint *> (patches.back()->new_points.back());
			rc = add_wire_nocheck(*old_pp1, *new_pp, layer, patches);
			if (rc != Success) {
				free_patch(patches);
				return rc;
			}			
		}		
		return Success;
	}

	int delete_wire(const QLine & org_wire, const QLine & del_wire, unsigned char layer, vector<PointPatch *> & patches)
	{
		if (del_wire.x1() == del_wire.x2() && del_wire.y1() == del_wire.y2())
			return ERR_INVALIDPARAM;

		unsigned area0, area1;
		int rc;
		area0 = DBID::xy2id_area(org_wire.x1(), org_wire.y1());
		area1 = DBID::xy2id_area(org_wire.x2(), org_wire.y2());

		vector<MemVWPoint> p0, p1;
		MemAreaWireVia * areavw0 = get_area(area0);
		areavw0->get_point(org_wire.x1(), org_wire.y1(), p0, layer, MemAreaWireVia::DO_CLONE);
		if (p0.empty())
			return ERR_PointP0NotExist;
		Q_ASSERT(p0.size() == 1);

		MemAreaWireVia * areavw1 = get_area(area1);
		areavw1->get_point(org_wire.x2(), org_wire.y2(), p1, layer, MemAreaWireVia::DO_CLONE);
		if (p1.empty())
			return ERR_PointP1NotExist;
		Q_ASSERT(p1.size() == 1);

		rc = del_wire_nocheck(p0[0], p1[0], layer, patches);
		if (rc != Success) {
			free_patch(patches);
			return rc;
		}
		bool p0_is_new = ((patches.back()->action & DELLINE_DELETE_P0) != 0);
		bool p1_is_new = ((patches.back()->action & DELLINE_DELETE_P1) != 0);
		MemVWPoint old_p0(org_wire.x1(), org_wire.y1()), old_p1(org_wire.x2(), org_wire.y2());
		MemVWPoint * old_pp0, *old_pp1;
		if (patches.back()->action & DELLINE_DELETE_P0)
			old_pp0 = &old_p0;
		else
			old_pp0 = dynamic_cast <MemVWPoint *> (patches.back()->new_points[0]);
		if (patches.back()->action & DELLINE_DELETE_P1)
			old_pp1 = &old_p1;
		else
			old_pp1 = dynamic_cast <MemVWPoint *> (patches.back()->new_points.back());

		if (org_wire.p1() != del_wire.p1()) {
			if (!online(org_wire, del_wire.p1())) {
				rc = add_wire(QLine(org_wire.x1(), org_wire.y1(), del_wire.x1(), del_wire.y1()), layer, p0_is_new, true, patches);
				if (rc != Success) {
					free_patch(patches);
					return rc;
				}
			}
			else {
				MemVWPoint new_p(del_wire.x1(), del_wire.y1());
				rc = add_wire_nocheck(*old_pp0, new_p, layer, patches);
				if (rc != Success) {
					free_patch(patches);
					return rc;
				}
			}
		}
		
		if (org_wire.p2() != del_wire.p2()) {
			if (!online(org_wire, del_wire.p2())) {
				rc = add_wire(QLine(org_wire.x2(), org_wire.y2(), del_wire.x2(), del_wire.y2()), layer, p1_is_new, true, patches);
				if (rc != Success) {
					free_patch(patches);
					return rc;
				}
			}
			else {
				MemVWPoint new_p(del_wire.x2(), del_wire.y2());
				rc = add_wire_nocheck(*old_pp1, new_p, layer, patches);
				if (rc != Success) {
					free_patch(patches);
					return rc;
				}
			}
		}				
		return Success;
	}
    /*int add_via(unsigned x0, unsigned y0, unsigned char l0, unsigned char l1, vector<PointPatch> & patch);
    int add_inst(unsigned x0, unsigned y0, unsigned short template_dir, vector<PointPatch> & patch);
    int add_connect(unsigned x0, unsigned y0, unsigned char l0, DBID id1,
                     unsigned x1, unsigned y1, unsigned short type, unsigned short port1, DBID id2, vector<PointPatch> & patch);
    int add_name(unsigned x0, unsigned y0, unsigned short type, DBID id, const char * name, vector<PointPatch> & patch);
    int add_parameter(unsigned x0, unsigned y0, unsigned short type, DBID id,
                      const char * para_name, const char * para_data, vector<PointPatch> & patch);   */ 


    /*int del_via(unsigned x0, unsigned y0, DBID id, vector<PointPatch> & patch);
    int del_inst(unsigned x0, unsigned y0, unsigned short template_dir, DBID &id, vector<PointPatch> & patch);
    int del_connect(unsigned x0, unsigned y0, unsigned char l0, DBID id1,
                    unsigned x1, unsigned y1, unsigned short type, unsigned short port1, DBID id2, vector<PointPatch> & patch);
    int del_name(unsigned x0, unsigned y0, unsigned short type, DBID id, vector<PointPatch> & patch);
    int del_parameter(unsigned x0, unsigned y0, unsigned short type, DBID id,
                      const char * para_name, vector<PointPatch> & patch);
    */
};
#endif // ELEMENT_DB_H
