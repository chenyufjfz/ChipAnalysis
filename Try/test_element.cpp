#include "element_db.h"
#include <iostream>
#include <map>
#include <set>
#define E(expr) CHECK((rc = (expr)) == MDB_SUCCESS, #expr)
#define RES(err, expr) ((rc = expr) == (err) || (CHECK(!rc, #expr), 0))
#define CHECK(test, msg) ((test) ? (void)0 : ((void)fprintf(stderr, \
    "%s:%d: %s: %d, %s\n", __FILE__, __LINE__, msg, rc, mdb_strerror(rc)), abort()))

int test_element0()
{
	unsigned x0 = 0x08000000, y0 = 0x078000000;
	MemVWPoint vwp(x0, y0, 0, 0, NULL);
	vector <PairPoint> cmp_ppset;
	vwp.add_inst_wire(x0 + 10, y0 + 1000, 33);
	vwp.get_layer_wire(0, cmp_ppset);
	if (cmp_ppset.empty()) {
		printf("add inst wire fail");
		return -1;
	}		
	if (cmp_ppset[0].ip.x != x0 + 10 || cmp_ppset[0].ip.y != y0 + 1000 || cmp_ppset[0].ip.port != 33) {
		printf("add non_inst wire fail");
		return -1;
	}
	vwp.add_inst_wire(x0 + 1000, y0 + 10, 233);
	if (vwp.delete_layer_wire(INST_LAYER, x0 + 10, y0 + 1000, 33) < 0) {
		printf("del inst wire fail");
		return -1;
	}		
	vwp.get_layer_wire(0, cmp_ppset);
	if (cmp_ppset.size()!=1) {
		printf("del/add inst wire fail");
		return -1;
	}
	if (cmp_ppset[0].ip.x != x0 + 1000 || cmp_ppset[0].ip.y != y0 + 10 || cmp_ppset[0].ip.port != 233) {
		printf("del/add non_inst wire xy fail");
		return -1;
	}
	vwp.add_noninst_wire(1, x0 + 1000, y0);
	vwp.add_noninst_wire(1, x0 + 1000, y0 + 1000);
	vwp.get_layer_wire(1, cmp_ppset);
	if (cmp_ppset.size() != 2) {
		printf("del/add non_inst wire fail");
		return -1;
	}
	if ((cmp_ppset[0].wp.x != x0 + 1000 && cmp_ppset[0].wp.y != y0 + 1000) ||
		(cmp_ppset[1].wp.x != x0 + 1000 && cmp_ppset[1].wp.y != y0)) {
		printf("del/add non_inst wire xy fail");
		return -1;
	}
	printf("test_element0 success!\n");
	return 0;
}

int test_element1()
{
	unsigned x0 = 0x08000000, y0 = 0x07800000;
	MemVWPoint vwp(x0, y0, 0, 0, NULL);
	vector <PairPoint> pp_set[32];

	srand(1);

	for (unsigned test_num = 0; test_num < 100000; test_num++) {
		unsigned op = rand() & 0x1ff;
		PairPoint pp;
		if (op < 192) { //add wire
			unsigned x = x0, y = y0;
			unsigned char layer = INST_LAYER;
			while (layer == INST_LAYER || layer == 31)
				layer = rand() & 0x0f;
			if (pp_set[layer].size() > 6)
				continue;
			if (op < 128) { //add specific angle non_inst
				unsigned dir = op & 7;
				unsigned scale = (op & 0x18) >> 3;
				unsigned d = (rand() << 12) ^ rand();				
				switch (scale) {
				case 0:
					d = d & 0xff;
					break;
				case 1:
					d = d & 0xffff;
					break;
				case 2:
					d = d & 0xffffff;
					break;
				}
				if (d == 0)
					d = 1;
				switch (dir) {
				case 0:
					y -= d;
					break;
				case 1:
					y -= d;
					x += d;
					break;
				case 2:
					x += d;
					break;
				case 3:
					x += d;
					y += d;
					break;
				case 4:
					y += d;
					break;
				case 5:
					x -= d;
					y += d;
					break;
				case 6:
					x -= d;
					break;
				case 7:
					x -= d;
					y -= d;
					break;
				}								
				if (vwp.add_noninst_wire(layer, x, y)<0) {
					printf("can't add new wire for sepeific angle, check error");
					return -1;
				}
				pp.wp.x = x;
				pp.wp.y = y;
				pp_set[layer].push_back(pp);
			} else
				if (op < 160) { //add any angle non_inst
					int dir = op & 3;
					unsigned scalex = (op & 4) >> 2;
					unsigned scaley = (op & 8) >> 3;
					unsigned dx = (rand() << 12) ^ rand();
					unsigned dy = (rand() << 12) ^ rand();
					if (scalex)
						dx = dx & 0xffff;
					if (scaley)
						dy = dy & 0xffff;
					if (dx == 0 && dy == 0)
						dx = 1;
					switch (dir) {
					case 0:
						x -= dx;
						y -= dy;
						break;
					case 1:
						x += dx;
						y -= dy;
						break;
					case 2:
						x += dx;
						y += dy;
						break;
					case 3:
						x -= dx;
						y += dy;
						break;
					}
					
					if (vwp.add_noninst_wire(layer, x, y) < 0) {
						printf("can't add new wire for any angle, check error");
						return -1;
					}
					pp.wp.x = x;
					pp.wp.y = y;
					pp_set[layer].push_back(pp);
				}
				else { //add inst wire
					if (pp_set[INST_LAYER].size() > 6)
						continue;
					int dir = op & 3;
					int scale = (op & 4)>>2;
					unsigned short port = rand();
					unsigned dx = rand() & 0x7fff;
					unsigned dy = rand() & 0x7fff;
					if (dx == 0 && dy == 0)
						dx = 1;
					if (scale)
						port = port & 0x7f;
					switch (dir) {
					case 0:
						x -= dx;
						y -= dy;
						break;
					case 1:
						x += dx;
						y -= dy;
						break;
					case 2:
						x += dx;
						y += dy;
						break;
					case 3:
						x -= dx;
						y += dy;
						break;
					}
	
					if (vwp.add_inst_wire(x, y, port) < 0) {
						printf("can't add new inst wire");
						return -1;
					}

					pp.ip.x = x;
					pp.ip.y = y;
					pp.ip.port = port;
					pp_set[INST_LAYER].push_back(pp);
				}
			
		} 
		if (op < 256) {
			vector<unsigned char> layers;

			vwp.get_layers(layers);
			for (unsigned char i = 0; i < 32; i++) {
				if (find(layers.begin(), layers.end(), i) == layers.end()) {
					if (!pp_set[i].empty()) {
						printf("can't find exist layers, check error");
						return -1;
					}
				}
				else {
					if (pp_set[i].empty()) {
						printf("found not-exist layers, check error");
						return -1;
					}
				}
			}
			vector <PairPoint> cmp_ppset;
			for (unsigned char i = 0; i < 32; i++) {
				vwp.get_layer_wire(i, cmp_ppset);
				if (cmp_ppset.size() != pp_set[i].size()) {
					printf("layer size not equal s1=%d, s2=%d check error", cmp_ppset.size(), pp_set[i].size());
					return -1;
				}
				bool found;
				for (int j = 0; j < cmp_ppset.size(); j++) {
					found = false;
					for (int k = 0; k < pp_set[i].size(); k++) {
						if (i == INST_LAYER) {
							if (cmp_ppset[j].ip.port == pp_set[i][k].ip.port &&
								cmp_ppset[j].ip.x == pp_set[i][k].ip.x &&
								cmp_ppset[j].ip.y == pp_set[i][k].ip.y)
								found = true;
						}
						else {
							if (cmp_ppset[j].wp.x == pp_set[i][k].wp.x &&
								cmp_ppset[j].wp.y == pp_set[i][k].wp.y)
								found = true;
						}
						if (found)
							break;
					}
						
					if (!found) {
						printf("can't find wire, layer=%d, size=%d, check error", i, cmp_ppset.size());
						return -1;
					}
				}
			}

			continue;
		}
		if (op <= 480) {
			int del_num = op - 256;
			unsigned char layer;
			for (layer = 0; layer < 32; layer++) {
				if (del_num >= pp_set[layer].size())
					del_num -= (int) pp_set[layer].size();
				else
					break;
			}
			if (layer == 32)
				continue;
			int rc;

			if (layer != INST_LAYER)
				rc = vwp.delete_layer_wire(layer, pp_set[layer][del_num].wp.x, pp_set[layer][del_num].wp.y);
			else
				rc = vwp.delete_layer_wire(layer, pp_set[layer][del_num].ip.x, pp_set[layer][del_num].ip.y,
				pp_set[layer][del_num].ip.port);

			if (rc != 0) {
				printf("can't delete wire, layer=%d, size=%d, check error", layer, pp_set[layer].size());
				return -1;
			}
			pp_set[layer].erase(pp_set[layer].begin() + del_num);
		}
		else {
			unsigned del_layer = op - 481;			
			while (!pp_set[del_layer].empty()) {
				unsigned char del_num = (unsigned char) (rand() % pp_set[del_layer].size());
				int rc;

				if (del_layer != INST_LAYER)
					rc = vwp.delete_layer_wire(del_layer, pp_set[del_layer][del_num].wp.x, pp_set[del_layer][del_num].wp.y);
				else
					rc = vwp.delete_layer_wire(del_layer, pp_set[del_layer][del_num].ip.x, pp_set[del_layer][del_num].ip.y,
					pp_set[del_layer][del_num].ip.port);
				pp_set[del_layer].erase(pp_set[del_layer].begin() + del_num);
			}
		}
	}
	printf("test_element1 success!\n");
	return 0;
}

