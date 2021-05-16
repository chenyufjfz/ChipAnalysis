#include "searchobject.h"
#include "clientthread.h"

extern RakNet::RakPeerInterface *rak_peer;
extern ClientThread ct;

bool SearchObject::inited = false;
QAtomicInteger<unsigned> SearchObject::global_token(1);

#define PRJ_NAME_LIMIT 255

static void search_result_del(SearchResults * sr)
{
	qInfo("Delete SearchResults");
	delete sr;
}

static void search_request_del(ReqSearchPkt * rsp)
{
	qInfo("Delete SearchRequest");
	free(rsp);
}

static string get_host_name(string prj_file)
{
	string host_name;
	if ((prj_file[0] == '/' && prj_file[1] == '/') ||
		(prj_file[0] == '\\' && prj_file[1] == '\\'))
        host_name = prj_file.substr(2, prj_file.find_first_of("\\/", 3) - 2);
	else
		host_name = "127.0.0.1";
	return host_name;
}

static void couple_prj_license(unsigned char * str, string prj, string license)
{
	str[0] = (unsigned char)prj.length();
	int k = str[0];
	memcpy((char*)&(str[1]), prj.data(), k);
	str[k + 1] = (unsigned char)license.length();
	memcpy((char*)&(str[k + 2]), license.data(), str[k + 1]);
}

void SearchObject::register_new_window(QObject * pobj, ChooseServerPolicy policy)
{
	if (!inited) {
		inited = true;
		qRegisterMetaType<QSharedPointer<SearchResults> >();
		qRegisterMetaType<QSharedPointer<SearchRects> >();
		qRegisterMetaType<QSharedPointer<VWSearchRequest> >();
		qRegisterMetaType<QSharedPointer<RakNet::Packet> >();
		qRegisterMetaType<void *>();
		qRegisterMetaType<string>("string");
	}
	SearchObject * search_object = new SearchObject(pobj);

	if (!connect(pobj, SIGNAL(train_cell(string, string, unsigned char, unsigned char, unsigned char, unsigned char, unsigned char, QRect, float, float, float)),
		search_object, SLOT(train_cell(string, string, unsigned char, unsigned char, unsigned char, unsigned char, unsigned char, QRect, float, float, float))))
		qFatal("Connect train_cell fail");
	if (!connect(pobj, SIGNAL(extract_cell(string, string, unsigned char, unsigned char, unsigned char, unsigned char, QSharedPointer<SearchRects>, float, float, float)),
		search_object, SLOT(extract_cell(string, string, unsigned char, unsigned char, unsigned char, unsigned char, QSharedPointer<SearchRects>, float, float, float))))
		qFatal("Connect extract_cell fail");
	if (!connect(search_object, SIGNAL(extract_cell_done(QSharedPointer<SearchResults>)),
		pobj, SLOT(extract_cell_done(QSharedPointer<SearchResults>))))
		qFatal("Connect extract_cell_done fail");
    if (!connect(pobj, SIGNAL(extract_wire_via(string, string, QSharedPointer<VWSearchRequest>, QRect, int)),
        search_object, SLOT(extract_wire_via(string, string, QSharedPointer<VWSearchRequest>, QRect, int))))
		qFatal("Connect extract_wire_via fail");
	if (!connect(search_object, SIGNAL(extract_wire_via_done(QSharedPointer<SearchResults>)),
		pobj, SLOT(extract_wire_via_done(QSharedPointer<SearchResults>))))
        qFatal("Connect extract_wire_via_done fail");
	if (!connect(pobj, SIGNAL(extract_single_wire(string, string, int, int, int, int, int, int, int, int, void *, float, float, int)),
		search_object, SLOT(extract_single_wire(string, string, int, int, int, int, int, int, int, int, void *, float, float, int))))
        qFatal("Connect extract_single_wire fail");
	if (!connect(search_object, SIGNAL(extract_single_wire_done(QSharedPointer<SearchResults>)),
		pobj, SLOT(extract_single_wire_done(QSharedPointer<SearchResults>))))
		qFatal("Connect extract_single_wire_done fail");
	if (!connect(pobj, SIGNAL(train_via_ml(string, string, int, int, int, int, int, int, int, int, int, int)),
		search_object, SLOT(train_via_ml(string, string, int, int, int, int, int, int, int, int, int, int))))
		qFatal("Connect train_via_ml fail");
	if (!connect(pobj, SIGNAL(train_wire_ml(string, string, int, int, int, int, int, int, int, int)),
		search_object, SLOT(train_wire_ml(string, string, int, int, int, int, int, int, int, int))))
		qFatal("Connect train_wire_ml fail");
	if (!connect(search_object, SIGNAL(train_vw_ml_done(QSharedPointer<SearchResults>)),
		pobj, SLOT(train_vw_ml_done(QSharedPointer<SearchResults>))))
		qFatal("Connect train_via_ml_done fail");
	if (!connect(pobj, SIGNAL(del_vw_ml(string, string, int, int, int, int)),
		search_object, SLOT(del_vw_ml(string, string, int, int, int, int))))
		qFatal("Connect del_vw_ml fail");	
	if (!connect(search_object, SIGNAL(del_vw_ml_done()),
		pobj, SLOT(del_vw_ml_done())))
		qFatal("Connect del_via_ml_done fail");
	if (!connect(pobj, SIGNAL(get_train_vw_ml(string, string, int)),
		search_object, SLOT(get_train_vw_ml(string, string, int))))
		qFatal("Connect get_train_vw_ml fail");
	if (!connect(search_object, SIGNAL(get_train_vw_ml_done(QSharedPointer<SearchResults>)),
		pobj, SLOT(get_train_vw_ml_done(QSharedPointer<SearchResults>))))
		qFatal("Connect del_via_ml_done fail");
	if (!connect(pobj, SIGNAL(extract_ml(string, string, int, int, QPolygon, int, int, int, int, int, int)),
		search_object, SLOT(extract_ml(string, string, int, int, QPolygon, int, int, int, int, int, int))))
		qFatal("Connect extract_ml fail");
	if (!connect(search_object, SIGNAL(extract_ml_done(QSharedPointer<SearchResults>)),
		pobj, SLOT(extract_ml_done(QSharedPointer<SearchResults>))))
		qFatal("Connect extract_ml_done fail");

	if (!connect(&ct, SIGNAL(search_packet_arrive(QSharedPointer<RakNet::Packet>)), search_object, SLOT(search_packet_arrive(QSharedPointer<RakNet::Packet>))))
		qFatal("Connect search_packet_arrive fail");
	if (!connect(&ct, SIGNAL(server_connected(QSharedPointer<RakNet::Packet>)), search_object, SLOT(server_connected(QSharedPointer<RakNet::Packet>))))
		qFatal("Connect ct's server_connected fail");
	if (!connect(&ct, SIGNAL(server_disconnected(QSharedPointer<RakNet::Packet>)), search_object, SLOT(server_disconnected(QSharedPointer<RakNet::Packet>))))
		qFatal("Connect ct's server_disconnected fail");
	if (!connect(search_object, SIGNAL(server_found()), pobj, SLOT(server_connected())))
		qFatal("Connect view  server_connected fail");
	if (!connect(search_object, SIGNAL(server_lost()), pobj, SLOT(server_disconnected())))
		qFatal("Connect view  server_disconnected fail");

	search_object->set_connect_policy(policy);
	qInfo("SearchObject %p is created by %p, policy=%d", search_object, pobj, policy);
}

