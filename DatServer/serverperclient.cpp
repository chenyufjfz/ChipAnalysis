#include "communication.hpp"
#include "serverperclient.h"
#include <set>
#include <QScopedPointer>
extern RakNet::RakPeerInterface *rak_peer;
BkImgRoMgr bkimg_faty;
#define DUMP_RESULT     1
void unpack_layer(unsigned char * layer, int layers)
{
    layer[0] = layers & 0xff;
    layer[1] = (layers >> 8) & 0xff;
    layer[2] = (layers >> 16) & 0xff;
    layer[3] = layers >> 24;
}

static string get_host_name(string prj_file)
{
	string host_name;
	if ((prj_file[0] == '/' && prj_file[1] == '/') ||
		(prj_file[0] == '\\' && prj_file[1] == '\\'))
		host_name = prj_file.substr(2, prj_file.find_first_of("\\/") - 2);
	else
		host_name = "127.0.0.1";
	return host_name;
}

static void decouple_prj_license(unsigned char * str, string & prj, string & license)
{
	int k = str[0];
	prj = string((char*)&(str[1]), k);
	license = string((char*)&(str[k + 2]), str[k + 1]);
}

CellExtractService::CellExtractService(unsigned _token, QObject *parent) : QObject(parent)
{
	token = _token;
    for (int i=0; i<sizeof(ce)/sizeof(ce[0]); i++)
        ce[i] = NULL;
	qInfo("CellExtractfor token:%d is created", token);
}