int test_element_db0()
{
	unsigned area = 0x400400;
	MDB_val key, data;
	DBAreaWireVia db_area_wire;
	MDB_env *env;
	MDB_txn *txn;
	MDB_dbi dbi;
	MDB_stat mst;
	int rc;

	mdb_init();
	srand(1);
	remove("./db");
	remove("./db-lock");
	E(mdb_env_create(&env));
	E(mdb_env_set_maxreaders(env, 2));
	E(mdb_env_set_mapsize(env, 0x800000000));
	E(mdb_env_set_maxdbs(env, 64));
	E(mdb_env_open(env, "./db", MDB_NOMETASYNC | MDB_NOSUBDIR, 0664));

	E(mdb_txn_begin(env, NULL, 0, &txn));
	if (sizeof(DBID) == 4)
		E(mdb_dbi_open(txn, NULL, MDB_INTEGERKEY, &dbi));
	else
		E(mdb_dbi_open(txn, NULL, 0, &dbi));
	memset(&db_area_wire, 0, sizeof(db_area_wire));

	DBID id(area, META_TYPE, AREA_WIREVIA_INFO);
	key.mv_size = sizeof(id);
	key.mv_data = &id;
	data.mv_size = sizeof(db_area_wire);
	data.mv_data = &db_area_wire;
	rc = mdb_put(txn, dbi, &key, &data, 0);
	if (rc != 0) {
		qCritical(mdb_strerror(rc));
		return -1;
	}
	E(mdb_txn_commit(txn));
	
	//do add test
	E(mdb_txn_begin(env, NULL, 0, &txn));
	MemAreaWireVia wp_set(txn, dbi, area);
	for (int i = 0; i < 16; i++) {
		unsigned part_y = i / 4;
		unsigned part_x = i % 4;
		DBID id(area, WIRE_TYPE, part_x, part_y, 0);
		unsigned x0 = id.localx2grid(0);
		unsigned y0 = id.localy2grid(0);
		MemVWPoint vwp(x0, y0, 0, 0, NULL);
		vwp.add_noninst_wire(1, x0 + 0x10000, y0 + 0x10000);
		vwp.add_inst_wire(x0 + 0x5000, y0 + 0x5000, 33);
		E(wp_set.add_point(vwp));
	}
	wp_set.close(true);
	E(mdb_txn_commit(txn));

	//do check, del and add test
	E(mdb_txn_begin(env, NULL, 0, &txn));
	wp_set.renew(txn, dbi, area);
	vector<MemVWPoint> vwps;
	wp_set.get_points_all(vwps);
	if (vwps.size() != 16) {
		printf("get points size %d check error", vwps.size());
		return -1;
	}
		
	for (int i = 0; i < 16; i++) {
		unsigned part_y = i / 4;
		unsigned part_x = i % 4;
		DBID id(area, WIRE_TYPE, part_x, part_y, 0);
		unsigned x0 = id.localx2grid(0);
		unsigned y0 = id.localy2grid(0);
		vector<unsigned char> layers;
		if (vwps[i].y != y0 || vwps[i].x != x0) {
			printf("x, y not equal, check error");
			return -1;
		}

		vwps[i].get_layers(layers);
		if (layers.size() != 2 || layers[0] != INST_LAYER || layers[1] != 1) {
			printf("get layer wrong check error");
			return -1;
		}
		vector <PairPoint> cmp_ppset;
		vwps[i].get_layer_wire(INST_LAYER, cmp_ppset);
		if (cmp_ppset[0].ip.x != x0 + 0x5000 || cmp_ppset[0].ip.y != y0 + 0x5000 || cmp_ppset[0].ip.port != 33) {
			printf("part %d get layer 0 wire wrong, check error", i);
			return -1;
		}
		vwps[i].get_layer_wire(1, cmp_ppset);
		if (cmp_ppset[0].wp.x != x0 + 0x10000 || cmp_ppset[0].wp.y != y0 + 0x10000) {
			printf("part %d get layer 1 wire wrong, check error", i);
			return -1;
		}
		wp_set.del_point(vwps[i]);
		vwps[i].delete_layer_wire(INST_LAYER, x0 + 0x5000, y0 + 0x5000, 33);
		vwps[i].delete_layer_wire(1, x0 + 0x10000, y0 + 0x10000);
		vwps[i].add_noninst_wire(2, x0 + 0x100, y0 + 0x100);
		wp_set.add_point(vwps[i]);
	}
	wp_set.close(true);
	E(mdb_txn_commit(txn));

	//do check, del and add test
	E(mdb_txn_begin(env, NULL, MDB_RDONLY, &txn));
	wp_set.renew(txn, dbi, area);
	wp_set.get_points_all(vwps);
	if (vwps.size() != 16) {
		printf("get points size %d check error", vwps.size());
		return -1;
	}

	for (int i = 0; i < 16; i++) {
		unsigned part_y = i / 4;
		unsigned part_x = i % 4;
		DBID id(area, WIRE_TYPE, part_x, part_y, 0);
		unsigned x0 = id.localx2grid(0);
		unsigned y0 = id.localy2grid(0);
		vector<unsigned char> layers;
		if (vwps[i].y != y0 || vwps[i].x != x0) {
			printf("x, y not equal, check error");
			return -1;
		}

		vwps[i].get_layers(layers);
		if (layers.size() != 1 || layers[0] != 2) {
			printf("get layer wrong check error");
			return -1;
		}
		vector <PairPoint> cmp_ppset;
		vwps[i].get_layer_wire(2, cmp_ppset);
		if (cmp_ppset[0].wp.x != x0 + 0x100 || cmp_ppset[0].wp.y != y0 + 0x100) {
			printf("part %d get layer 1 wire wrong, check error", i);
			return -1;
		}		
	}
	mdb_txn_abort(txn);

	E(mdb_env_stat(env, &mst));
	mdb_dbi_close(env, dbi);
	mdb_env_close(env);
	printf("bracpage=%d, depth=%d, entry=%d, leafpage=%d, ovflpage=%d, psize=%d\n", mst.ms_branch_pages, mst.ms_depth,
		mst.ms_entries, mst.ms_leaf_pages, mst.ms_overflow_pages, mst.ms_psize);
	printf("test_element_db0 success!\n");	
	return 0;
}