SearchObject::SearchObject(QObject *parent) : QObject(parent)
{	
	token = 0;
}

SearchObject::~SearchObject()
{
	if (token != 0)
		send_disconnect();
	qInfo("SearchObject %p is deleted, token=%d", this, token);
}

void SearchObject::set_connect_policy(ChooseServerPolicy policy)
{
	qInfo("SearchObject %p policy=%d", this, policy);
	connect_policy = policy;
}

SearchObject::ChooseServerPolicy SearchObject::get_connect_policy()
{
	return connect_policy;
}

void SearchObject::set_prefer_server(string server)
{
	qInfo("SearchObject prefer server=%s", server.c_str());
	prefer_server = server;
}

string SearchObject::get_prefer_server()
{
	return prefer_server;
}

void SearchObject::send_disconnect()
{
	if (token != 0) {
		ReqSearchPkt pkt;
		pkt.typeId = ID_REQUIRE_OBJ_SEARCH;
		pkt.token = token;
		pkt.command = SHUT_DOWN;
		pkt.req_search_num = 0;
		pkt.prj_file[0] = 0;
		rak_peer->Send((char *)&pkt, sizeof(pkt), HIGH_PRIORITY,
			RELIABLE_ORDERED, ELEMENT_STREAM, server_addr, false);
		ct.disconnect_server(server_addr);
	}
}

