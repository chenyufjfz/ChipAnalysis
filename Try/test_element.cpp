#include "element_db.h"
#include <map>
#define E(expr) CHECK((rc = (expr)) == MDB_SUCCESS, #expr)
#define RES(err, expr) ((rc = expr) == (err) || (CHECK(!rc, #expr), 0))
#define CHECK(test, msg) ((test) ? (void)0 : ((void)fprintf(stderr, \
    "%s:%d: %s: %s\n", __FILE__, __LINE__, msg, mdb_strerror(rc)), abort()))

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
		wp_set.add_point(vwp);
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
	printf("test_element_db1 success!");
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
	printf("test_element_db2 success!");
	return 0;
}