int modify_vwwire(MemVWPoint &vwp)
{
	for (int i = 0; i < 5; i++) {
		unsigned op = rand() & 0xff;
		PairPoint pp;
		if (op < 192) { //add wire
			unsigned x = vwp.x, y = vwp.y;
			unsigned char layer = INST_LAYER;
			while (layer == INST_LAYER || layer == 31)
				layer = rand() & 0x0f;
			if (op < 128) { //add specific angle non_inst
				unsigned dir = op & 7;
				unsigned scale = (op & 0x18) >> 3;
				unsigned d = (rand() << 12) ^ rand();
				switch (scale) {
				case 0:
					d = d & 0xff;
					break;
				case 1:
					d = d & 0xffff;
					break;
				case 2:
					d = d & 0xffffff;
					break;
				}
				if (d == 0)
					d = 1;
				switch (dir) {
				case 0:
					y -= d;
					break;
				case 1:
					y -= d;
					x += d;
					break;
				case 2:
					x += d;
					break;
				case 3:
					x += d;
					y += d;
					break;
				case 4:
					y += d;
					break;
				case 5:
					x -= d;
					y += d;
					break;
				case 6:
					x -= d;
					break;
				case 7:
					x -= d;
					y -= d;
					break;
				}
				vwp.add_noninst_wire(layer, x, y);
				pp.wp.x = x;
				pp.wp.y = y;				
			}
			else
				if (op < 160) { //add any angle non_inst
					int dir = op & 3;
					unsigned scalex = (op & 4) >> 2;
					unsigned scaley = (op & 8) >> 3;
					unsigned dx = (rand() << 12) ^ rand();
					unsigned dy = (rand() << 12) ^ rand();
					if (scalex)
						dx = dx & 0xffff;
					if (scaley)
						dy = dy & 0xffff;
					if (dx == 0 && dy == 0)
						dx = 1;
					switch (dir) {
					case 0:
						x -= dx;
						y -= dy;
						break;
					case 1:
						x += dx;
						y -= dy;
						break;
					case 2:
						x += dx;
						y += dy;
						break;
					case 3:
						x -= dx;
						y += dy;
						break;
					}
					vwp.add_noninst_wire(layer, x, y);
				}
				else { //add inst wire					
					int dir = op & 3;
					int scale = (op & 4) >> 2;
					unsigned short port = rand();
					unsigned dx = rand() & 0x7fff;
					unsigned dy = rand() & 0x7fff;
					if (dx == 0 && dy == 0)
						dx = 1;
					if (scale)
						port = port & 0x7f;
					switch (dir) {
					case 0:
						x -= dx;
						y -= dy;
						break;
					case 1:
						x += dx;
						y -= dy;
						break;
					case 2:
						x += dx;
						y += dy;
						break;
					case 3:
						x -= dx;
						y += dy;
						break;
					}
					vwp.add_inst_wire(x, y, port);					
				}
			continue;
		}
		
		unsigned del_layer = op & 0x1f;
		vector<unsigned char> layers;
		vector<PairPoint> delp;
		vwp.get_layers(layers);
		if (layers.size() > del_layer)
			del_layer = layers[del_layer];
		else
			continue;
		vwp.get_layer_wire(del_layer, delp);
		for (int i = 0; i < delp.size(); i++) {
			int rc;
			if (del_layer != INST_LAYER)
				rc = vwp.delete_layer_wire(del_layer, delp[i].wp.x, delp[i].wp.y);
			else
				rc = vwp.delete_layer_wire(del_layer, delp[i].ip.x, delp[i].ip.y, delp[i].ip.port);
			if (rc != 0) {
				printf("can't delete wire, layer=%d, size=%d, check error", del_layer, delp.size());
				return -1;
			}
		}				
	}
	return 0;
}