int SearchObject::try_server(bool resel)
{
	if (token != 0) {
		send_disconnect();			
		resel = true;
	}
	if (cad_idx >= cad_server.size() || cad_server.empty())
		resel = true;
	if (resel) {
		cad_idx = 0;
		token = 0;
		cad_server.clear();
		switch (connect_policy) {
		case RemoteSpecifyLocal:
			cad_server.push_back(prj_host);
			if (!prefer_server.empty())
				cad_server.push_back(prefer_server);
			cad_server.push_back("127.0.0.1");
			break;
		case RemoteSpecify:
			cad_server.push_back(prj_host);
			if (!prefer_server.empty())
				cad_server.push_back(prefer_server);
			break;
		case RemoteLocal:
			cad_server.push_back(prj_host);
			cad_server.push_back("127.0.0.1");
			break;
		case RemoteOnly:
			cad_server.push_back(prj_host);
			break;
		case SpecifyRemoteLocal:
			if (!prefer_server.empty())
				cad_server.push_back(prefer_server);
			cad_server.push_back(prj_host);
			cad_server.push_back("127.0.0.1");
			break;
		case SpecifyRemote:
			if (!prefer_server.empty())
				cad_server.push_back(prefer_server);
			cad_server.push_back(prj_host);
			break;
		case SpecifyLocal:
			if (!prefer_server.empty())
				cad_server.push_back(prefer_server);
			cad_server.push_back("127.0.0.1");
			break;
		case SpecifyOnly:
			if (!prefer_server.empty())
				cad_server.push_back(prefer_server);
			break;
		case LocalOnly:
			cad_server.push_back("127.0.0.1");
			break;
		}
		for (int i = 0; i < cad_server.size() - 1; i++)
			for (int j = i + 1; j < cad_server.size();) {
			if (cad_server[i] == cad_server[j])
				cad_server.erase(cad_server.begin() + j);
			else
				j++;
			}
		Q_ASSERT(!cad_server.empty());
	}		
	else {
		Q_ASSERT(token == 0);
		cad_idx++;
		if (cad_idx >= cad_server.size()) {
			emit server_lost();
			return -1;
		}
	}	
	Q_ASSERT(cad_idx < cad_server.size());
	qInfo("Trying connect to %s", cad_server[cad_idx].c_str());
	int ret = ct.connect_to_server(cad_server[cad_idx], server_addr);
	switch (ret) {
	case (ClientThread::CONNECTED) :		
		token = global_token++;
		qInfo("Connect to server immediately, toke=%d", token);
		emit server_found();
		return 0;
	case (ClientThread::CONNECT_IN_PROGRESS) :
		return 1;
	case (ClientThread::INVALID_NAME) :
		return try_server(false);
	default:
		qCritical("ClientThread connect_to_server return %d", ret);
		return try_server(false);
	}
}

void SearchObject::process_req_queue()
{
	if (token != 0 && !req_queue.empty()) { //if connected and req_queue not empty, send one request
        req_queue.begin()->req_pkt->token = token;
        if (req_queue.begin()->req_pkt->command == SINGLE_WIRE_EXTRACT ||
            req_queue.begin()->req_pkt->command == CELL_TRAIN ||
            req_queue.begin()->req_pkt->command == CELL_EXTRACT ||
            req_queue.begin()->req_pkt->command == VWML_TRAIN ||
            req_queue.begin()->req_pkt->command == VWML_EXTRACT)
            rak_peer->Send((char *)req_queue.begin()->req_pkt.data(), req_queue.begin()->req_len, IMMEDIATE_PRIORITY,
                RELIABLE_ORDERED, SINGLE_WIRE_STREAM, server_addr, false);
        else
            rak_peer->Send((char *)req_queue.begin()->req_pkt.data(), req_queue.begin()->req_len, HIGH_PRIORITY,
                RELIABLE_ORDERED, ELEMENT_STREAM, server_addr, false);
		req_queue.erase(req_queue.begin());
	}
}

void SearchObject::train_cell(string prj, string , unsigned char l0, unsigned char , unsigned char , unsigned char ,
     unsigned char dir, const QRect rect, float , float , float )
{
	qInfo("train %s (%d,%d)_(%d,%d) l=%x dir=%d", prj.c_str(),
          rect.left(), rect.top(), rect.right(), rect.bottom(), l0, dir);
	cell_train_rect = rect;
	cell_train_dir = dir;
}

void SearchObject::extract_cell(string prj, string license, unsigned char l0, unsigned char l1, unsigned char l2, unsigned char l3,
     QSharedPointer<SearchRects> prect, float param1, float param2, float param3)
{
	if (prect->rects.empty() || prect->dir.empty()) {
		qCritical("extract_cell rect empty");
		return;
	}
	if (prj.length() + license.length()  > PRJ_NAME_LIMIT) {
		qCritical("prj name too long:%s", prj.c_str());
		return;
	}
    if (get_host_name(prj) != prj_host || token==0) {
		prj_host = get_host_name(prj);
		req_queue.clear();
		if (try_server(true) < 0)
			return;
	}
	ReqSearch rs;
    rs.req_len = sizeof(ReqSearchPkt) + sizeof(ReqSearchParam) + (prect->rects.size()) * (unsigned) sizeof(Location);
	rs.req_pkt = QSharedPointer<ReqSearchPkt>((ReqSearchPkt *)malloc(rs.req_len), search_request_del);
	couple_prj_license(rs.req_pkt->prj_file, prj, license);
	rs.req_pkt->typeId = ID_REQUIRE_OBJ_SEARCH;
	rs.req_pkt->command = CELL_EXTRACT;
	rs.req_pkt->req_search_num = prect->rects.size() + 1;
	rs.req_pkt->params[0].parami[0] = (int)l0 | ((int)l1 << 8) | ((int)l2 << 16) | ((int)l3 << 24);
	rs.req_pkt->params[0].parami[1] = 0xffffffff;
	rs.req_pkt->params[0].parami[8] = 100 * param1;
	rs.req_pkt->params[0].parami[9] = 100 * param2;
	rs.req_pkt->params[0].parami[10] = 100 *param3;
	rs.req_pkt->params[0].loc[0].opt = cell_train_dir;
	rs.req_pkt->params[0].loc[0].x0 = cell_train_rect.left();
	rs.req_pkt->params[0].loc[0].y0 = cell_train_rect.top();
	rs.req_pkt->params[0].loc[0].x1 = cell_train_rect.right();
	rs.req_pkt->params[0].loc[0].y1 = cell_train_rect.bottom();

	qInfo("extract cell %s", prj.c_str());
    for (int i=0; i < (int) prect->rects.size(); i++) {
		rs.req_pkt->params[0].loc[i + 1].opt = prect->dir[i];
		rs.req_pkt->params[0].loc[i + 1].x0 = prect->rects[i].left();
		rs.req_pkt->params[0].loc[i + 1].y0 = prect->rects[i].top();
		rs.req_pkt->params[0].loc[i + 1].x1 = prect->rects[i].right();
		rs.req_pkt->params[0].loc[i + 1].y1 = prect->rects[i].bottom();
        qInfo("(%d,%d)_(%d,%d) l=%x dir=%d p1=%f, p2=%f, p3=%f",
              prect->rects[i].left(), prect->rects[i].top(),
              prect->rects[i].right(), prect->rects[i].bottom(),
              rs.req_pkt->params[0].parami[0], prect->dir[i], param1, param2, param3);
    }
	req_queue.push_back(rs);
	process_req_queue();
}

