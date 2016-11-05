#include "searchobject.h"
#include "RakPeerInterface.h"
#include "globalconst.h"

extern RakNet::RakPeerInterface *rak_peer;
extern RakNet::SystemAddress server_addr;
extern GlobalConst gcst;

SearchObject::SearchObject(QObject *parent) : QObject(parent)
{
    connect_to_server = false;
}

void SearchObject::server_connected()
{
    connect_to_server = true;
}

void SearchObject::server_disconnected()
{
    connect_to_server = false;
}

static void search_result_del(SearchResults * ms)
{
    qInfo("Do delete SearchResults");
    delete ms;
}

void SearchObject::train_cell(unsigned char l0, unsigned char l1, unsigned char l2, unsigned char l3,
     unsigned char dir, const QRect rect, float param1, float param2, float param3)
{
    if (!connect_to_server)
        return;

    int req_len = sizeof(ReqSearchPkt) + sizeof(ReqSearchParam);
    ReqSearchPkt * req_pkt = (ReqSearchPkt *) malloc(req_len);
    req_pkt->typeId = ID_REQUIRE_OBJ_SEARCH;
    req_pkt->command = CELL_TRAIN;
    req_pkt->req_search_num = 1;
    req_pkt->params[0].parami[0] = (int) l0 | ((int)l1 << 8) | ((int)l2 << 16) | ((int)l3 << 24);
    req_pkt->params[0].parami[1] = 0xffffffff;
    req_pkt->params[0].paramf[0] = param1;
    req_pkt->params[0].paramf[1] = param2;
    req_pkt->params[0].paramf[2] = param3;
    req_pkt->params[0].loc[0].opt = dir;
    req_pkt->params[0].loc[0].x0 = rect.left();
    req_pkt->params[0].loc[0].y0 = rect.top();
    req_pkt->params[0].loc[0].x1 = rect.right();
    req_pkt->params[0].loc[0].y1 = rect.bottom();
    qInfo("train (%d,%d)_(%d,%d) l=%x dir=%d p1=%f, p2=%f, p3=%f",
          rect.left(), rect.top(), rect.right(), rect.bottom(),
          req_pkt->params[0].parami[0], dir, param1, param2, param3);
    rak_peer->Send((char *)req_pkt, req_len, HIGH_PRIORITY,
                   RELIABLE_ORDERED, ELEMENT_STREAM, server_addr, false);
    free(req_pkt);
}

void SearchObject::extract_cell(unsigned char l0, unsigned char l1, unsigned char l2, unsigned char l3,
     QSharedPointer<SearchRects> prect, float param1, float param2, float param3)
{
    if (!connect_to_server)
        return;

     unsigned req_len = sizeof(ReqSearchPkt) + sizeof(ReqSearchParam) + (prect->rects.size() -1) * sizeof(Location);
     ReqSearchPkt * req_pkt = (ReqSearchPkt * ) malloc(req_len);
     req_pkt->typeId = ID_REQUIRE_OBJ_SEARCH;
     req_pkt->command = CELL_EXTRACT;
     req_pkt->req_search_num = prect->rects.size();
     req_pkt->params[0].parami[0] = (int) l0 | ((int)l1 << 8) | ((int)l2 << 16) | ((int)l3 << 24);
     req_pkt->params[0].parami[1] = 0xffffffff;
     req_pkt->params[0].paramf[0] = param1;
     req_pkt->params[0].paramf[1] = param2;
     req_pkt->params[0].paramf[2] = param3;

     for (int i=0; i<prect->rects.size(); i++) {
         req_pkt->params[0].loc[i].opt = prect->dir[i];
         req_pkt->params[0].loc[i].x0 = prect->rects[i].left();
         req_pkt->params[0].loc[i].y0 = prect->rects[i].top();
         req_pkt->params[0].loc[i].x1 = prect->rects[i].right();
         req_pkt->params[0].loc[i].y1 = prect->rects[i].bottom();
         qInfo("extract (%d,%d)_(%d,%d) l=%x dir=%d p1=%f, p2=%f, p3=%f",
               prect->rects[i].left(), prect->rects[i].top(),
               prect->rects[i].right(), prect->rects[i].bottom(),
               req_pkt->params[0].parami[0], prect->dir[i], param1, param2, param3);
     }
     rak_peer->Send((char *)req_pkt, req_len, HIGH_PRIORITY,
                    RELIABLE_ORDERED, ELEMENT_STREAM, server_addr, false);
     free(req_pkt);
}

void SearchObject::search_packet_arrive(void * p)
{
    RakNet::Packet *packet = (RakNet::Packet *) p;
    RspSearchPkt * rsp_pkt = (RspSearchPkt *) packet->data;
    SearchResults * prst = new SearchResults;

    if (packet->length != sizeof(RspSearchPkt) + rsp_pkt->rsp_search_num * sizeof(Location)) {
        qCritical("Response %d length error!", rsp_pkt->command);
        rak_peer->DeallocatePacket(packet);
        return;
    }

    switch (rsp_pkt->command) {
    case CELL_EXTRACT:
        for (unsigned i = 0; i < rsp_pkt->rsp_search_num; i++) {
            MarkObj obj;
            obj.type = OBJ_AREA;
            obj.type2 = AREA_CELL;
            obj.type3 = rsp_pkt->result[i].opt;
            obj.state = 0;
            obj.prob = rsp_pkt->result[i].prob;
            obj.p0 = QPoint(rsp_pkt->result[i].x0, rsp_pkt->result[i].y0);
            obj.p1 = QPoint(rsp_pkt->result[i].x1, rsp_pkt->result[i].y1);
            prst->objs.push_back(obj);
            qInfo("extract (%d,%d)_(%d,%d) dir=%d prob=%f",
                  obj.p0.x(), obj.p0.y(), obj.p1.x(), obj.p1.y(),
                  obj.type3, obj.prob);
        }
        break;
    case CELL_TRAIN:
        break;
    default:
        qCritical("Response command error %d!", rsp_pkt->command);
    }
    emit extract_cell_done(QSharedPointer<SearchResults>(prst, search_result_del));
    rak_peer->DeallocatePacket(packet);
}