int test_element_db1()
{
	unsigned area = 0x400400;
	MDB_val key, data;
	DBAreaWireVia db_area_wire;
	MDB_env *env;
	MDB_txn *txn;
	MDB_dbi dbi;
	MDB_stat mst;
	vector<MemVWPoint *> clone;
	vector<int> del_num;
	map<MemPoint *, int, MemPointCmp> map_set;
	map<MemPoint *, int, MemPointCmp>::iterator map_set_iter;
	int rc;

	mdb_init();
	srand(1);
	remove("./db");
	remove("./db-lock");
	E(mdb_env_create(&env));
	E(mdb_env_set_maxreaders(env, 2));
	E(mdb_env_set_mapsize(env, 0x800000000));
	E(mdb_env_set_maxdbs(env, 64));
	E(mdb_env_open(env, "./db", MDB_NOMETASYNC | MDB_NOSUBDIR, 0664));

	E(mdb_txn_begin(env, NULL, 0, &txn));
	if (sizeof(DBID)==4)
		E(mdb_dbi_open(txn, NULL, MDB_INTEGERKEY, &dbi));
	else
		E(mdb_dbi_open(txn, NULL, 0, &dbi));
	memset(&db_area_wire, 0, sizeof(db_area_wire));

	DBID id(area, META_TYPE, AREA_WIREVIA_INFO);
	key.mv_size = sizeof(id);
	key.mv_data = &id;
	data.mv_size = sizeof(db_area_wire);
	data.mv_data = &db_area_wire;
	rc = mdb_put(txn, dbi, &key, &data, 0);
	if (rc != 0) {
		qCritical(mdb_strerror(rc));
		return -1;
	}
	E(mdb_txn_commit(txn));

	MemAreaWireVia wp_set;

	for (unsigned test_num = 0; test_num < 3000; test_num++) {
		E(mdb_txn_begin(env, NULL, 0, &txn));
		wp_set.renew(txn, dbi, area);
		for (unsigned op_num = 0; op_num < 10; op_num++) {
			unsigned op = rand() % 4096;
			if (op < 1536) {//delete existing point
				unsigned del = op;				
				if (clone.size() <= del || clone[del] == NULL)
					continue;
				MemVWPoint *vwp = clone[del];
				if (wp_set.del_point(*vwp) < 0) {
					printf("delete element error");
					return -1;
				}
				map_set.erase(vwp);
				delete vwp;
				del_num.push_back(del);
				clone[del] = NULL;				
				continue;
			}
			if (op < 3072) { //add new point
				unsigned char part = (rand() % 16) << 2;
				SET_FIELD(part, TYPE, WIRE_TYPE);
				id.set_id_part(part);
				unsigned x0 = id.localx2grid(rand() & 0x7fff);
				unsigned y0 = id.localy2grid(rand() & 0x7fff);
				MemVWPoint *vwp = new MemVWPoint(x0, y0, 0, part, NULL);
				if (modify_vwwire(*vwp) != 0) {
					printf("add new Point error");
					return -1;
				}
				int rc;
				if ((rc = wp_set.add_point(*vwp)) == 0) {
					if (del_num.empty()) {
						clone.push_back(vwp);
						map_set[vwp] = (int) clone.size() - 1;
					}
					else {
						clone[del_num.back()] = vwp;
						map_set[vwp] = del_num.back();
						del_num.pop_back();
					}
				}
				else {
					printf("DB add new point error %d", rc);
				}				
				continue;
			}
			//modify point
			unsigned del = op - 3072;
			if (clone.size() <= del || clone[del] == NULL)
				continue;
			MemVWPoint *vwp = clone[del];
			if (wp_set.del_point(*vwp) < 0) {
				printf("replace element delete error");
				return -1;
			}
			map_set.erase(vwp);
			modify_vwwire(*vwp);
			wp_set.add_point(*vwp);
			map_set[vwp] = del;
		}
		wp_set.close(true);
		E(mdb_txn_commit(txn));

		E(mdb_txn_begin(env, NULL, MDB_RDONLY, &txn));
		wp_set.renew(txn, dbi, area);
		vector<MemVWPoint> db_vwp_set;
		rc = wp_set.get_points_all(db_vwp_set);
		if (rc != 0)
			return -1;
		for (int i = 0; i < db_vwp_set.size(); i++) {
			map_set_iter = map_set.find(&db_vwp_set[i]);
			if (map_set_iter == map_set.end()) {
				printf("found non-exist point, check error");
				return -1;
			}
			if (map_set_iter->second > clone.size() || clone[map_set_iter->second]==NULL) {
				printf("test code internal error");
				return -1;
			}
			if (!(*clone[map_set_iter->second] == db_vwp_set[i])) {
				printf("point attach is wrong, check error");
				return -1;
			}				
		}
		mdb_txn_abort(txn);
	}	
	
	for (int i = 0; i < clone.size(); i++)
		if (clone[i] != NULL)
			delete clone[i];
	
	E(mdb_env_stat(env, &mst));
	mdb_dbi_close(env, dbi);
	mdb_env_close(env);
	printf("csize=%d, dsize=%d, bracpage=%d, depth=%d, entry=%d, leafpage=%d, ovflpage=%d, psize=%d\n", clone.size(), del_num.size(),
		mst.ms_branch_pages, mst.ms_depth, mst.ms_entries, mst.ms_leaf_pages, mst.ms_overflow_pages, mst.ms_psize);
	printf("test_element_db1 success!\n");
	return 0;
}