CellExtractService::~CellExtractService()
{
	for (int i = 0; i < sizeof(ce) / sizeof(ce[0]); i++) {
		if (ce[i])
			delete ce[i];
		ce[i] = NULL;
	}
	qInfo("CellExtractfor token:%d is destroyed", token);
}
void CellExtractService::cell_extract_req(void * p_cli_addr, QSharedPointer<BkImgInterface> bk_img, QSharedPointer<RakNet::Packet> packet)
{	
    ReqSearchPkt * req_pkt = (ReqSearchPkt *) packet->data;
	if (req_pkt->token != token)
		return;
	RakNet::SystemAddress cli_addr = *((RakNet::SystemAddress *) p_cli_addr);

    switch (req_pkt->command) {
        case CELL_TRAIN:
        if (packet->length != sizeof(ReqSearchPkt) + sizeof(ReqSearchParam))
            qCritical("receive cell train req length error!");
        else {
            ReqSearchParam * pa = &(req_pkt->params[0]);
            unsigned char layer[8];
            unpack_layer(layer, pa->parami[0]);
            unpack_layer(&layer[4], pa->parami[1]);

            for (int i = 0; i < 8; i++)
				if (layer[i] < bk_img->getLayerNum()) {
					if (ce[layer[i]] == NULL)
						ce[layer[i]] = new CellExtract;
                    ce[layer[i]]->set_train_param(pa->parami[2], pa->parami[3], pa->parami[4], pa->parami[5], pa->parami[6],
                        pa->parami[7], pa->parami[8], pa->parami[9], pa->parami[10], pa->paramf);
					MarkObj obj;
					vector<MarkObj> objs;
					obj.type = OBJ_AREA;
					obj.type2 = AREA_CELL;
					obj.type3 = pa->loc[0].opt;
					obj.p0 = QPoint(pa->loc[0].x0, pa->loc[0].y0);
					obj.p1 = QPoint(pa->loc[0].x1, pa->loc[0].y1);
					objs.push_back(obj);
                    qInfo("start cell train l=%d, dir=%x, (%d,%d)_(%d,%d)", (int) layer[i], obj.type3,
                          obj.p0.x(), obj.p0.y(), obj.p1.x(), obj.p1.y());
					vector<ICLayerWrInterface *> pic;
                    pic.push_back(bk_img->get_layer(layer[i]));
					if (pic.back() != NULL)
						ce[layer[i]]->train(pic, objs);
					else
						qCritical("ce train receive layer[%d]=%d, invalid", i, layer[i]);
					break;
				}
            RspSearchPkt rsp_pkt;
            rsp_pkt.typeId = ID_RESPONSE_OBJ_SEARCH;
            rsp_pkt.command = CELL_TRAIN;
			rsp_pkt.token = req_pkt->token;
            rsp_pkt.rsp_search_num = 0;
            rak_peer->Send((char*) &rsp_pkt, sizeof(RspSearchPkt), MEDIUM_PRIORITY,
                RELIABLE_ORDERED, ELEMENT_STREAM, cli_addr, false);
        }
        break;

        case CELL_EXTRACT:
        if (packet->length != sizeof(ReqSearchPkt) + sizeof(ReqSearchParam) +
                (req_pkt->req_search_num - 1) * sizeof(Location))
            qCritical("receive cell extract req length error!");
		else {
			ReqSearchParam * pa = &(req_pkt->params[0]);
			unsigned char layer[8];
			unpack_layer(layer, pa->parami[0]);
			unpack_layer(&layer[4], pa->parami[1]);

			for (int i = 0; i < 8; i++)
				if (layer[i] < bk_img->getLayerNum()) {
					if (ce[layer[i]] == NULL) {
						qCritical("receive cell extract req before train!");
						break;
					}
                    ce[layer[i]]->set_extract_param(pa->parami[2], pa->parami[3], pa->parami[4], pa->parami[5], pa->parami[6],
						pa->parami[7], pa->parami[8], pa->parami[9], pa->parami[10], pa->paramf);
					vector<SearchArea> areas;
					for (int j = 0; j < req_pkt->req_search_num; j++)
						areas.push_back(SearchArea(QRect(pa->loc[j].x0, pa->loc[j].y0,
						pa->loc[j].x1 - pa->loc[j].x0 + 1, pa->loc[j].y1 - pa->loc[j].y0 + 1), pa->loc[j].opt));
					vector<MarkObj> objs;
					vector<ICLayerWrInterface *> pic;
                    qInfo("start cell extract, layer=%d", layer[i]);
                    pic.push_back(bk_img->get_layer(layer[i]));
					if (pic.back() != NULL)
						ce[layer[i]]->extract(pic, areas, objs);
					else
						qCritical("cell_extract_req Receive layer[%d]=%d, invalid", i, layer[i]);                    
					//send response
                    unsigned rsp_len = sizeof(RspSearchPkt) + (unsigned) objs.size() * sizeof(Location);
					RspSearchPkt * rsp_pkt = (RspSearchPkt *)malloc(rsp_len);
					rsp_pkt->typeId = ID_RESPONSE_OBJ_SEARCH;
					rsp_pkt->command = CELL_EXTRACT;
					rsp_pkt->token = req_pkt->token;
                    rsp_pkt->rsp_search_num = (unsigned) objs.size();
					Location * ploc = &(rsp_pkt->result[0]);
					for (unsigned j = 0; j < objs.size(); j++) {
						ploc[j].x0 = objs[j].p0.x();
						ploc[j].y0 = objs[j].p0.y();
						ploc[j].x1 = objs[j].p1.x();
						ploc[j].y1 = objs[j].p1.y();
						ploc[j].opt = objs[j].type3;
                        ploc[j].prob = objs[j].prob;                        
					}
					rak_peer->Send((char*)rsp_pkt, rsp_len, MEDIUM_PRIORITY,
						RELIABLE_ORDERED, ELEMENT_STREAM, cli_addr, false);
                    free(rsp_pkt);
					break;
				}
		}
        break;
    }
    
}

VWExtractService::VWExtractService(unsigned _token, QObject *parent) : QObject(parent)
{
	token = _token;
    vwe_single = VWExtract::create_extract(1);
	qInfo("VWExtract for token:%d is created", token);
}

VWExtractService::~VWExtractService()
{
    delete vwe_single;
	qInfo("VWExtract for token:%d is destroyed", token);
}