void SearchObject::extract_wire_via(string prj, string license, QSharedPointer<VWSearchRequest> preq, const QRect rect, int option)
{
	if (prj.length() + license.length() > PRJ_NAME_LIMIT) {
		qCritical("prj name too long:%s", prj.c_str());
		return;
	}
    if (get_host_name(prj) != prj_host || token==0) {
		prj_host = get_host_name(prj);
		req_queue.clear();
		if (try_server(true) < 0)
			return;
	}
	ReqSearch rs;
    rs.req_len = sizeof(ReqSearchPkt) + sizeof(ReqSearchParam) * preq->lpa.size();
	rs.req_pkt = QSharedPointer<ReqSearchPkt>((ReqSearchPkt *)malloc(rs.req_len), search_request_del);
	couple_prj_license(rs.req_pkt->prj_file, prj, license);
	rs.req_pkt->typeId = ID_REQUIRE_OBJ_SEARCH;
	rs.req_pkt->command = VW_EXTRACT;
	rs.req_pkt->req_search_num = preq->lpa.size();
	ReqSearchParam * pa = &(rs.req_pkt->params[0]);

    qInfo("extract wire_via %s, x0=%d,y0=%d, w=%d,h=%d, opt=%d",
          prj.c_str(), rect.left(), rect.top(), rect.width(), rect.height(), option);
    for (int l=0; l<preq->lpa.size(); l++) {
        pa[l].parami[0] = preq->lpa[l].pi[0];
        pa[l].parami[1] = preq->lpa[l].pi[1];
        pa[l].parami[2] = preq->lpa[l].pi[2];
        pa[l].parami[3] = preq->lpa[l].pi[3];
        pa[l].parami[4] = preq->lpa[l].pi[4];
        pa[l].parami[5] = preq->lpa[l].pi[5];
        pa[l].parami[6] = preq->lpa[l].pi[6];
        pa[l].parami[7] = preq->lpa[l].pi[7];
        pa[l].parami[8] = preq->lpa[l].pi[8];
        pa[l].paramf = preq->lpa[l].pf;
        qInfo("extract_wire, i0=%x,i1=%x,i2=%x,i3=%x,i4=%x,i5=%x,i6=%x,i7=%x,i8=%x,f=%f",
              pa[l].parami[0], pa[l].parami[1], pa[l].parami[2], pa[l].parami[3], pa[l].parami[4],
              pa[l].parami[5], pa[l].parami[6], pa[l].parami[7], pa[l].parami[8], pa[l].paramf);
        pa[l].loc[0].x0 = 0;
        pa[l].loc[0].y0 = 0;
        pa[l].loc[0].x1 = 0;
        pa[l].loc[0].y1 = 0;
    }

    pa[0].loc[0].x0 = rect.left();
    pa[0].loc[0].y0 = rect.top();
    pa[0].loc[0].x1 = rect.right();
    pa[0].loc[0].y1 = rect.bottom();
    pa[0].parami[10] = option;
	req_queue.push_back(rs);
	process_req_queue();
}