void rand_xy_outarea(unsigned &x, unsigned &y, unsigned area)
{
	x = (rand() << 13) ^ rand();
	do {
		y = (rand() << 13) ^ rand();
	} while (DBID::xy2id_area(x, y) == area);
}
int test_element_db2()
{
	unsigned area = 0x400400;
	MDB_env *env;
	MDB_txn *txn;
	MDB_dbi dbi;
	MDB_stat mst;
	MDB_val key, data;
	DBAreaWireVia db_area_wire;
	int rc;
	vector<DBExtWire *> clone;
	vector<int> del_num;
	map<DBExtWire *, int, DBExtWireCmp> map_set;
	map<DBExtWire *, int, DBExtWireCmp>::iterator map_set_iter;

	mdb_init();
	srand(1);
	remove("./db");
	remove("./db-lock");
	E(mdb_env_create(&env));
	E(mdb_env_set_maxreaders(env, 2));
	E(mdb_env_set_mapsize(env, 0x800000000));
	E(mdb_env_set_maxdbs(env, 64));
	E(mdb_env_open(env, "./db", MDB_NOMETASYNC | MDB_NOSUBDIR, 0664));

	E(mdb_txn_begin(env, NULL, 0, &txn));
	if (sizeof(DBID) == 4)
		E(mdb_dbi_open(txn, NULL, MDB_INTEGERKEY, &dbi));
	else
		E(mdb_dbi_open(txn, NULL, 0, &dbi));
	memset(&db_area_wire, 0, sizeof(db_area_wire));

	DBID id(area, META_TYPE, AREA_WIREVIA_INFO);
	key.mv_size = sizeof(id);
	key.mv_data = &id;
	data.mv_size = sizeof(db_area_wire);
	data.mv_data = &db_area_wire;
	rc = mdb_put(txn, dbi, &key, &data, 0);
	if (rc != 0) {
		qCritical(mdb_strerror(rc));
		return -1;
	}
	E(mdb_txn_commit(txn));

	MemAreaWireVia wp_set;
	for (unsigned test_num = 0; test_num < 2000; test_num++) {
		E(mdb_txn_begin(env, NULL, 0, &txn));
		wp_set.renew(txn, dbi, area);
		for (unsigned op_num = 0; op_num < 10; op_num++) {
			unsigned op = rand() % 256;
			if (op < 128) { //delete existing ext wire
				unsigned del = op;
				if (clone.size() <= del || clone[del] == NULL)
					continue;
				if ((rc = wp_set.del_ext_wire(*clone[del])) < 0) {
					printf("delete extern wire error %d", rc);
					return -1;
				}
				map_set.erase(clone[del]);
				delete clone[del];
				del_num.push_back(del);
				clone[del] = NULL;
				continue;
			}
			//TODO add new wire
			unsigned x0, y0, x1, y1;
			unsigned char layer;			
			rand_xy_outarea(x0, y0, area);
			rand_xy_outarea(x1, y1, area);
			layer = (rand() & 0xf) + 1;

			DBExtWire * dbw = new DBExtWire(x0, y0, x1, y1, layer);			
			if ((rc = wp_set.add_ext_wire(*dbw) == 0)) {
				if (del_num.empty()) {
					clone.push_back(dbw);
					map_set[dbw] = (int) clone.size() - 1;
				}
				else {
					clone[del_num.back()] = dbw;
					map_set[dbw] = del_num.back();
					del_num.pop_back();
				}
			}
			else {
				printf("DB add extern wire error %d", rc);
				return -1;
			}			
		}
		wp_set.close(true);
		E(mdb_txn_commit(txn));

		E(mdb_txn_begin(env, NULL, MDB_RDONLY, &txn));
		wp_set.renew(txn, dbi, area);
		vector<DBExtWire> ext_wire_set;
		rc = wp_set.get_ext_wire_all(ext_wire_set);
		if (rc != 0)
			return -1;
		for (int i = 0; i < ext_wire_set.size(); i++) {
			map_set_iter = map_set.find(&ext_wire_set[i]);
			if (map_set_iter == map_set.end()) {
				printf("found non-exist ext wire, check error");
				return -1;
			}
			if (map_set_iter->second > clone.size() || clone[map_set_iter->second] == NULL) {
				printf("test code internal error");
				return -1;
			}
			if (!(*clone[map_set_iter->second] == ext_wire_set[i])) {
				printf("extern wire not match, test code internal error");
				return -1;
			}
		}
		mdb_txn_abort(txn);
	}
	for (int i = 0; i < clone.size(); i++)
		if (clone[i] != NULL)
			delete clone[i];
	E(mdb_env_stat(env, &mst));
	mdb_dbi_close(env, dbi);
	mdb_env_close(env);
	printf("csize=%d, dsize=%d, bracpage=%d, depth=%d, entry=%d, leafpage=%d, ovflpage=%d, psize=%d\n", clone.size(), del_num.size(),
		mst.ms_branch_pages, mst.ms_depth, mst.ms_entries, mst.ms_leaf_pages, mst.ms_overflow_pages, mst.ms_psize);
	printf("test_element_db2 success!\n");
	return 0;
}