void VWExtractService::vw_extract_req(void * p_cli_addr, QSharedPointer<BkImgInterface> bk_img, QSharedPointer<RakNet::Packet> packet)
{    
    ReqSearchPkt * req_pkt = (ReqSearchPkt *) packet->data;
	if (req_pkt->token != token)
		return;
	RakNet::SystemAddress cli_addr = *((RakNet::SystemAddress *) p_cli_addr);
	
    switch (req_pkt->command) {
    case VW_EXTRACT:
        if (packet->length != sizeof(ReqSearchPkt) + sizeof(ReqSearchParam) * req_pkt->req_search_num)
            qCritical("receive vw extract req length error!");
        else {
            ReqSearchParam * pa = &(req_pkt->params[0]);
            vector<ICLayerWrInterface *> pic;
            vector<SearchArea> search;
			bool invalid_req = false;
			set<int> layer_sets;
            vector<int> map_layer; //map from 0, 1, 2 to actual layer
            map<int, int> anti_map_layer; //map from ParamItem to layer 0, 1, 2
			for (int i = 0; i < req_pkt->req_search_num; i++)
				if (pa[i].parami[0]>=0)
					layer_sets.insert(pa[i].parami[0]);
			for (set<int>::iterator it = layer_sets.begin(); it != layer_sets.end(); it++) {
                anti_map_layer[*it] = (int) map_layer.size();
                int l = -1;
                for (int i = 0; i < bk_img->getLayerNum(); i++) {
                    string layer_name = bk_img->getLayerName(i);
                    int t = (int) layer_name.find_last_of('.');
                    int h = (int) layer_name.find_last_of('M', t);
                    string sub = layer_name.substr(h + 1, t - h - 1);
                    if (atoi(sub.c_str()) == *it) {
                        l = i;
                        break;
                    }
                }
                if (l >= 0) {
                    map_layer.push_back(l);
                    pic.push_back(bk_img->get_layer(l));
                    if (pic.back() == NULL) {
                        qCritical("vw_extract_req receive layer[%d]=%d, invalid", map_layer.size() - 1, l);
                        return;
                    } else
                        qInfo("vw_extract_req receive layer[%d]=%d", map_layer.size() - 1, l);
                }
                else {
                    qCritical("vw_extract_req layer M%d not exist", *it);
                    return;
                }
			}
			VWExtract * vwe = VWExtract::create_extract(0);
            for (int l = 0; l<req_pkt->req_search_num; l++) {
				if (pa[l].parami[0] >= 0) {
					vwe->set_extract_param(anti_map_layer[pa[l].parami[0]], pa[l].parami[1], pa[l].parami[2], pa[l].parami[3], pa[l].parami[4],
						pa[l].parami[5], pa[l].parami[6], pa[l].parami[7], pa[l].parami[8], pa[l].paramf);
				}
				else
					vwe->set_extract_param(pa[l].parami[0], pa[l].parami[1], pa[l].parami[2], pa[l].parami[3], pa[l].parami[4],
						pa[l].parami[5], pa[l].parami[6], pa[l].parami[7], pa[l].parami[8], pa[l].paramf);                
				
                qInfo("extract vw l=%d, i0=%x,i1=%x,i2=%x,i3=%x,i4=%x,i5=%x,i6=%x,i7=%x,i8=%x,f=%f", anti_map_layer[pa[l].parami[0]],
					  pa[l].parami[1], pa[l].parami[2], pa[l].parami[3], pa[l].parami[4],
                      pa[l].parami[5], pa[l].parami[6], pa[l].parami[7], pa[l].parami[8], pa[l].paramf);
                QRect rect(QPoint(pa[l].loc[0].x0, pa[l].loc[0].y0), QPoint(pa[l].loc[0].x1, pa[l].loc[0].y1));
                if (rect.width()<32768 || rect.height()<32768)
                    continue;
                search.push_back(SearchArea(rect, pa[l].parami[10]));
                qInfo("Receive wire&via search request (%d,%d)_(%d,%d)", rect.left(),
                      rect.top(), rect.right(), rect.bottom());
            }

            vector<MarkObj> objs;
			if (!invalid_req)
				vwe->extract(pic, search, objs);
#if DUMP_RESULT
            FILE * fp;
            int scale = 32768 / bk_img->getBlockWidth();
            fp = fopen("result.txt", "w");
            if (fp!=NULL) {
                for (int i = 0; i < objs.size(); i++) {
                    unsigned t = objs[i].type;
                    if (t == OBJ_POINT) {
                        fprintf(fp, "via, l=%d, x=%d, y=%d, prob=%f\n", map_layer[objs[i].type3], objs[i].p0.x() / scale,
                                objs[i].p0.y() / scale, objs[i].prob);
                    }
                    else {
                        fprintf(fp, "wire, l=%d, (x=%d,y=%d)->(x=%d,y=%d), prob=%f\n", map_layer[objs[i].type3], objs[i].p0.x() / scale, objs[i].p0.y() / scale,
                            objs[i].p1.x() / scale, objs[i].p1.y() / scale, objs[i].prob);
                    }
                    continue;
                }
                fclose(fp);
            }
#endif
            //send response
            unsigned rsp_len = sizeof(RspSearchPkt) + (unsigned) objs.size() * sizeof(Location);
            RspSearchPkt * rsp_pkt = (RspSearchPkt *)malloc(rsp_len);
            rsp_pkt->typeId = ID_RESPONSE_OBJ_SEARCH;
            rsp_pkt->command = VW_EXTRACT;
			rsp_pkt->token = req_pkt->token;
            rsp_pkt->rsp_search_num = (unsigned) objs.size();
            Location * ploc = &(rsp_pkt->result[0]);
            for (unsigned j = 0; j < objs.size(); j++) {
                ploc[j].x0 = objs[j].p0.x();
                ploc[j].y0 = objs[j].p0.y();
                ploc[j].x1 = objs[j].p1.x();
                ploc[j].y1 = objs[j].p1.y();
                unsigned short t = objs[j].type;
                ploc[j].opt = t << 8 | map_layer[objs[j].type3];
                ploc[j].prob = objs[j].prob;
            }
            rak_peer->Send((char*)rsp_pkt, rsp_len, MEDIUM_PRIORITY,
                RELIABLE_ORDERED, ELEMENT_STREAM, cli_addr, false);
            free(rsp_pkt);
        }
        break;
    case SINGLE_WIRE_EXTRACT:
        if (packet->length != sizeof(ReqSearchPkt) + sizeof(ReqSearchParam) * req_pkt->req_search_num)
            qCritical("receive single extract req length error!");
        else {
            ReqSearchParam * pa = &(req_pkt->params[0]);
            vector<ICLayerWrInterface *> pic;
            vector<SearchArea> search;
            vector<MarkObj> objs;
            vwe_single->set_extract_param(pa[0].parami[0], pa[0].parami[1], pa[0].parami[2], pa[0].parami[3], pa[0].parami[4],
                    pa[0].parami[5], pa[0].parami[6], pa[0].parami[7], pa[0].parami[8], pa[0].paramf);
            qInfo("extract single wire l=%d, i1=%x,i2=%x,i3=%x,i4=%x,i5=%x,i6=%x,x0=%d,y0=%d,f=%f",
                  pa[0].parami[0], pa[0].parami[1], pa[0].parami[2], pa[0].parami[3], pa[0].parami[4],
                  pa[0].parami[5], pa[0].parami[6], pa[0].loc[0].x0, pa[0].loc[0].y0, pa[0].paramf);
            QRect rect(QPoint(pa[0].loc[0].x0, pa[0].loc[0].y0), QPoint(pa[0].loc[0].x1, pa[0].loc[0].y1));
            search.push_back(SearchArea(rect, 0));
            pic.push_back(bk_img->get_layer(pa[0].parami[0]));
            if (pic.back() == NULL) {
                qCritical("vw_extract_req receive layer=%d, invalid", pa[0].parami[0]);
                return;
            }
            vwe_single->extract(pic, search, objs);
            //send response
            unsigned rsp_len = sizeof(RspSearchPkt) + (unsigned) objs.size() * sizeof(Location);
            RspSearchPkt * rsp_pkt = (RspSearchPkt *)malloc(rsp_len);
            rsp_pkt->typeId = ID_RESPONSE_OBJ_SEARCH;
            rsp_pkt->command = SINGLE_WIRE_EXTRACT;
            rsp_pkt->token = req_pkt->token;
            rsp_pkt->rsp_search_num = (unsigned) objs.size();
            Location * ploc = &(rsp_pkt->result[0]);
            for (unsigned j = 0; j < objs.size(); j++) {
                ploc[j].x0 = objs[j].p0.x();
                ploc[j].y0 = objs[j].p0.y();
                ploc[j].x1 = objs[j].p1.x();
                ploc[j].y1 = objs[j].p1.y();
                unsigned short t = objs[j].type;
                ploc[j].opt = t << 8 | objs[j].type3;
                ploc[j].prob = objs[j].prob;
            }
            rak_peer->Send((char*)rsp_pkt, rsp_len, IMMEDIATE_PRIORITY,
                RELIABLE_ORDERED, SINGLE_WIRE_STREAM, cli_addr, false);
            free(rsp_pkt);
        }
        break;	

	case VWML_TRAIN:
	case VWML_EXTRACT:
		if (packet->length != sizeof(ReqSearchPkt)+sizeof(ReqSearchParam)+
			(req_pkt->req_search_num - 1) * sizeof(Location))
			qCritical("receive vwml train or extract req length error!");
		else {
			ReqSearchParam * pa = &(req_pkt->params[0]);
			vector<ICLayerWrInterface *> pic;
			for (int l = 0; l < bk_img->getLayerNum(); l++) {
				pic.push_back(bk_img->get_layer(l));
				CV_Assert(pic.back() != NULL);
			}
			QScopedPointer<VWExtract> vwe(VWExtract::create_extract(2));
			vector<MarkObj> objs;
			if (req_pkt->command == VWML_TRAIN) {
				qInfo("VWML train p0=%x,p1=%x,p2=%x,p3=%x,p4=%x,p5=%x,p6=%x,p7=%x,p8=%x",
					pa->parami[0], pa->parami[1], pa->parami[2], pa->parami[3], pa->parami[4], pa->parami[5],
					pa->parami[6], pa->parami[7], pa->parami[8]);
				vwe->set_train_param(pa->parami[0], pa->parami[1], pa->parami[2], pa->parami[3], pa->parami[4], pa->parami[5],
					pa->parami[6], pa->parami[7], pa->parami[8], pa->paramf);				
				for (int i = 0; i < req_pkt->req_search_num; i++) {
					MarkObj obj;					
					obj.type = pa->parami[0] & 0xff;
					obj.type2 = pa->loc[i].opt & 0xff;
					obj.type3 = pa->loc[i].opt >> 8 & 0xff;
					obj.p0 = QPoint(pa->loc[i].x0, pa->loc[i].y0);
					obj.p1 = QPoint(pa->loc[i].x1, pa->loc[i].y1);
					objs.push_back(obj);
					qInfo("VWML train l=%d,(x=%d,y=%d),t=%d,t2=%d",(int) obj.type3, obj.p0.x(), 
						obj.p0.y(), (int)obj.type, (int)obj.type2);
				}
				vwe->train(pic, objs);
			} 
			else {
				qInfo("VWML extract p0=%x,p1=%x,p2=%x,p3=%x,p4=%x,p5=%x,p6=%x,p7=%x,p8=%x",
					pa->parami[0], pa->parami[1], pa->parami[2], pa->parami[3], pa->parami[4], pa->parami[5],
					pa->parami[6], pa->parami[7], pa->parami[8]);
				vwe->set_extract_param(pa->parami[0], pa->parami[1], pa->parami[2], pa->parami[3], pa->parami[4], pa->parami[5],
					pa->parami[6], pa->parami[7], pa->parami[8], pa->paramf);
				vector<SearchArea> areas;
				for (int j = 0; j < req_pkt->req_search_num; j++) {
					qInfo("VWML extract (%d,%d)_(%d,%d)", pa->loc[j].x0, pa->loc[j].y0,
						pa->loc[j].x1, pa->loc[j].y1);
					areas.push_back(SearchArea(QRect(pa->loc[j].x0, pa->loc[j].y0,
						pa->loc[j].x1 - pa->loc[j].x0 + 1, pa->loc[j].y1 - pa->loc[j].y0 + 1), pa->loc[j].opt));
				}
				vwe->extract(pic, areas, objs);
			}
			
			//send response
			unsigned rsp_len = sizeof(RspSearchPkt)+(unsigned)objs.size() * sizeof(Location);
			RspSearchPkt * rsp_pkt = (RspSearchPkt *)malloc(rsp_len);
			rsp_pkt->typeId = ID_RESPONSE_OBJ_SEARCH;
			rsp_pkt->command = req_pkt->command;
			rsp_pkt->token = req_pkt->token;
			rsp_pkt->rsp_search_num = (unsigned)objs.size();
			Location * ploc = &(rsp_pkt->result[0]);
			for (unsigned j = 0; j < objs.size(); j++) {
				ploc[j].x0 = objs[j].p0.x();
				ploc[j].y0 = objs[j].p0.y();
				ploc[j].x1 = objs[j].p1.x();
				ploc[j].y1 = objs[j].p1.y();
				unsigned short t = objs[j].type;
				ploc[j].opt = t << 8 | objs[j].type3;
				ploc[j].prob = objs[j].prob;
			}
			rak_peer->Send((char*)rsp_pkt, rsp_len, MEDIUM_PRIORITY,
				RELIABLE_ORDERED, ELEMENT_STREAM, cli_addr, false);
			free(rsp_pkt);
		}
		break;
    }
    
}