void SearchObject::extract_single_wire(string prj, string license, int layer, int wmin, int wmax, int ihigh, int opt, int gray_th, int channel, int scale, void * ploc, float cr, float cg, int shape_mask)
{
	QScopedPointer<vector<QPoint> >loc((vector<QPoint> *) ploc);
	if (prj.length() + license.length() > PRJ_NAME_LIMIT) {
        qCritical("extract_single_wire prj name too long:%s", prj.c_str());
        return;
    }
    if (get_host_name(prj) != prj_host || token==0) {
        prj_host = get_host_name(prj);
        req_queue.clear();
        if (try_server(true) < 0)
            return;
    }
    if (!req_queue.empty()) {
        qWarning("extract_single_wire previous wire or cell extract is not finished");
        return;
    }
    if (wmin > 200 || wmax>1000 || ihigh > 20 || opt > 255 || gray_th>100 || channel > 3 || scale > 255
        || wmin < 0 || wmax < 0 || ihigh < 0 || opt < 0 || gray_th<0 || channel < 0 || scale < 0 || loc->empty()) {
        qWarning("extract_single_wire invalid param");
        return;
    }
	qInfo("extract_single_wire l=%d, wmin=%d, wmax=%d, ihigh=%d, opt=%d, gray_th=%d, channel=%d, scale=%d, cr=%f, cg=%f",
		layer, wmin, wmax, ihigh, opt, gray_th, channel, scale, cr, cg);
    ReqSearch rs;
	rs.req_len = sizeof(ReqSearchPkt)+sizeof(ReqSearchParam)+ (loc->size() - 1) *sizeof(Location);
    rs.req_pkt = QSharedPointer<ReqSearchPkt>((ReqSearchPkt *)malloc(rs.req_len), search_request_del);
	couple_prj_license(rs.req_pkt->prj_file, prj, license);
    rs.req_pkt->typeId = ID_REQUIRE_OBJ_SEARCH;
    rs.req_pkt->command = SINGLE_WIRE_EXTRACT;
    rs.req_pkt->req_search_num = loc->size();
    rs.req_pkt->params[0].parami[0] = layer;
    rs.req_pkt->params[0].parami[1] = 0xffffffff;
    rs.req_pkt->params[0].parami[2] = wmax << 16 | wmin;
    rs.req_pkt->params[0].parami[3] = channel << 24 | gray_th << 16 | opt << 8 | ihigh;
    int cr_int = 100 * cr;
    int cg_int = 100 * cg;
    rs.req_pkt->params[0].parami[4] = (cg_int & 0xff) << 16 | (cr_int & 0xff) << 8 | scale;
    rs.req_pkt->params[0].parami[5] = shape_mask;
	for (int i = 0; i < loc->size(); i++) {
		qInfo("extract_single_wire x=%d, y=%d", loc->at(i).x(), loc->at(i).y());
		rs.req_pkt->params[0].loc[i].opt = 0;
		rs.req_pkt->params[0].loc[i].x0 = loc->at(i).x();
		rs.req_pkt->params[0].loc[i].y0 = loc->at(i).y();
		rs.req_pkt->params[0].loc[i].x1 = loc->at(i).x();
		rs.req_pkt->params[0].loc[i].y1 = loc->at(i).y();
	}
    req_queue.push_back(rs);
    process_req_queue();
}

void SearchObject::train_via_ml(string prj, string license, int layer, int label, int dmin, int dmax, int x, int y, int param3, int param4, int param5, int param6)
{
	if (prj.length() + license.length() > PRJ_NAME_LIMIT) {
		qCritical("prj name too long:%s", prj.c_str());
		return;
	}
	if (dmin > dmax || dmax > 255) {
		qCritical("invalid dmin %d or dmax %d", dmin, dmax);
		return;
	}
	if (get_host_name(prj) != prj_host || token == 0) {
		prj_host = get_host_name(prj);
		req_queue.clear();
		if (try_server(true) < 0)
			return;
	}
	current_ml_train = (label & 1) ? POINT_NORMAL_VIA0 : POINT_NO_VIA;
	ReqSearch rs;
	rs.req_len = sizeof(ReqSearchPkt)+sizeof(ReqSearchParam);
	rs.req_pkt = QSharedPointer<ReqSearchPkt>((ReqSearchPkt *)malloc(rs.req_len), search_request_del);
	couple_prj_license(rs.req_pkt->prj_file, prj, license);
	rs.req_pkt->typeId = ID_REQUIRE_OBJ_SEARCH;
	rs.req_pkt->command = VWML_TRAIN;
	rs.req_pkt->req_search_num = 1;
	rs.req_pkt->params[0].parami[0] = layer << 8 | layer;
	rs.req_pkt->params[0].parami[1] = current_ml_train << 8 | OBJ_POINT;
	rs.req_pkt->params[0].parami[2] = dmax << 8 | dmin;
	rs.req_pkt->params[0].parami[3] = param3;
	rs.req_pkt->params[0].parami[4] = param4;
	rs.req_pkt->params[0].parami[5] = param5;
	rs.req_pkt->params[0].parami[6] = param6;
	rs.req_pkt->params[0].loc[0].x0 = x;
	rs.req_pkt->params[0].loc[0].y0 = y;	
	rs.req_pkt->params[0].loc[0].opt = current_ml_train << 8 | OBJ_POINT;
	qInfo("train_via_ml %s l=%d,(%d,%d), dmin=%d dmax=%d label=%d p3=%x p4=%x p5=%x p6=%x", prj.c_str(), layer,
		x, y, dmin, dmax, label, param3, param4, param5, param6);
	req_queue.push_back(rs);
	process_req_queue();
}