int test_cross()
{
	QLine l1(100, 100, 300, 100);
	QLine l2(150, 100, 150, 300);
	QLine l3(100, 200, 300, 200);
	QLine l4(100, 100, 100, 300);
	QLine l5(300, 100, 500, 100);
	QLine l6(400, 100, 600, 100);
	QLine l7(0, 0, 300, 300);
	QLine l8(0, 100, 99, 100);
	QLine l9(100, 100, 200, 200);
	QLine l10(150, 0, 150, 301);
	QLine l12(300, 300, 300, 100);
	QLine l13(300, 300, 600, 0);
	QLine l14(0, 600, 600, 0);
	QLine l15(100, 99, 200, 99);
	QLine l16(0, 599, 599, 0);
	QLine l17(0, 199, 99, 100);
	QLine l18(100, 99, 199, 0);
	QLine l19(0, 50, 200, 150);
	QPoint p;
	if (intersect(l1, l2, p) != BOUNDINTERSECTION1)
		return -1;
	printf("x=%d, y=%d\n", p.x(), p.y());
	if (intersect(l2, l1, p) != TEXTEND1)
		return -1;
	printf("x=%d, y=%d\n", p.x(), p.y());
	if (intersect(l2, l3, p) != CENTERINTERSECTION)
		return -1;
	printf("x=%d, y=%d\n", p.x(), p.y());
	if (intersect(l1, l3, p) != PARALLEL)
		return -1;
	if (intersect(l1, l4, p) != LEXTEND1)
		return -1;
	if (intersect(l4, l1, p) != LEXTEND1)
		return -1;
	printf("x=%d, y=%d\n", p.x(), p.y());
	if (intersect(l3, l4, p) != TEXTEND1)
		return -1;
	printf("x=%d, y=%d\n", p.x(), p.y());
	if (intersect(l1, l5, p) != EXTEND1)
		return -1;
	if (intersect(l1, l6, p) != PARALLEL)
		return -1;
	if (intersect(l5, l6, p) != EXTEND1)
		return -1;
	if (intersect(l6, l5, p) != EXTEND2)
		return -1;
	if (intersect(l6, l7, p) != NOINTERSECTION)
		return -1;
	if (intersect(l1, l7, p) != TEXTEND1)
		return -1;
	if (intersect(l7, l1, p) != BOUNDINTERSECTION1)
		return -1;
	printf("x=%d, y=%d\n", p.x(), p.y());
	if (intersect(l2, l7, p) != CENTERINTERSECTION)
		return -1;
	if (intersect(l1, l8, p) != PARALLEL)
		return -1;
	if (intersect(l7, l8, p) != NOINTERSECTION)
		return -1;
	if (intersect(l7, l9, p) != INCLUDEEDBY)
		return -1;
	if (intersect(l9, l7, p) != INCLUDE)
		return -1;
	if (intersect(l10, l2, p) != INCLUDEEDBY)
		return -1;
	if (intersect(l2, l10, p) != INCLUDE)
		return -1;
	if (intersect(l12, l1, p) != LEXTEND2)
		return -1;
	if (intersect(l1, l12, p) != LEXTEND2)
		return -1;
	if (intersect(l3, l12, p) != TEXTEND2)
		return -1;
	printf("x=%d, y=%d\n", p.x(), p.y());
	if (intersect(l12, l3, p) != BOUNDINTERSECTION2)
		return -1;
	printf("x=%d, y=%d\n", p.x(), p.y());
	if (intersect(l7, l13, p) != LEXTEND2)
		return -1;
	if (intersect(l13, l7, p) != LEXTEND1)
		return -1;
	if (intersect(l7, l14, p) != TEXTEND2)
		return -1;
	if (intersect(l14, l7, p) != BOUNDINTERSECTION2)
		return -1;
	if (intersect(l14, l2, p) != NOINTERSECTION)
		return -1;
	if (intersect(l7, l15, p) != NOINTERSECTION)
		return -1;
	if (intersect(l14, l16, p) != PARALLEL)
		return -1;
	if (intersect(l7, l16, p) != CENTERINTERSECTION)
		return -1;
	if (intersect(l7, l17, p) != NOINTERSECTION)
		return -1;
	if (intersect(l18, l7, p) != NOINTERSECTION)
		return -1;
	if (intersect(l19, l17, p) != NOINTERSECTION)
		return -1;
	if (intersect(l18, l19, p) != NOINTERSECTION)
		return -1;
	printf("test cross success\n");
	return 0;
}

//if success, return 0
int apply_patch(set<MemVWPoint *, MemVWPointCmp> & point_set, vector<PointPatch*> & patch)
{
	set<MemVWPoint *, MemVWPointCmp>::iterator set_it;
	Q_ASSERT(patch.size() == 1);
	for (int i = 0; i < patch[0]->old_points.size(); i++) {
		MemVWPoint * old_point = dynamic_cast<MemVWPoint *> (patch[0]->old_points[i]);
		if (old_point == NULL)
			return -1;
		set_it = point_set.find(old_point);
		if (set_it == point_set.end())
			return -2;
		if (**set_it != *old_point)
			return -2;
		delete *set_it;
		point_set.erase(set_it);		
	}

	for (int i = 0; i < patch[0]->new_points.size(); i++) {
		MemVWPoint * new_point = dynamic_cast<MemVWPoint *> (patch[0]->new_points[i]);
		if (new_point == NULL)
			return -1;
		set_it = point_set.find(new_point);
		if (set_it != point_set.end())
			return -2;
		MemVWPoint * clone_point = new MemVWPoint(*new_point);
		point_set.insert(clone_point);
	}
	return 0;
}

void decode_wire(unsigned wire, unsigned & x0, unsigned & y0, unsigned & x1, unsigned & y1, unsigned char & layer, int shift)
{
	layer = wire & 0xf;
	x0 = (((wire & 0x00000070) >> 4) << 14) | (((wire & 0x00000780) >> 4) << (14+shift));
	y0 = (((wire & 0x00003800) >> 11) << 14) | (((wire & 0x0003c000) >> 11) << (14+shift));
	x1 = (((wire & 0x001c0000) >> 18) << 14) | (((wire & 0x01e00000) >> 18) << (14+shift));
	y1 = (((wire & 0x0e000000) >> 25) << 14) | (((wire & 0xf0000000) >> 25) << (14+shift));
	return;
}

unsigned encode_wire(unsigned x0, unsigned y0, unsigned x1, unsigned y1, unsigned layer, int shift)
{
	unsigned area_x0, local_x0, area_y0, local_y0, area_x1, local_x1, area_y1, local_y1;
	local_x0 = (x0 >> 14) & 0x7;
	area_x0 = x0 >> (17 + shift);
	local_x1 = (x1 >> 14) & 0x7;
	area_x1 = x1 >> (17 + shift);
	local_y0 = (y0 >> 14) & 0x7;
	area_y0 = y0 >> (17 + shift);
	local_y1 = (y1 >> 14) & 0x7;
	area_y1 = y1 >> (17 + shift);
	return layer | (local_x0 << 4) | (area_x0 << 7) | (local_y0 << 11) | (area_y0 << 14) |
		(local_x1 << 18) | (area_x1 << 21) | (local_y1 << 25) | (area_y1 << 28);
}