bool ServerPerClient::inited = false;
ServerPerClient::ServerPerClient(RakNet::SystemAddress cli_addr_, QObject *parent) : QObject(parent)
{
	if (!inited) {
		inited = true;
		qRegisterMetaType<QSharedPointer<RakNet::Packet> >();
		qRegisterMetaType<QSharedPointer<BkImgInterface> >();
	}
	cli_addr = cli_addr_;
	qInfo("Server for %s is created.", cli_addr.ToString());
	work_thread = new QThread;
	work_thread->start();
	if (!connect(work_thread, SIGNAL(finished()), work_thread, SLOT(deleteLater())))
		qFatal("Connect thread finished failed");
}

ServerPerClient::~ServerPerClient()
{
	map<unsigned, ServicePerToken>::iterator iter;
	for (iter = service_pool.begin(); iter != service_pool.end(); iter++) {
		iter->second.ce_service->deleteLater();
		iter->second.vw_service->deleteLater();
	}
	qInfo("Server for %s is destroyed.", cli_addr.ToString());
    work_thread->quit();
}

void ServerPerClient::handle_client_req(QSharedPointer<RakNet::Packet> packet)
{
	ReqSearchPkt * req_pkt = (ReqSearchPkt *)packet->data;
	unsigned token = req_pkt->token;
	map<unsigned, ServicePerToken>::iterator iter = service_pool.find(token);
	if (cli_addr != packet->systemAddress) 
		qCritical("Received wrong addr %s, excepted %s", packet->systemAddress.ToString(), cli_addr.ToString());
	
	if (iter == service_pool.end()) {
		ServicePerToken service;
		service.ce_service = new CellExtractService(token);
		service.vw_service = new VWExtractService(token);
		if (!connect(this, SIGNAL(cell_extract_req(void *, QSharedPointer<BkImgInterface>, QSharedPointer<RakNet::Packet>)),
			service.ce_service, SLOT(cell_extract_req(void *, QSharedPointer<BkImgInterface>, QSharedPointer<RakNet::Packet>))))
			qFatal("Connect cell_extract_req fail");
		if (!connect(this, SIGNAL(vw_extract_req(void*, QSharedPointer<BkImgInterface>, QSharedPointer<RakNet::Packet>)),
            service.vw_service, SLOT(vw_extract_req(void *, QSharedPointer<BkImgInterface>, QSharedPointer<RakNet::Packet>))))
			qFatal("Connect vw_extract_req fail");
		service.ce_service->moveToThread(work_thread);
		service.vw_service->moveToThread(work_thread);
		service_pool[token] = service;
		iter = service_pool.find(token);		
	}
	Q_ASSERT(iter != service_pool.end());	
	
	switch (req_pkt->typeId) {
    case ID_REQUIRE_OBJ_SEARCH:
		if (req_pkt->command != SHUT_DOWN) {
			string prj;
			string license;
			decouple_prj_license(req_pkt->prj_file, prj, license);
            if (iter->second.bk_img.isNull() || iter->second.bk_img->get_prj_name() != prj) {
				iter->second.bk_img = bkimg_faty.open(prj, license, 0);
                if (iter->second.bk_img.isNull())
                    return;
            }
		}
        switch (req_pkt->command) {
        case CELL_TRAIN:            
        case CELL_EXTRACT:
            qInfo("Receive request cell %s", packet->data[1]==CELL_TRAIN ? "train" : "extract");
			emit cell_extract_req((void*)&cli_addr, iter->second.bk_img, packet);
            break;
        case VW_EXTRACT:
        case SINGLE_WIRE_EXTRACT:
		case VWML_TRAIN:
		case VWML_EXTRACT:
            qInfo("Receive request vwextract %s", packet->data[1]==VW_EXTRACT ? "area" : "single");
            emit vw_extract_req((void*)&cli_addr, iter->second.bk_img, packet);
            break;
		case SHUT_DOWN:
			iter->second.ce_service->deleteLater();
			iter->second.vw_service->deleteLater();
			service_pool.erase(iter);
			break;
		default:
			qCritical("Unknown command %d arrive", req_pkt->command);
        }
        break;

    default:
        qCritical("Unknown packet %d arrive", packet->data[0]);
    }
}