void SearchObject::train_wire_ml(string prj, string license, int layer, int label, int wmin_x, int wmin_y, int via_d, int x, int y, int param3)
{
	if (prj.length() + license.length() > PRJ_NAME_LIMIT) {
		qCritical("prj name too long:%s", prj.c_str());
		return;
	}
	wmin_x = min(wmin_x, 100);
	wmin_y = min(wmin_y, 100);
	via_d = min(via_d, 100);
	if (get_host_name(prj) != prj_host || token == 0) {
		prj_host = get_host_name(prj);
		req_queue.clear();
		if (try_server(true) < 0)
			return;
	}
	current_ml_train = label;
	ReqSearch rs;
	rs.req_len = sizeof(ReqSearchPkt) + sizeof(ReqSearchParam);
	rs.req_pkt = QSharedPointer<ReqSearchPkt>((ReqSearchPkt *)malloc(rs.req_len), search_request_del);
	couple_prj_license(rs.req_pkt->prj_file, prj, license);
	rs.req_pkt->typeId = ID_REQUIRE_OBJ_SEARCH;
	rs.req_pkt->command = VWML_TRAIN;
	rs.req_pkt->req_search_num = 1;
	rs.req_pkt->params[0].parami[0] = layer << 8 | layer;
	rs.req_pkt->params[0].parami[1] = current_ml_train << 8 | OBJ_POINT;
	rs.req_pkt->params[0].parami[2] = wmin_y << 16 | wmin_x << 8 | via_d;
	rs.req_pkt->params[0].parami[3] = param3;
	rs.req_pkt->params[0].parami[4] = 0;
	rs.req_pkt->params[0].parami[5] = 0;
	rs.req_pkt->params[0].parami[6] = 0;
	rs.req_pkt->params[0].loc[0].x0 = x;
	rs.req_pkt->params[0].loc[0].y0 = y;
	rs.req_pkt->params[0].loc[0].opt = current_ml_train << 8 | OBJ_POINT;
	qInfo("train_wire_ml %s l=%d,(%d,%d), label=%d,wmin_x=%d wmin_y=%d p3=%x", prj.c_str(), layer,
		x, y, label, wmin_x, wmin_y, param3);
	req_queue.push_back(rs);
	process_req_queue();
}

void SearchObject::get_train_vw_ml(string prj, string license, int layer)
{
	if (prj.length() + license.length() > PRJ_NAME_LIMIT) {
		qCritical("prj name too long:%s", prj.c_str());
		return;
	}
	if (get_host_name(prj) != prj_host || token == 0) {
		prj_host = get_host_name(prj);
		req_queue.clear();
		if (try_server(true) < 0)
			return;
	}
	current_ml_train = -2;
	ReqSearch rs;
	rs.req_len = sizeof(ReqSearchPkt) + sizeof(ReqSearchParam);
	rs.req_pkt = QSharedPointer<ReqSearchPkt>((ReqSearchPkt *)malloc(rs.req_len), search_request_del);
	couple_prj_license(rs.req_pkt->prj_file, prj, license);
	strcpy((char*)&(rs.req_pkt->prj_file[1]), prj.c_str());
	rs.req_pkt->typeId = ID_REQUIRE_OBJ_SEARCH;
	rs.req_pkt->command = VWML_TRAIN;
	rs.req_pkt->req_search_num = 1;
	rs.req_pkt->params[0].parami[0] = 2 << 16 | layer << 8 | layer; //1 << 16 is delete
	rs.req_pkt->params[0].parami[1] = 0;
	rs.req_pkt->params[0].parami[2] = 0;
	rs.req_pkt->params[0].loc[0].x0 = 0;
	rs.req_pkt->params[0].loc[0].y0 = 0;
	rs.req_pkt->params[0].loc[0].opt = 0;
	qInfo("get vw obj %s l=%d", prj.c_str(), layer);
	req_queue.push_back(rs);
	process_req_queue();
}

void SearchObject::del_vw_ml(string prj, string license, int layer, int d, int x, int y)
{
	if (prj.length() + license.length()  > PRJ_NAME_LIMIT) {
		qCritical("prj name too long:%s", prj.c_str());
		return;
	}
	if (get_host_name(prj) != prj_host || token == 0) {
		prj_host = get_host_name(prj);
		req_queue.clear();
		if (try_server(true) < 0)
			return;
	}
	current_ml_train = -1;
	ReqSearch rs;
	rs.req_len = sizeof(ReqSearchPkt)+sizeof(ReqSearchParam);
    rs.req_pkt = QSharedPointer<ReqSearchPkt>((ReqSearchPkt *)malloc(rs.req_len), search_request_del);
	couple_prj_license(rs.req_pkt->prj_file, prj, license);
	strcpy((char*)&(rs.req_pkt->prj_file[1]), prj.c_str());
	rs.req_pkt->typeId = ID_REQUIRE_OBJ_SEARCH;
	rs.req_pkt->command = VWML_TRAIN;
	rs.req_pkt->req_search_num = 1;
	rs.req_pkt->params[0].parami[0] = 1 << 16 | layer << 8 | layer; //1 << 16 is delete
	rs.req_pkt->params[0].parami[1] = POINT_NORMAL_VIA0 << 8 | OBJ_POINT;
	rs.req_pkt->params[0].parami[2] = d << 8 | d;
	rs.req_pkt->params[0].loc[0].x0 = x;
	rs.req_pkt->params[0].loc[0].y0 = y;
	rs.req_pkt->params[0].loc[0].opt = OBJ_POINT;
	qInfo("del_vw_ml %s (%d,%d)", prj.c_str(), x, y);
	req_queue.push_back(rs);
	process_req_queue();
}