unsigned char count_wire[128][128][4];
unsigned rand_wire(unsigned area_num, unsigned local_num, unsigned layer_num)
{
	unsigned area_x0, local_x0, area_y0, local_y0, area_x1, local_x1, area_y1, local_y1;
	unsigned char layer;
	int r;
	int ax, ay;
	do {
		r = rand();
		area_x0 = (r & 0xff) % area_num;
		local_x0 = (r >> 8) % local_num;
		r = rand();
		area_y0 = (r & 0xff) % area_num;
		local_y0 = (r >> 8) % local_num;
		layer = (rand() >> 5) % layer_num + 1;
		ax = (area_x0 << 3) + local_x0;
		ay = (area_y0 << 3) + local_y0;
	} while (count_wire[ay][ax][layer] > 5);
	do {
		do {
			r = rand();
			area_x1 = (r & 0xff) % area_num;
			local_x1 = (r >> 8) % local_num;
		} while (area_x1 == area_x0 && local_x1 == local_x0);
		do {
			r = rand();
			area_y1 = (r & 0xff) % area_num;
			local_y1 = (r >> 8) % local_num;
		} while (area_y1 == area_y0 && local_y1 == local_y0);		
		r = rand();	
		if (r & 0x100) {
			if (r & 0x200) {
				area_x1 = area_x0;
				local_x1 = local_x0;
			}
			else {
				area_y1 = area_y0;
				local_y1 = local_y0;
			}			
		} 
		ax = (area_x1 << 3) + local_x1;
		ay = (area_y1 << 3) + local_y1;
	} while (count_wire[ay][ax][layer] > 5);
	return layer | (local_x0 << 4) | (area_x0 << 7) | (local_y0 << 11) | (area_y0 << 14) |
		(local_x1 << 18) | (area_x1 << 21) | (local_y1 << 25) | (area_y1 << 28);
}

MemVWPoint * find_point(const set<MemVWPoint *, MemVWPointCmp> & point_set, MemVWPoint * p)
{
	set<MemVWPoint *, MemVWPointCmp>::iterator set_it;
	set_it = point_set.find(p);
	if (set_it == point_set.end())
		return NULL;
	else
		return *set_it;
}

int test_element_draw0()
{
	MDB_env *env;
	int rc;	
	set<unsigned int> wire_set;
	vector<unsigned int> clone_wire;
	set<MemVWPoint *, MemVWPointCmp> point_set;
	int shift = 0;
	unsigned area_num = 10;
	unsigned local_num = 4;
	unsigned layer_num = 1;
	MDB_stat mst;

	mdb_init();
	srand(2);
	memset(count_wire, 0, sizeof(count_wire));

	remove("./db");
	remove("./db-lock");
	E(mdb_env_create(&env));
	E(mdb_env_set_maxreaders(env, 2));
	E(mdb_env_set_mapsize(env, 0x800000000));
	E(mdb_env_set_maxdbs(env, 64));
	E(mdb_env_open(env, "./db", MDB_NOMETASYNC | MDB_NOSUBDIR, 0664));

	DBProject *prj = new DBProject(env, NULL);
	for (int test_num =300; test_num >=0; test_num--) {
		E(prj->new_write_txn());
		for (unsigned op_num = 0; op_num < 30; op_num++) {
			unsigned op = rand() & 0xfff;
			unsigned x0, y0, x1, y1;
			unsigned char layer;
			unsigned wire;
			int ax, ay;
			vector<PointPatch*> patch;
			if (test_num == 0) {
				if (clone_wire.size() == 0)
					break;
				op = (unsigned) clone_wire.size() - 1;
				op_num = 0;
			}
			if (op < 2048) {
				if (op >= clone_wire.size())
					continue;
				wire = clone_wire[op];
				decode_wire(wire, x0, y0, x1, y1, layer, shift);
				clone_wire[op] = clone_wire.back();
				clone_wire.pop_back();
				wire_set.erase(wire);
				MemVWPoint p0(x0, y0, layer), p1(x1, y1, layer);
				MemVWPoint * del_p0 = find_point(point_set, &p0);
				MemVWPoint * del_p1 = find_point(point_set, &p1);
				if (del_p0 == NULL || del_p1 == NULL) {
					printf("delete point not exist in point_set, check error");
					return -1;
				}
				ax = (wire >> 4) & 0x7f;
				ay = (wire >> 11) & 0x7f;
				if (count_wire[ay][ax][layer] == 0) {
					printf("Internal error");
					return -1;
				}
				count_wire[ay][ax][layer]--;
				ax = (wire >> 18) & 0x7f;
				ay = (wire >> 25) & 0x7f;
				if (count_wire[ay][ax][layer] == 0) {
					printf("Internal error");
					return -1;
				}
				count_wire[ay][ax][layer]--;
				E(prj->del_wire_nocheck(*del_p0, *del_p1, layer, patch));
				E(apply_patch(point_set, patch));
			}
			else {
				wire = rand_wire(area_num, local_num, layer_num);
				if (wire_set.find(wire) != wire_set.end())
					continue;
				decode_wire(wire, x0, y0, x1, y1, layer, shift);
				unsigned wire1 = encode_wire(x1, y1, x0, y0, layer, shift);
				if (wire_set.find(wire1) != wire_set.end())
					continue;
				ax = (wire >> 4) & 0x7f;
				ay = (wire >> 11) & 0x7f;
				count_wire[ay][ax][layer]++;
				ax = (wire >> 18) & 0x7f;
				ay = (wire >> 25) & 0x7f;
				count_wire[ay][ax][layer]++;
				clone_wire.push_back(wire);
				wire_set.insert(wire);
				MemVWPoint p0(x0, y0, layer), p1(x1, y1, layer);
				MemVWPoint * ins_p0 = find_point(point_set, &p0);
				MemVWPoint * ins_p1 = find_point(point_set, &p1);
				if (ins_p0 == NULL) {
					p0.set_pack_info(0);
					ins_p0 = &p0;
				}
				if (ins_p1 == NULL) {
					p1.set_pack_info(0);
					ins_p1 = &p1;
				}
				E(prj->add_wire_nocheck(*ins_p0, *ins_p1, layer, patch));
				E(apply_patch(point_set, patch));
			}
			prj->free_patch(patch);
		}
		E(prj->close_write_txn(true));
		set<MemVWPoint *, MemVWPointCmp>::iterator point_it;
		unsigned num = 0;
		for (point_it = point_set.begin(); point_it != point_set.end(); point_it++) {
			MemVWPoint * vwp = *point_it;
			unsigned char layer;
			unsigned wire;
			layer = vwp->get_layer_min();
			if (layer != vwp->get_layer_max() || layer==0) {
				printf("point set error");
				return -2;
			}
			if ((vwp->x & 0x3fff) != 0 || (vwp->y & 0x3fff) != 0) {
				printf("point xy error");
				return -3;
			}
			vector <PairPoint> rp;			
			vwp->get_layer_wire(layer, rp);
			for (int j = 0; j < rp.size(); j++) {
				if ((rp[j].wp.x & 0x3fff) != 0 || (rp[j].wp.y & 0x3fff) != 0) {
					printf("point connect xy error");
					return -3;
				}
				wire = encode_wire(vwp->x, vwp->y, rp[j].wp.x, rp[j].wp.y, layer, shift);
				if (wire_set.find(wire) == wire_set.end()) {
					wire = encode_wire(rp[j].wp.x, rp[j].wp.y, vwp->x, vwp->y, layer, shift);
					if (wire_set.find(wire) == wire_set.end()) {
						printf("pointset mismatch error");
						return -4;
					}
				}
				num++;
			}
		}
		if (num != wire_set.size() * 2) {
			printf("wire num is error");
			return -5;
		}
		num = 0;
		for (unsigned area_y = 0; area_y < area_num; area_y++)
			for (unsigned area_x = 0; area_x < area_num; area_x++) {
				unsigned area = (area_y << 12) | area_x;
				vector <MemVWPoint> points;
				prj->get_internal_points(area, 0, points, false, true);
				for (unsigned i = 0; i < points.size(); i++) {
					point_it = point_set.find(&points[i]);
					if (point_it == point_set.end()) {
						printf("database found point should not exist");
						return -5;
					}
					
					if (points[i] != **point_it) {
						printf("database compare mismatch error");
						return -5;
					}
					num++;
				}
			}
		if (num != point_set.size()) {
			printf("database wire num is error");
			return -5;
		}
	}
	delete prj;
	E(mdb_env_stat(env, &mst));
	mdb_env_close(env);
	printf("bracpage=%d, depth=%d, entry=%d, leafpage=%d, ovflpage=%d, psize=%d\n",
		mst.ms_branch_pages, mst.ms_depth, mst.ms_entries, mst.ms_leaf_pages, mst.ms_overflow_pages, mst.ms_psize);
	printf("test_element_draw0 success\n");
	return 0;
}

