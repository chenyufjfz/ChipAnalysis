#include "communication.hpp"
#include "serverperclient.h"
extern RakNet::RakPeerInterface *rak_peer;

void unpack_layer(unsigned char * layer, int layers)
{
    layer[0] = layers & 0xff;
    layer[1] = (layers >> 8) & 0xff;
    layer[2] = (layers >> 16) & 0xff;
    layer[3] = layers >> 24;
}

CellExtractService::CellExtractService(QObject *parent) : QObject(parent)
{
    for (int i=0; i<sizeof(ce)/sizeof(ce[0]); i++)
        ce[i] = NULL;
}

void CellExtractService::cell_extract_req(void * p_cli_addr, void * bk_img_, void *p)
{
    BkImgDB * bk_img = (BkImgDB *) bk_img_;
	RakNet::SystemAddress cli_addr = *((RakNet::SystemAddress *) p_cli_addr);
    RakNet::Packet *packet = (RakNet::Packet *) p;
    ReqSearchPkt * req_pkt = (ReqSearchPkt *) packet->data;
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
				if (layer[i] < bk_img->get_layer_num()) {
					if (ce[layer[i]] == NULL)
						ce[layer[i]] = new CellExtract;
                    ce[layer[i]]->set_train_param(pa->parami[2], pa->parami[3], pa->parami[4], pa->parami[5], pa->parami[6],
                        pa->parami[7], pa->paramf[0], pa->paramf[1], pa->paramf[2], pa->paramf[3]);
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
                    vector<ICLayerWr *> pic;
                    pic.push_back(bk_img->get_layer(layer[i]));
                    ce[layer[i]]->train(pic, objs);
					break;
				}
            RspSearchPkt rsp_pkt;
            rsp_pkt.typeId = ID_RESPONSE_OBJ_SEARCH;
            rsp_pkt.command = CELL_TRAIN;
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
				if (layer[i] < bk_img->get_layer_num()) {
					if (ce[layer[i]] == NULL) {
						qCritical("receive cell extract req before train!");
						break;
					}
                    ce[layer[i]]->set_extract_param(pa->parami[2], pa->parami[3], pa->parami[4], pa->parami[5], pa->parami[6],
                        pa->parami[7], pa->paramf[0], pa->paramf[1], pa->paramf[2], pa->paramf[3]);
					vector<SearchArea> areas;
					for (int j = 0; j < req_pkt->req_search_num; j++)
						areas.push_back(SearchArea(QRect(pa->loc[j].x0, pa->loc[j].y0,
						pa->loc[j].x1 - pa->loc[j].x0 + 1, pa->loc[j].y1 - pa->loc[j].y0 + 1), pa->loc[j].opt));
					vector<MarkObj> objs;
                    vector<ICLayerWr *> pic;
                    pic.push_back(bk_img->get_layer(layer[i]));
                    ce[layer[i]]->extract(pic, areas, objs);
					//send response
					unsigned rsp_len = sizeof(RspSearchPkt) + objs.size() * sizeof(Location);
					RspSearchPkt * rsp_pkt = (RspSearchPkt *)malloc(rsp_len);
					rsp_pkt->typeId = ID_RESPONSE_OBJ_SEARCH;
					rsp_pkt->command = CELL_EXTRACT;
					rsp_pkt->rsp_search_num = objs.size();
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
    rak_peer->DeallocatePacket(packet);
}

VWExtractService::VWExtractService(QObject *parent) : QObject(parent)
{
    vwe = VWExtract::create_extract(0);
}

void VWExtractService::vw_extract_req(void * p_cli_addr, void * bk_img_, void * p)
{
    BkImgDB * bk_img = (BkImgDB *) bk_img_;
    RakNet::SystemAddress cli_addr = *((RakNet::SystemAddress *) p_cli_addr);
    RakNet::Packet *packet = (RakNet::Packet *) p;
    ReqSearchPkt * req_pkt = (ReqSearchPkt *) packet->data;

    switch (req_pkt->command) {
    case VW_EXTRACT:
        if (packet->length != sizeof(ReqSearchPkt) + sizeof(ReqSearchParam) * req_pkt->req_search_num)
            qCritical("receive cell extract req length error!");
        else {
            ReqSearchParam * pa = &(req_pkt->params[0]);
            vector<ICLayerWr *> pic;
            vector<SearchArea> search;
            for (int l=0; l<req_pkt->req_search_num; l++) {
                vwe->set_extract_param(l, pa[l].parami[1], pa[l].parami[2], pa[l].parami[3], pa[l].parami[4],
                          pa[l].parami[5], pa[l].paramf[0], pa[l].paramf[1], pa[l].paramf[2], pa[l].paramf[3]);
                pic.push_back(bk_img->get_layer(pa[l].parami[0]));
                qInfo("extract l=%d, wd=%d, vr=%d, rule=%x, wrule=%x, gd=%d, p1=%f, p2=%f, p3=%f",
                      pa[l].parami[0], pa[l].parami[1], pa[l].parami[2], pa[l].parami[3], pa[l].parami[4],
                      pa[l].parami[5], pa[l].paramf[0], pa[l].paramf[1], pa[l].paramf[2], pa[l].paramf[3]);
                QRect rect(QPoint(pa[l].loc[0].x0, pa[l].loc[0].y0), QPoint(pa[l].loc[0].x1, pa[l].loc[0].y1));
                if (rect.width()<32768 || rect.height()<32768)
                    continue;
                search.push_back(SearchArea(rect, 0));
                qInfo("Receive wire&via search request (%d,%d)_(%d,%d)", rect.left(),
                      rect.top(), rect.right(), rect.bottom());
            }

            vector<MarkObj> objs;
            vwe->extract(pic, search, objs);
            //send response
            unsigned rsp_len = sizeof(RspSearchPkt) + objs.size() * sizeof(Location);
            RspSearchPkt * rsp_pkt = (RspSearchPkt *)malloc(rsp_len);
            rsp_pkt->typeId = ID_RESPONSE_OBJ_SEARCH;
            rsp_pkt->command = VW_EXTRACT;
            rsp_pkt->rsp_search_num = objs.size();
            Location * ploc = &(rsp_pkt->result[0]);
            for (unsigned j = 0; j < objs.size(); j++) {
                ploc[j].x0 = objs[j].p0.x();
                ploc[j].y0 = objs[j].p0.y();
                ploc[j].x1 = objs[j].p1.x();
                ploc[j].y1 = objs[j].p1.y();
                unsigned short t = objs[j].type;
                ploc[j].opt = t << 8 | pa[objs[j].type3].parami[0];
                ploc[j].prob = objs[j].prob;
            }
            rak_peer->Send((char*)rsp_pkt, rsp_len, MEDIUM_PRIORITY,
                RELIABLE_ORDERED, ELEMENT_STREAM, cli_addr, false);
            free(rsp_pkt);
        }
        break;
    }
    rak_peer->DeallocatePacket(packet);
}

ServerPerClient::ServerPerClient(QObject *parent) : QObject(parent)
{
    bk_img = NULL;
    ano_thread = new QThread;
    ce_service = new CellExtractService;
    vw_service = new VWExtractService;
    connect(this, SIGNAL(cell_extract_req(void *, void *, void *)), 
		ce_service, SLOT(cell_extract_req(void *, void *, void *)));
    connect(this, SIGNAL(vw_extract_req(void*,void*,void*)),
            vw_service, SLOT(vw_extract_req(void *, void *, void *)));
    ce_service->moveToThread(ano_thread);
    vw_service->moveToThread(ano_thread);
    ano_thread->start();
}

ServerPerClient::~ServerPerClient()
{
    ce_service->deleteLater();
    ano_thread->quit();
    delete ano_thread;
}

void ServerPerClient::prepare(BkImgDB * bk_img_, RakNet::SystemAddress cli_addr_)
{
    if (bk_img!=NULL)
        qCritical("Server Bkimg can only be set once");
    bk_img = bk_img_;
    cli_addr = cli_addr_;
}

void ServerPerClient::req_bk_img(unsigned char layer, unsigned char scale,
        unsigned short x, unsigned short y, unsigned char priority)
{
    vector<uchar> data;
    bk_img->get_layer(layer)->getRawImgByIdx(data, x, y, scale, sizeof(RspBkImgPkt));
#if ENABLE_CHECK_SUM_BKIMG & ENABLE_CHECK_SUM
    CHECKSUM_TYPE cksum=0;
    if (!data.empty()) {
        CHECKSUM_TYPE *p = (CHECKSUM_TYPE *) &data[reserved];
        for (unsigned i=reserved; i+sizeof(CHECKSUM_TYPE)<data.size(); p++, i+=sizeof(CHECKSUM_TYPE))
            cksum = cksum ^ *p;
    }
#endif
    if (!data.empty()) {
        RspBkImgPkt * rsp_pkt = (RspBkImgPkt *) &data[0];
        rsp_pkt->typeId = ID_RESPONSE_BG_IMG;
        rsp_pkt->scale = scale;
        rsp_pkt->x = x;
        rsp_pkt->y = y;
        rsp_pkt->layer = layer;
        rsp_pkt->len = (unsigned)data.size() - sizeof(RspBkImgPkt);
#if ENABLE_CHECK_SUM_BKIMG & ENABLE_CHECK_SUM
        rsp_pkt->check_sum = cksum;
#endif
        qInfo("Send bg_img l=%d, (%d,%d,%d), pkt_size=%d.", layer, scale, y, x, data.size());
        rak_peer->Send((char*) &data[0], (int)data.size(), static_cast<PacketPriority> (priority),
            RELIABLE_ORDERED, BKIMAGE_STREAM, cli_addr, false);
    }
}

void ServerPerClient::handle_client_req(void * p)
{
    RakNet::Packet *packet = (RakNet::Packet *) p;
    bool need_delete = true;

    if (bk_img==NULL) {
        qCritical("back image is not registered");
        rak_peer->DeallocatePacket(packet);
        return;
    }

    switch (packet->data[0]) {
    case ID_REQUIRE_BG_IMG:
        ReqBkImgPkt * req_pkt;
        req_pkt = (ReqBkImgPkt *) packet->data;
        if  (packet->length != sizeof(ReqBkImgPkt))
            qFatal("Client %s send wrong ID_REQUIRE_BG_IMG.",  packet->systemAddress.ToString());
        else
            req_bk_img(req_pkt->layer, req_pkt->scale, req_pkt->x, req_pkt->y, req_pkt->priority);
        break;

    case ID_REQUIRE_OBJ_SEARCH:
        need_delete = false;
        switch (packet->data[1]) {
        case CELL_TRAIN:            
        case CELL_EXTRACT:
            qInfo("Receive request cell %s", packet->data[1]==CELL_TRAIN ? "train" : "extract");
            emit cell_extract_req((void*) &cli_addr, (void*) bk_img, (void*) packet);
            break;
        case VW_EXTRACT:
            emit vw_extract_req((void*) &cli_addr, (void*) bk_img, (void*) packet);
            break;
        }
        break;

    default:
        qCritical("Unknown packet %d arrive", packet->data[0]);
    }
    if (need_delete)
        rak_peer->DeallocatePacket(packet);
}