void SearchObject::extract_ml(string prj, string license, int layer_min, int layer_max, QPolygon area, int wmin_x, int wmin_y, int via_d, int force_via, int force_wire, int opt)
{
	if (prj.length() + license.length() > PRJ_NAME_LIMIT) {
		qCritical("prj name too long:%s", prj.c_str());
		return;
	}
	if (get_host_name(prj) != prj_host || token == 0) {
		prj_host = get_host_name(prj);
		req_queue.clear();
		if (try_server(true) < 0)
			return;
	}
	wmin_x = max(0, min(wmin_x, 100));
	wmin_y = max(0, min(wmin_y, 100));
	via_d = max(0, min(via_d, 100));
    force_via = max(-10, min(force_via, 10));
	QRect r = area.boundingRect();
	ReqSearch rs;
	rs.req_len = sizeof(ReqSearchPkt)+sizeof(ReqSearchParam);
	rs.req_pkt = QSharedPointer<ReqSearchPkt>((ReqSearchPkt *)malloc(rs.req_len), search_request_del);
	couple_prj_license(rs.req_pkt->prj_file, prj, license);
	rs.req_pkt->typeId = ID_REQUIRE_OBJ_SEARCH;
	rs.req_pkt->command = VWML_EXTRACT;
	rs.req_pkt->req_search_num = 1;
	rs.req_pkt->params[0].parami[0] = layer_max << 8 | layer_min;
	rs.req_pkt->params[0].parami[1] = POINT_WIRE_INSU << 8 | OBJ_POINT;
    rs.req_pkt->params[0].parami[2] = force_via << 24 | wmin_y << 16 | wmin_x << 8 | via_d;
    rs.req_pkt->params[0].parami[3] = force_wire << 24;
	rs.req_pkt->params[0].parami[4] = 0;
	rs.req_pkt->params[0].parami[5] = 0;
	rs.req_pkt->params[0].parami[6] = 0;
	rs.req_pkt->params[0].loc[0].x0 = r.left();
	rs.req_pkt->params[0].loc[0].y0 = r.top();
	rs.req_pkt->params[0].loc[0].x1 = r.right();
	rs.req_pkt->params[0].loc[0].y1 = r.bottom();
	rs.req_pkt->params[0].loc[0].opt = opt;
    qInfo("extract_ml %s l=%d,%d, lt=(%d,%d), rb=(%d,%d), wmin_x=%d wmin_y=%d, via_d=%d, fv=%d, fw=%x, opt=%x", prj.c_str(), layer_min, layer_max,
        r.left(), r.top(), r.right(), r.bottom(), wmin_x, wmin_y, via_d, force_via, force_wire, opt);
	req_queue.push_back(rs);
	process_req_queue();
}

void SearchObject::server_connected(QSharedPointer<RakNet::Packet> packet)
{
	if (packet->systemAddress == server_addr) {
		switch (packet->data[0]) {
		case ID_CONNECTION_REQUEST_ACCEPTED:
		case ID_ALREADY_CONNECTED:
			if (token != 0)
				qCritical("Already connected, why receive ID_CONNECTION_REQUEST_ACCEPTED again");
			else {
				token = global_token++;
				emit server_found();
				qInfo("Connect to server, toke=%d", token);
				process_req_queue();
			}
			break;
		case ID_CONNECTION_ATTEMPT_FAILED:
		case ID_NO_FREE_INCOMING_CONNECTIONS:
			if (try_server(false) < 0)
				req_queue.clear();
			break;
		}
	}
}

void SearchObject::server_disconnected(QSharedPointer<RakNet::Packet> packet)
{
	if (packet->systemAddress == server_addr) {
        token = 0;
        try_server(false);
        req_queue.clear();
	}
}

