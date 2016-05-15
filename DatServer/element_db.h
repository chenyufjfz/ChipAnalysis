#ifndef ELEMENT_DB_H
#define ELEMENT_DB_H

#include <QLine>
#include "element.h"
#include "lmdb.h"
#include <set>


#pragma pack(push)
#pragma pack(1)
typedef struct {    
    unsigned short y, x;
    unsigned pack_info; //layer_num(5), layer_min(5), layer_max(5) score(4), template(13), dir(3), flag(4), attach_ref(11)
} DBPoint;

typedef struct {						
	unsigned char part_point_num[16][4];
} DBAreaWireVia;

#pragma pack(pop)

#define HASH_MASK	0x20
#define SPLIT_TH	24
//Area contain 16(4*4) parts, each part have at most 4 entries, entry may be empty

class MemAreaWireVia {
protected:
	DBAreaWireVia db_info;						//saved in database
	unsigned short part_size[16][4];			//saved as data.size
	DBPoint * part_points[16][4];					//maybe point to memory or database	
	unsigned long long point_set_change;
	MDB_txn *txn;
	MDB_dbi dbi;
	unsigned area;

	typedef pair<int, int> PII;

protected:
	int _get_point(DBID id, unsigned x, unsigned y, vector<MemVWPoint> & points, vector<PII> &idx, bool clone_point=false)
	{
		points.clear();
		idx.clear();
		unsigned char part_yx = id.get_id_partyx();
		unsigned char local = id.get_id_local();
		unsigned short local_y = DBID::grid2local(y), local_x = DBID::grid2local(x);
		unsigned local_yx = MAKE_U32(local_y, local_x);		

		if (db_info.part_point_num[part_yx][local] == 0)
			return 0;
		DBPoint * search_set = part_points[part_yx][local];
		if (search_set == NULL) {			
			MDB_val key, data;
			int rc;
			key.mv_size = sizeof(id);
			key.mv_data = &id;
			rc = mdb_get(txn, dbi, &key, &data);
			if (rc != 0) {
				qCritical(mdb_strerror(rc));
				return -1;
			}
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
				points.push_back(point);
				if (clone_point)
					points[points.size() - 1].alloc_attach_mem(point.get_attach_size());
				PII idx_new = make_pair(i, offset);
				idx.push_back(idx_new);
			}
			if (yx > local_yx)
				break;
			offset += GET_FIELD(search_set[i].pack_info, ATTACH_SIZE);
		}
		return 0;
	}
	
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
			*((unsigned short*)new_part) = cap;
			new_part += 2;
			memcpy(new_part, part_points[part_yx][local], old_size);
			free((unsigned short*)(part_points[part_yx][local]) - 1);
			part_points[part_yx][local] = (DBPoint*) new_part;
		}
	}

	int write_database()
	{
		MDB_val key, data;
		unsigned long long change = point_set_change;
		int rc;
		Q_ASSERT(txn != NULL);

		if (change == 0)
			return 0;

		DBID id(area, META_TYPE, AREA_WIREVIA_INFO);
		key.mv_size = sizeof(id);
		key.mv_data = &id;
		data.mv_size = sizeof(db_info);
		data.mv_data = &db_info;
		rc = mdb_put(txn, dbi, &key, &data, 0);
		if (rc != 0) {
			qCritical(mdb_strerror(rc));
			return -1;
		}
		for (int i = 0; i < 64; i++) {
			unsigned part_y = i >> 4;
			unsigned part_x = (i >> 2) & 3;
			unsigned part = i >> 2;
			unsigned local = i & 3;
			DBID id(area, 0, part_x, part_y, local);
			if (change & 1) {
				key.mv_size = sizeof(id);
				key.mv_data = &id;
				data.mv_size = part_size[part][local];
				data.mv_data = part_points[part][local];
				if (part_size[part][local] == 0)
					rc = mdb_del(txn, dbi, &key, &data);
				else {
					rc = mdb_put(txn, dbi, &key, &data, 0);
					if (rc != 0) {
						qCritical(mdb_strerror(rc));
						return -1;
					}
				}
			}
			change = change >> 1;
		}
		return 0;
	}