void rand_xy(int &x, int &y, unsigned max_x, unsigned max_y)
{
	x = ((rand() << 14) ^ rand()) & (max_x - 1);
	y = ((rand() << 14) ^ rand()) & (max_y - 1);
}

int test_element_draw1()
{
	MDB_env *env;
	int rc;
	unsigned max_x = 0x1ffffff, max_y=0x1ffffff;
	vector<PointPatch*> patch;
	MDB_stat mst;

	mdb_init();
	srand(1);

	remove("./db");
	remove("./db-lock");
	E(mdb_env_create(&env));
	E(mdb_env_set_maxreaders(env, 2));
	E(mdb_env_set_mapsize(env, 0x800000000));
	E(mdb_env_set_maxdbs(env, 64));
	E(mdb_env_open(env, "./db", MDB_NOMETASYNC | MDB_NOSUBDIR, 0664));

	DBProject *prj = new DBProject(env, NULL);

	for (int test_num = 200000; test_num >= 0; test_num--) {
		int x, y;
		E(prj->new_write_txn());
		rand_xy(x, y, max_x, max_y);
		QLine shu(x, 0, x, max_y);
		E(prj->add_wire(shu, 1, true, true, patch));
		if (patch.size()!=1 || !patch[0]->old_points.empty() || patch[0]->new_points.size()!=2) {
			printf("addline patch oldpoint should be empty, check error");
			return -6;
		}
		E(prj->make_point_on_line(shu, QPoint(x, y), 1, patch));
		prj->free_patch(patch);
		E(prj->add_wire(QLine(0, y, x, y), 1, true, false, patch));
		if (patch.size() != 1 || patch[0]->old_points.size() != 1 || patch[0]->new_points.size() != 2) {
			printf("add point patch error");
			return -8;
		}
		prj->free_patch(patch);
		E(prj->add_wire(QLine(x, y, max_x, y), 1, false, true, patch));
		if (patch.size() != 1 || patch[0]->old_points.size() != 1 || patch[0]->new_points.size() != 2) {
			printf("add point patch error");
			return -8;
		}
		prj->free_patch(patch);
		for (unsigned op_num = 0; op_num < 21; op_num++) {
			int x0, y0, x1, y1;
			bool same_quad = true;
			
			rand_xy(x0, y0, max_x, max_y);
			rand_xy(x1, y1, max_x, max_y);
			if (SGN(x0 - x) * SGN(x1 - x) <= 0 || SGN(y0 - y) *SGN(y1 - y) <= 0)
				same_quad = false;
			if (x0 == x && y0 == y || x1 == x && y1 == y)
				continue;
			rc = prj->add_wire(QLine(x0, y0, x1, y1), 1, true, true, patch);
			if (same_quad && rc != Success)  {
				printf("add wire should success, check error,x=%d;y=%d,x0=%d,y0=%d,x1=%d,y1=%d", 
					x, y, x0, y0, x1, y1);
				return -1;
			}
			if (!same_quad && rc == Success) {
				printf("add wire should fail, check error,x=%d;y=%d,x0=%d,y0=%d,x1=%d,y1=%d", 
					x, y, x0, y0, x1, y1);
				return -2;
			}
			if (rc != Success && !patch.empty()) {
				printf("add wire fail patch should be empty, check error,x=%d;y=%d,x0=%d,y0=%d,x1=%d,y1=%d",
					x, y, x0, y0, x1, y1);
				return -3;
			}
			if (rc == Success) {
				if (!patch[0]->old_points.empty()) {
					printf("addline patch oldpoint should be empty, check error,x=%d;y=%d,x0=%d,y0=%d,x1=%d,y1=%d",
						x, y, x0, y0, x1, y1);
					return -4;
				}
				prj->free_patch(patch);
				E(prj->delete_wire(QLine(x0, y0, x1, y1), QLine(x0, y0, x1, y1), 1, patch));
				if (!patch[0]->new_points.empty()) {
					printf("deleteline patch newpoint should be empty, check error,x=%d;y=%d,x0=%d,y0=%d,x1=%d,y1=%d",
						x, y, x0, y0, x1, y1);
					return -5;
				}
				prj->free_patch(patch);
			}			
		}
		E(prj->close_write_txn(false));
	}
	delete prj;
	E(mdb_env_stat(env, &mst));
	mdb_env_close(env);
	printf("bracpage=%d, depth=%d, entry=%d, leafpage=%d, ovflpage=%d, psize=%d\n",
		mst.ms_branch_pages, mst.ms_depth, mst.ms_entries, mst.ms_leaf_pages, mst.ms_overflow_pages, mst.ms_psize);
	printf("test_element_draw1 success\n");
	return 0;
}