void SearchObject::search_packet_arrive(QSharedPointer<RakNet::Packet> packet)
{    
    RspSearchPkt * rsp_pkt = (RspSearchPkt *) packet->data;	
	if (rsp_pkt->token != token)
		return;
	if (packet->systemAddress != server_addr) {
		qCritical("Search result receive from unexcepted server:%s, excepted server:%s",
			packet->systemAddress.ToString(), server_addr.ToString());
		return;
	}
    if (packet->length != sizeof(RspSearchPkt) + rsp_pkt->rsp_search_num * sizeof(Location)) {
        qCritical("Response %d length error!", rsp_pkt->command);        
        return;
    }

	SearchResults * prst = new SearchResults;
    switch (rsp_pkt->command) {
    case CELL_EXTRACT:
        for (unsigned i = 0; i < rsp_pkt->rsp_search_num; i++) {
            MarkObj obj;
			unsigned short t = rsp_pkt->result[i].opt;
			obj.type = t >> 8;
			obj.type2 = (obj.type == OBJ_PARA) ? PARA_PROGRESS : AREA_CELL;
            obj.type3 = t & 0xff;
            obj.state = 0;
            obj.prob = rsp_pkt->result[i].prob;
            obj.p0 = QPoint(rsp_pkt->result[i].x0, rsp_pkt->result[i].y0);
            obj.p1 = QPoint(rsp_pkt->result[i].x1, rsp_pkt->result[i].y1);
            prst->objs.push_back(obj);
            qInfo("extract (%d,%d)_(%d,%d) dir=%d prob=%f",
                  obj.p0.x(), obj.p0.y(), obj.p1.x(), obj.p1.y(),
                  obj.type3, obj.prob);
        }
        emit extract_cell_done(QSharedPointer<SearchResults>(prst, search_result_del));
        break;
    case CELL_TRAIN:
        //emit extract_cell_done(QSharedPointer<SearchResults>(prst, search_result_del));
        search_result_del(prst);
        break;
    case VW_EXTRACT:
    case SINGLE_WIRE_EXTRACT:
        for (unsigned i = 0; i < rsp_pkt->rsp_search_num; i++) {
            MarkObj obj;
            unsigned short t = rsp_pkt->result[i].opt;
            obj.type = t >> 8;
            obj.type3 = t & 0xff;
			obj.type2 = (obj.type == OBJ_PARA) ? PARA_PROGRESS :
						(obj.type == OBJ_LINE) ? LINE_WIRE_AUTO_EXTRACT : POINT_VIA_AUTO_EXTRACT;
            obj.state = 0;
            obj.prob = rsp_pkt->result[i].prob;
            obj.p0 = QPoint(rsp_pkt->result[i].x0, rsp_pkt->result[i].y0);
            obj.p1 = QPoint(rsp_pkt->result[i].x1, rsp_pkt->result[i].y1);
            prst->objs.push_back(obj);
            if (rsp_pkt->command == SINGLE_WIRE_EXTRACT)
                qInfo("single wire extract l=%d, (%d,%d)_(%d,%d)  p=%f, type=%d", obj.type3,
                  obj.p0.x(), obj.p0.y(), obj.p1.x(), obj.p1.y(), obj.prob, obj.type);
        }
        if (rsp_pkt->command == VW_EXTRACT)
            emit extract_wire_via_done(QSharedPointer<SearchResults>(prst, search_result_del));
        else
            emit extract_single_wire_done(QSharedPointer<SearchResults>(prst, search_result_del));
        break;
	case VWML_TRAIN:
	case VWML_EXTRACT:
		for (unsigned i = 0; i < rsp_pkt->rsp_search_num; i++) {
			MarkObj obj;
			unsigned short t = rsp_pkt->result[i].opt;
			obj.type = t >> 8;
			obj.type3 = t & 0xff;
			if (rsp_pkt->command == VWML_EXTRACT) {
				obj.type2 = (obj.type == OBJ_PARA) ? PARA_PROGRESS : 
							(obj.type == OBJ_POINT) ? POINT_VIA_AUTO_EXTRACT1 : LINE_WIRE_AUTO_EXTRACT1;
				obj.prob = rsp_pkt->result[i].prob;
			}
			else {
				obj.type2 = (int)rsp_pkt->result[i].prob;
				obj.prob = 1;
			}
			obj.state = 0;			
			obj.p0 = QPoint(rsp_pkt->result[i].x0, rsp_pkt->result[i].y0);
			obj.p1 = QPoint(rsp_pkt->result[i].x1, rsp_pkt->result[i].y1);
			if (obj.type == OBJ_POINT && (obj.p1.x() < 1 || obj.p1.y() < 1))
				continue;
			prst->objs.push_back(obj);
		}
		if (rsp_pkt->command == VWML_TRAIN) {
			qInfo("VWML train num=%d finish", prst->objs.size());
			if (current_ml_train >= 0)
				emit train_vw_ml_done(QSharedPointer<SearchResults>(prst, search_result_del));
			else
				if (current_ml_train == -1)
					emit del_vw_ml_done();
				else
					if (current_ml_train == -2)
						emit get_train_vw_ml_done(QSharedPointer<SearchResults>(prst, search_result_del));
		}
		else {
			qInfo("VWML extract num=%d finish", prst->objs.size());
			emit extract_ml_done(QSharedPointer<SearchResults>(prst, search_result_del));
		}
		break;
    default:
        qCritical("Response command error %d!", rsp_pkt->command);
    }
	process_req_queue();
}