public:
	MemAreaWireVia(MDB_txn * _txn = NULL, MDB_dbi _dbi = 0, unsigned _area = 0) 
	{	
		point_set_change = 0;
		renew(_txn, _dbi, _area);
	}

	~MemAreaWireVia()
	{
		if (point_set_change)
			close(false);
	}

	unsigned char get_local(unsigned x, unsigned y)
	{
		unsigned char search_local = ((y + 2 * x) & HASH_MASK) ? 2 : 0;
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
				if ((2 * y + x) & HASH_MASK)
					search_local++;
			}
		}
		
		return search_local;
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
		memset(&part_points[0][0], 0, sizeof(part_points));		
		memset(&part_size[0][0], 0, sizeof(part_size));
		if (_txn == NULL) 
			memset(&db_info, 0, sizeof(db_info));		
		else {
			DBID id(area, META_TYPE, AREA_WIREVIA_INFO);
			key.mv_size = sizeof(id);
			key.mv_data = &id;			
			rc = mdb_get(txn, dbi, &key, &data);
			if (rc != 0) {
				qCritical(mdb_strerror(rc));
				return -1;
			}
			Q_ASSERT(data.mv_size == sizeof(db_info));
			memcpy(&db_info, data.mv_data, sizeof(db_info));
		}
		return 0;
	}

	int close(bool submit)
	{
		if (point_set_change == 0)
			return 0;

		if (submit && point_set_change != 0)
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
		return 0;
	}

	//points is valid until database item contain points is changed
	int get_point(unsigned x, unsigned y, vector<MemVWPoint> & points)
	{
		Q_ASSERT(DBID::xy2id_area(x, y) == area);
		unsigned char search_local = get_local(x, y);
		vector<PII> idx;
		bool clone = false;

		points.clear();
		if (search_local > 3)
			return 0;
		DBID id(x, y, WIRE_TYPE, search_local);
		if ((point_set_change >> id.get_id_part()) & 1)
			clone = true; //in memory, need clone
		return _get_point(id, x, y, points, idx, clone);		
	}

	int get_points_all(vector<MemVWPoint> & points)
	{
		points.clear();
		
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
					rc = mdb_get(txn, dbi, &key, &data);
					if (rc != 0) {
						qCritical(mdb_strerror(rc));
						return -1;
					}
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
					points.push_back(point);
					if ((point_set_change >> id.get_id_part()) & 1)
						points[points.size() - 1].alloc_attach_mem(point.get_attach_size()); //clone
					offset += GET_FIELD(search_set[i].pack_info, ATTACH_SIZE);
				}
			}
		
		return 0;
	}

	int del_point(const MemVWPoint & p)
	{
		return del_point(p.x, p.y, p.get_pack_info());
	}

	int del_point(unsigned x, unsigned y, unsigned pack_info)
	{
		unsigned char search_local = get_local(x, y);
		vector<MemVWPoint> points;
		vector<PII> idx;

		if (search_local > 3)
			return -2;
		DBID id(x, y, WIRE_TYPE, search_local);
		if (_get_point(id, x, y, points, idx, false) < 0)
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
				if ((part_points[part_yx][0][i].y + 2 * part_points[part_yx][0][i].x) & HASH_MASK) {
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
		alloc_part_mem(id.get_id_part() | 2, point2_num*sizeof(DBPoint) + offset2);
		memcpy(part_points[part_yx][2], points2, point2_num*sizeof(DBPoint));
		memcpy(part_points[part_yx][2] + point2_num, attach2, offset2);
		db_info.part_point_num[part_yx][2] = point2_num;		
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
				if ((2 * part_points[part_yx][local][i].y + part_points[part_yx][local][i].x) & HASH_MASK) {
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
		alloc_part_mem(id.get_id_part() | 1, point1_num*sizeof(DBPoint) + offset1);
		memcpy(part_points[part_yx][local + 1], points1, point1_num*sizeof(DBPoint));
		memcpy(part_points[part_yx][local + 1] + point1_num, attach1, offset1);
		db_info.part_point_num[part_yx][local + 1] = point1_num;
		free(points1);
		free(attach1);
	}

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
				rc = mdb_get(txn, dbi, &key, &data);
				if (rc != 0) {
					qCritical(mdb_strerror(rc));
					return -1;
				}
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
						(nmin >= omin && nmin <= omax) || (nmax >= omin && nmax <= omax))
						return -2;
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
	
};


enum {
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
    EXTEND2             // [(])
} ;

bool get_cross(const QPoint & u1, const QPoint & u2, const QPoint & v1, const QPoint &v2, QPoint &p){
    int x, y;
    x = u1.x();
    y = u1.y();
    double a= (double)(u1.x()-u2.x())*(v1.y()-v2.y())-(double)(u1.y()-u2.y())*(v1.x()-v2.x());
    double t=0;
    if (a!=0)
        t=((double)(u1.x()-v1.x())*(v1.y()-v2.y())-(double)(u1.y()-v1.y())*(v1.x()-v2.x())) /a;
    x+=(u2.x()-u1.x())*t;
    y+=(u2.y()-u1.y())*t;
    p.setX(x);
    p.setY(y);
    return (a!=0);
}

bool online(const QLine &l, const QPoint &p)
{
    double a= (double)(l.x1()-p.x())*(l.y2()-p.y())-(double)(l.y1()-p.y())*(l.x2()-p.x());
    return (a!=0);
}

#define SGN(x) (((x)>0) ? 1 : (((x)==0) ? 0 : -1))
int intersect(QLine l1, QLine l2, QPoint &point)
{
    Q_ASSERT(l1.x1()!=l1.x2() || l1.y1()!=l1.y2());
    Q_ASSERT(l2.x1()!=l2.x2() || l2.y1()!=l2.y2());

    bool parallel = (!get_cross(l1.p1(), l1.p2(), l2.p1(), l2.p2(), point));

    int x11 = SGN(l1.x1() -l2.x1());
    int x12 = SGN(l1.x1() -l2.x2());
    int x21 = SGN(l1.x2() -l2.x1());
    int x22 = SGN(l1.x2() -l2.x2());
    bool same_x = (x11==x12 && x21==x22 && x12==x21);
    if  (same_x && x11!=0)
        return parallel ? PARALLEL: NOINTERSECTION;

    int y11 = SGN(l1.y1() -l2.y1());
    int y12 = SGN(l1.y1() -l2.y2());
    int y21 = SGN(l1.y2() -l2.y1());
    int y22 = SGN(l1.y2() -l2.y2());
    bool same_y = (y11==y12 && y21==y22 && y12==y21);
    if (same_y && y11!=0)
        return parallel ? PARALLEL: NOINTERSECTION;

    int x01 = SGN(point.x() -l2.x1());
    int x02 = SGN(point.x() -l2.x2());
    int x10 = SGN(l1.x1() -point.x());
    int x20 = SGN(l1.x2() -point.x());
    int y01 = SGN(point.y() -l2.y1());
    int y02 = SGN(point.y() -l2.y2());
    int y10 = SGN(l1.y1() -point.y());
    int y20 = SGN(l1.y2() -point.y());

    if (l1.x1() ==l1.x2()) { //x11==x21, x12==x22
        Q_ASSERT(point.x()==l1.x1());
        if (x11 ==x12) { //l1.x1()==l1.x2()==l2.x1()==l2.x2()
            Q_ASSERT(x11==0); //4 points in one line
            if (y11!=y12 && y21!=y22) //l1.y1 & l1.y2 is inside l2
                return INCLUDE;
            if (y11==y12 && y21!=y22) //l1.y1 is outside l2, l1.y2 is inside l2
                return EXTEND1;
            if (y11!=y12 && y21==y22) //l1.y1 is inside l2, l1.y2 is outside l2
                return EXTEND2;

            Q_ASSERT(y12!=y21);
            return INCLUDEEDBY;
        } else {
            if (x11==0) { //l1.x1() ==l2.x1() = l1.x2(), 3 points in one line
                if (y11==y21) //l2.y1 is outside l1
                    return NOINTERSECTION;
                if (y11==0)  //l1.y1()==l2.y1()
                    return LEXTEND1;
                if (y21==0)  //l1.y2()==l2.y1()
                    return LEXTEND2;
                return BOUNDINTERSECTION1;
            }
            if (x12==0) { //l1.x1()==l2.x2()=l1.x2()
                if (y12==y22) //l2.y2 is outside l1
                    return NOINTERSECTION;
                if (y12==0) //l1.y1()==l2.y2()
                    return LEXTEND1;
                if (y22==0) //l1.y2()==l2.y2()
                    return LEXTEND2;
                return BOUNDINTERSECTION2;
            }
            //x11!=x12 && x11!=0 && x12!=0
            if (y10==y20)
                return NOINTERSECTION;
            if (y10==0)
                return TEXTEND1;
            if (y20==0)
                return TEXTEND2;
            return CENTERINTERSECTION;
        }
    }

    if (l1.y1() ==l1.y2()) { //y11==y21, y12==y22
        Q_ASSERT(point.y()==l1.y1());
        if (y11==y12) { //l1.y1()==l1.y2()==l2.y1()==l2.y2()
            Q_ASSERT(y11==0); //4 points in one line
            if (x11!=x12 && x21!=x22) //l1.x1 & l1.x2 is inside l2
                return INCLUDE;
            if (x11==x12 && x21!=x22) //l1.x1 is outside l2, l1.x2 is inside l2
                return EXTEND1;
            if (x11!=x12 && x21==x22) //l1.x1 is inside l2, l1.x2 is outside l2
                return EXTEND2;
             //l1.x1 & l1.x2 is outside l2
            Q_ASSERT(x12!=x21);
            return INCLUDEEDBY;
        } else {
            if (y11==0) { //l1.y1() ==l2.y1() = l1.y2(), 3 points in one line
                if (x11==x21) //l2.x1 is outside l1
                    return NOINTERSECTION;
                if (x11==0) //l1.x1()==l2.x1()
                    return LEXTEND1;
                if (x21==0) //l1.x2()==l2.x1()
                    return LEXTEND2;
                return BOUNDINTERSECTION1;
            }
            if (y12==0) { //l1.y1() ==l2.y2() = l1.y2(), 3 points in one line
                if (x12==x22) //l2.x2 is outside l1
                    return NOINTERSECTION;
                if (x12==0) //l1.x1()==l2.x2()
                    return LEXTEND1;
                if (x22==0) //l1.x2()==l2.x2()
                    return LEXTEND2;
                return BOUNDINTERSECTION2;
            }            
            //y11!=y12 && y11!=0 && y12!=0
            if (x10==x20) //cross point is outside l1
                return NOINTERSECTION;
            if (x10==0)
                return TEXTEND1;
            if (x20==0)
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

    if (!parallel && (x10==x20 && y10==y20)) //cross point is outside l1
        return NOINTERSECTION;

    if (online(l1, l2.p1())) {
        if (parallel) {//4 points in one line
            Q_ASSERT(y11==0); //4 points in one line
            if (x11!=x12 && x21!=x22) //l1.x1 & l1.x2 is inside l2
                return INCLUDE;
            if (x11==x12 && x21!=x22) //l1.x1 is outside l2, l1.x2 is inside l2
                return EXTEND1;
            if (x11!=x12 && x21==x22) //l1.x1 is inside l2, l1.x2 is outside l2
                return EXTEND2;
             //l1.x1 & l1.x2 is outside l2
            Q_ASSERT(x12!=x21);
            return INCLUDEEDBY;
        } else {//3 points in one line
            /*
            if (x11==x21) //l2.x1 is outside l1
                return NOINTERSECTION;*/
            Q_ASSERT(x11!=x21); //because x11==x10, x21==x20
            if (x11==0) //l1.x1()==l2.x1()
                return LEXTEND1;
            if (x21==0) //l1.x2()==l2.x1()
                return LEXTEND2;
            return BOUNDINTERSECTION1;
        }
    }

    if (online(l1, l2.p2())) { //3 points in one line
        Q_ASSERT(!parallel);
        /*if (x12==x22) //l2.x2 is outside l1
            return NOINTERSECTION;*/
        Q_ASSERT(x12!=x22); //because x12==x10, x22==x20
        if (x12==0) //l1.x1()==l2.x2()
            return LEXTEND1;
        if (x22==0) //l1.x2()==l2.x2()
            return LEXTEND2;
        return BOUNDINTERSECTION2;
    }

    if (parallel)
        return PARALLEL;
    //cross point is inside l1, and l1 is not 0 90 line
    // !parallel && x10!=x20 && y10!=y20
    if (x01==x02 && y01==y02)
        return NOINTERSECTION;
    if (online(l2, l1.p1()))
        return TEXTEND1;
    if (online(l2, l1.p2()))
        return TEXTEND2;
    return CENTERINTERSECTION;
}

class DBWorkArea {
public:
    int add_wire(unsigned x0, unsigned y0, unsigned char l0, unsigned x1, unsigned y1, vector<PointPatch> & patch);
    int add_via(unsigned x0, unsigned y0, unsigned char l0, unsigned char l1, vector<PointPatch> & patch);
    int add_inst(unsigned x0, unsigned y0, unsigned short template_dir, vector<PointPatch> & patch);
    int add_connect(unsigned x0, unsigned y0, unsigned char l0, DBID id1,
                     unsigned x1, unsigned y1, unsigned short type, unsigned short port1, DBID id2, vector<PointPatch> & patch);
    int add_name(unsigned x0, unsigned y0, unsigned short type, DBID id, const char * name, vector<PointPatch> & patch);
    int add_parameter(unsigned x0, unsigned y0, unsigned short type, DBID id,
                      const char * para_name, const char * para_data, vector<PointPatch> & patch);    
    int del_wire(unsigned x0, unsigned y0, unsigned char l0, unsigned x1, unsigned y1, vector<PointPatch> & patch);
    int del_via(unsigned x0, unsigned y0, DBID id, vector<PointPatch> & patch);
    int del_inst(unsigned x0, unsigned y0, unsigned short template_dir, DBID &id, vector<PointPatch> & patch);
    int del_connect(unsigned x0, unsigned y0, unsigned char l0, DBID id1,
                    unsigned x1, unsigned y1, unsigned short type, unsigned short port1, DBID id2, vector<PointPatch> & patch);
    int del_name(unsigned x0, unsigned y0, unsigned short type, DBID id, vector<PointPatch> & patch);
    int del_parameter(unsigned x0, unsigned y0, unsigned short type, DBID id,
                      const char * para_name, vector<PointPatch> & patch);
    void get_point_set(unsigned area, unsigned get_method, const set <DBID> & exclude_set);
};
#endif // ELEMENT_DB_H
