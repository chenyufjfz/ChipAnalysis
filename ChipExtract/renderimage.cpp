#include "renderimage.h"
#include "RakPeerInterface.h"
#include "MessageIdentifiers.h"
#include <QPainter>
#include <stdlib.h>
#include <algorithm>
#include <QBuffer>
#include <QDebug>
#include "globalconst.h"

//following parameter is for render_bkimg
const int scale_para0 = 12; //bigger, clearer
const int preload_para = 10;
const int render_time_para = 2000;
//following parameter is for congest control
const int max_load_packet_sent = 6;
const int max_preload_packet_sent = 8;
const int scale_score[] = {81, 27, 9, 3, 1};
const int max_score_init = 190;
const int cache_size_0 = 10; //at least 10

extern RakNet::RakPeerInterface *rak_peer;
extern RakNet::SystemAddress server_addr;
extern GlobalConst gcst;

RenderImage::RenderImage(QObject *parent) : QObject(parent)
{
    call_from_packet_arrivce = false;
    connect_to_server =false;
    max_score = max_score_init;
    req_score = 0;
    for (char s=0, cache_size=cache_size_0; s<MAX_SCALE; s++, cache_size=cache_size*5/2) {
        for (int i=0; i<cache_size; i++)
            cache_list[s].push_back(INVALID_MAP_ID);
    }
    startTimer(2000);
}

/*
 * rect's unit is ns
 * screen's unit is pixel
 */
void RenderImage::render_bkimg(const unsigned char layer, const QRect rect,
                               const QSize screen, RenderType rt, const QObject * view, bool preload_enable)
{
    if (!connect_to_server) {
        qDebug("Server is not connected, cancel request");
        return;
    }
    //following code init RenderRequest struct    
    std::map<const QObject *, RenderRequest>::iterator preq = view_request.find(view);
    if(preq == view_request.end()) {
        RenderRequest render_req;
        view_request[view] = render_req;
        preq = view_request.find(view);
    }
    preq->second.rect = rect;
    preq->second.screen = screen;
    preq->second.layer = layer;
    if (rt!=NO_NEED_RETURN)
        preq->second.rt = rt;
    preq->second.load_queue.clear();
    preq->second.preload_queue.clear();

    //Following code decide scale
    unsigned char scale;
    QRect rpixel = gcst.bu2pixel(rect);
    int w=gcst.img_block_w(), h=gcst.img_block_h();

    for (scale=0; scale<MAX_SCALE; scale++) {
        if (rpixel.width() *10 /gcst.img_block_w()  <= screen.width() * scale_para0 /w  &&
            rpixel.height() *10 /gcst.img_block_h() <= screen.height() *scale_para0 /h)
            break;
        w = w>>1;
        h = h>>1;
    }

    if (scale >=MAX_SCALE) {
        qCritical("Repair me, still no");
        return;
    }

    //Following code generate QImage and fill load_queue
    QImage image((rpixel.right()/gcst.img_block_w() -rpixel.left()/gcst.img_block_w()+1) *w,
                 (rpixel.bottom()/gcst.img_block_h() -rpixel.top()/gcst.img_block_h()+1) *h,
                 QImage::Format_RGB32);
    image.fill(QColor(160, 160, 160));
    QPainter painter(&image);
    for (int x=rpixel.left()/gcst.img_block_w(), x0=0; x<=rpixel.right()/gcst.img_block_w(); x++, x0+=w)
        for (int y=rpixel.top()/gcst.img_block_h(), y0=0; y<=rpixel.bottom()/gcst.img_block_h(); y++, y0+=h) {
            if (x<0 || x>=gcst.num_block_x() || y<0 || y>=gcst.num_block_y())
                continue;
            MapID id;
            map<MapID, Bkimg>::iterator pmap;
            char s;
            for (s = scale; s>=0; s--) { //check if we have clear cache image
                id = sxy2mapid(layer, s, x, y);
                pmap =cache_map.find(id);
                if (pmap!=cache_map.end()) {
                    if (rt==RETURN_WHEN_PART_READY ||
                            rt==RETURN_UNTIL_ALL_READY && preq->second.load_queue.empty()) {
                        QImage subimg;
                        subimg.loadFromData(pmap->second.data, pmap->second.len);
                        painter.drawImage(QRect(x0, y0, w, h), subimg);
                    }
                    break;
                }
            }
            if (pmap==cache_map.end()) { //if not, check blur cache image
                id = sxy2mapid(layer, scale, x, y);
                preq->second.load_queue.push_back(id);
                if (rt==RETURN_WHEN_PART_READY) {
                    for (s = scale+1; s<MAX_SCALE; s++) {
                        id = sxy2mapid(layer, s, x, y);
                        pmap =cache_map.find(id);
                        if (pmap!=cache_map.end()) {
                            QImage subimg;
                            subimg.loadFromData(pmap->second.data, pmap->second.len);
                            painter.drawImage(QRect(x0, y0, w, h), subimg);
                            break;
                        }
                    }
                    if (pmap==cache_map.end()) {
                        qCritical("Repair me, Still not do");
                    }
                }
            } else
                if (s==scale) { //push back to cache_list lastest used
                    id = sxy2mapid(layer, s, x, y);
                    Q_ASSERT(*(pmap->second.plist)==id);
                    cache_list[s].erase(pmap->second.plist);
                    cache_list[s].push_back(id);
                }
        }
    preq->second.update = false;
    if (rt==RETURN_WHEN_PART_READY ||
            rt==RETURN_UNTIL_ALL_READY && preq->second.load_queue.empty()) {
        QRect render_rect_pixel(rpixel.left()/gcst.img_block_w()*gcst.img_block_w(),
                                rpixel.top()/gcst.img_block_h()*gcst.img_block_h(),
                                rpixel.right()/gcst.img_block_w()*gcst.img_block_w()+gcst.img_block_w(),
                                rpixel.bottom()/gcst.img_block_h()*gcst.img_block_h()+gcst.img_block_h());

        emit render_bkimg_done(layer, gcst.pixel2bu(render_rect_pixel), screen,
                       image, preq->second.load_queue.empty(), preq->first);
        preq->second.last_render_time = RakNet::GetTimeMS();
        if (preq->second.load_queue.empty())
            preq->second.rt = NO_NEED_RETURN;
    }

    //Following code update preload queue
    if (preload_enable) {
        int dmax = (rpixel.width()/gcst.img_block_w() + rpixel.height()/gcst.img_block_h()) * preload_para/20 +1;
        unsigned char s2 = max(scale+2, MAX_SCALE-1);
        for (int d=1; d<dmax; d++) {
            int x, y;
            map<MapID, Bkimg>::iterator pmap;
            for (int k=0; k<=1; k++) { //preload up and down
                y =(k==0) ? rpixel.top()/gcst.img_block_h()-d : rpixel.bottom()/gcst.img_block_h()+d;
                if (y<0 || y>=gcst.num_block_y())
                    continue;
                for (x=rpixel.left()/gcst.img_block_w()-d; x<=rpixel.right()/gcst.img_block_w()+d; x++) {
                    if (x<0 || x>=gcst.num_block_x())
                        continue;
                    unsigned char s;
                    for (s=0; s<=s2; s++) {
                        MapID id = sxy2mapid(layer, s, x, y);
                        pmap = cache_map.find(id);
                        if (pmap!=cache_map.end()) {
                            if (s==s2) {
                                Q_ASSERT(*(pmap->second.plist)==id);
                                cache_list[s].erase(pmap->second.plist);
                                cache_list[s].push_back(id);
                            }
                            break;
                        }
                        if (req_pkt_queue.find(id)!=req_pkt_queue.end())
                            break;
                    }
                    if (s>s2)
                        preq->second.preload_queue.push_back(sxy2mapid(layer, s2, x, y));
                }
            }
            for (int k=0; k<=1; k++) { //preload rect left and right
                x =(k==0) ? rpixel.left()/gcst.img_block_w()-d : rpixel.right()/gcst.img_block_w()+d;
                if (x<0 || x>=gcst.num_block_x())
                    continue;
                for (y=rpixel.top()/gcst.img_block_h()-d+1; y<=rpixel.bottom()/gcst.img_block_h()+d-1; y++) {
                    if (y<0 || y>=gcst.num_block_y())
                        continue;
                    unsigned char s;
                    for (s=0; s<=s2; s++) {
                        MapID id = sxy2mapid(layer, s, x, y);
                        pmap = cache_map.find(id);
                        if (pmap!=cache_map.end()) {
                            if (s==s2) {
                                Q_ASSERT(*(pmap->second.plist)==id);
                                cache_list[s].erase(pmap->second.plist);
                                cache_list[s].push_back(id);
                            }
                            break;
                        }
                        if (req_pkt_queue.find(id)!=req_pkt_queue.end())
                            break;
                    }
                    if (s>s2)
                        preq->second.preload_queue.push_back(sxy2mapid(layer, s2, x, y));
                }
            }
        }

        for (int d=-1; d<=1; d+=2) { //preload different layer
            int l =layer;
            l +=d;
            if (l<0 || l>=gcst.num_layer())
                continue;
            for (int x=rpixel.left()/gcst.img_block_w()-1; x<=rpixel.right()/gcst.img_block_w()+1; x++)
                for (int y=rpixel.top()/gcst.img_block_h()-1; y<=rpixel.bottom()/gcst.img_block_h()+1; y++) {
                    if (x<0 || x>=gcst.num_block_x() || y<0 || y>=gcst.num_block_y())
                        continue;
                    unsigned char s;
                    for (s=0; s<=s2; s++) {
                        MapID id = sxy2mapid(l, s, x, y);
                        map<MapID, Bkimg>::iterator pmap = cache_map.find(id);
                        if (pmap!=cache_map.end()) {
                            if (s==s2) {
                                Q_ASSERT(*(pmap->second.plist)==id);
                                cache_list[s].erase(pmap->second.plist);
                                cache_list[s].push_back(id);
                            }
                            break;
                        }
                        if (req_pkt_queue.find(id)!=req_pkt_queue.end())
                            break;
                    }
                    if (s>s2)
                        preq->second.preload_queue.push_back(sxy2mapid(l, s2, x, y));
                }
        }
    }
    //Following code send request packet to server
    if (!call_from_packet_arrivce)
        send_server_req(view);
}

void RenderImage::server_connected()
{
    connect_to_server = true;
}

void RenderImage::server_disconnected()
{
    connect_to_server = false;
}

void RenderImage::bkimg_packet_arrive(void * p)
{
    RakNet::Packet *packet = (RakNet::Packet *) p;
    RspBkImgPkt * rsp_pkt = (RspBkImgPkt *) packet->data;
    unsigned char s;
    if  (packet->length != sizeof(RspBkImgPkt) + rsp_pkt->len)
        qCritical("Server %s send wrong ID_RESPONSE_BG_IMG.",  packet->systemAddress.ToString());
    bool found_req_pkt=false;
    qDebug("receive ID_RESPONSE_BG_IMG (l=%d, s=%d, x=%d, y=%d).",
           rsp_pkt->layer, rsp_pkt->scale, rsp_pkt->x, rsp_pkt->y);

    //following code remove response packet from req_pkt_queue, load_queue and preload_queue
    MapID rsp_id = sxy2mapid(rsp_pkt->layer, rsp_pkt->scale, rsp_pkt->x, rsp_pkt->y);
    if (req_pkt_queue.find(rsp_id) !=req_pkt_queue.end()) {
        found_req_pkt =true;
        req_pkt_queue.erase(rsp_id);
        req_score -= scale_score[rsp_pkt->scale];
        Q_ASSERT(req_score>=0);
    }

    for (s=rsp_pkt->scale; s<MAX_SCALE; s++) {
        MapID id = sxy2mapid(rsp_pkt->layer, s, rsp_pkt->x, rsp_pkt->y);
        for (map<const QObject *, RenderRequest>::iterator it = view_request.begin(); it!=view_request.end(); it++) {
            list <MapID>::iterator rit = find(it->second.load_queue.begin(), it->second.load_queue.end(), id);
            if (rit!=it->second.load_queue.end()) {
                it->second.update = true;
                it->second.load_queue.erase(rit);
            }
            rit = find(it->second.preload_queue.begin(), it->second.preload_queue.end(), id);
            if (rit!=it->second.preload_queue.end())
                it->second.preload_queue.erase(rit);
        }
    }

    //Following code send new request packet to server
    send_server_req(NULL);

    //following code save response image data
    if (found_req_pkt) {
        cache_list[rsp_pkt->scale].push_back(rsp_id);
        Bkimg img;
        img.data = (unsigned char *)malloc(rsp_pkt->len);
        img.len = rsp_pkt->len;
        memcpy(img.data, packet->data + sizeof(RspBkImgPkt), rsp_pkt->len);
        img.plist = cache_list[rsp_pkt->scale].end();
        img.plist--;
        Q_ASSERT(*img.plist == rsp_id);
        cache_map[rsp_id] = img;
    } else
        qCritical("receive unsend request (l=%d, s=%d, x=%d, y=%d).",
               rsp_pkt->layer, rsp_pkt->scale, rsp_pkt->x, rsp_pkt->y);
    s = rsp_pkt->scale;
    rak_peer->DeallocatePacket(packet);

    //following code check and call render_view
    call_from_packet_arrivce = true;
    for (map<const QObject *, RenderRequest>::iterator it = view_request.begin(); it!=view_request.end(); it++) {
        if (it->second.update && (it->second.rt!=NO_NEED_RETURN && it->second.load_queue.empty()
             || it->second.rt==RETURN_WHEN_PART_READY && RakNet::GetTimeMS() - it->second.last_render_time >render_time_para))
            render_bkimg(it->second.layer, it->second.rect, it->second.screen, it->second.rt, it->first, false);
    }
    call_from_packet_arrivce = false;

    if (found_req_pkt)
        remove_cache_list(s);
}

void RenderImage::send_server_req(const QObject * view)
{
    map<const QObject *, RenderRequest>::iterator pview;
    int loop;
    RakNet::TimeMS cur_time;
    bool loadqueue_finish;
    ReqBkImgPkt pkt;

    Q_ASSERT(server_addr !=RakNet::SystemAddress());

    //check and send load queue packet
    for (pview= view_request.begin(); pview!=view_request.end(); pview++)
        pview->second.rrp_load = pview->second.load_queue.begin();
    cur_time = RakNet::GetTimeMS();
    loop=0;
    if (view==NULL)
        pview = view_request.begin();
    else
        pview = view_request.find(view);
    Q_ASSERT(pview!=view_request.end());    
    pkt.priority = MEDIUM_PRIORITY;
    pkt.typeId = ID_REQUIRE_BG_IMG;

    //scale_score[1] is reserve space
    loadqueue_finish = false;
    while (req_score + scale_score[1] < max_score && loop < max_load_packet_sent) {
        for (; pview->second.rrp_load !=pview->second.load_queue.end(); pview->second.rrp_load++) {
            MapID id = *pview->second.rrp_load;
            mapid2sxy(id, pkt.layer, pkt.scale, pkt.x, pkt.y);
            Q_ASSERT(pkt.scale<MAX_SCALE);
            if (req_score + scale_score[pkt.scale] > max_score) //too big packet, congest control
                continue;
            bool already_request = false;
            for (char s =pkt.scale; s>=0; s--) {
                MapID sid = sxy2mapid(pkt.layer, s, pkt.x, pkt.y);
                if (req_pkt_queue.find(sid) !=req_pkt_queue.end()) {//already request
                    already_request = true;
                    break;
                }
            }
            if (already_request)
                continue;
            rak_peer->Send((char *)&pkt, sizeof(ReqBkImgPkt), MEDIUM_PRIORITY,
                   RELIABLE, 0, server_addr, false);
            req_score += scale_score[pkt.scale];
            req_pkt_queue[id] = cur_time;
            pview->second.rrp_load++;
            loadqueue_finish = false;
            break;
        }
        pview++;
        if (pview==view_request.end()) {
            if (loadqueue_finish)
                break;
            loadqueue_finish = true;
            loop++;
            pview = view_request.begin();
        }
    }

    //check and send preload queue packet
    for (pview= view_request.begin(); pview!=view_request.end(); pview++)
        pview->second.rrp_load = pview->second.preload_queue.begin();
    cur_time = RakNet::GetTimeMS();
    loop=0;
    if (view==NULL)
        pview = view_request.begin();
    else
        pview = view_request.find(view);
    Q_ASSERT(pview!=view_request.end());
    pkt.priority = LOW_PRIORITY;
    pkt.typeId = ID_REQUIRE_BG_IMG;
    //scale_score[0] is reserve space
    loadqueue_finish = false;
    while (req_score + scale_score[0] < max_score && loop < max_preload_packet_sent) {
        for (; pview->second.rrp_load !=pview->second.preload_queue.end(); pview->second.rrp_load++) {
            MapID id = *pview->second.rrp_load;
            mapid2sxy(id, pkt.layer, pkt.scale, pkt.x, pkt.y);
            Q_ASSERT(pkt.scale<MAX_SCALE);
            if (req_score + scale_score[pkt.scale] > max_score) //too big packet, congest control
                continue;
            bool already_request = false;
            for (char s =pkt.scale; s>=0; s--) {
                MapID sid = sxy2mapid(pkt.layer, s, pkt.x, pkt.y);
                if (req_pkt_queue.find(sid) !=req_pkt_queue.end()) {//already request
                    already_request = true;
                    break;
                }
            }
            if (already_request)
                continue;
            rak_peer->Send((char *)&pkt, sizeof(ReqBkImgPkt), MEDIUM_PRIORITY,
                   RELIABLE, 0, server_addr, false);
            req_score += scale_score[pkt.scale];
            req_pkt_queue[id] = cur_time;
            pview->second.rrp_load++;
            loadqueue_finish = false;
            break;
        }
        pview++;
        if (pview==view_request.end()) {
            if (loadqueue_finish)
                break;
            loadqueue_finish = true;
            loop++;
            pview = view_request.begin();
        }
    }
}
/*
 * it will move cache_list[scale].front() to cache_list[scale+1].back()
 * cache_list[scale+1].front() to cache_list[scale+2].back()
 */
void RenderImage::remove_cache_list(unsigned char scale)
{
    unsigned char layer;
    unsigned short x, y;

    if (scale>=MAX_SCALE)
        return;
    MapID id = cache_list[scale].front();
    cache_list[scale].erase(cache_list[scale].begin());
    if (id==INVALID_MAP_ID)
        return;

    map<MapID, Bkimg>::iterator it = cache_map.find(id);
    Q_ASSERT(it!=cache_map.end());

    mapid2sxy(id, layer, scale, x, y);
    MapID id_1 = sxy2mapid(layer, scale+1, x, y);
    if (cache_map.find(id_1) !=cache_map.end()
            || scale==MAX_SCALE-1) { //Already cached in higher scale, deleted directly
        free(it->second.data);
        cache_map.erase(it);
        qDebug() << "erase " <<id;
        return;
    }

    QImage bkimg, bkimg_1;
    bkimg.loadFromData((uchar*) it->second.data, it->second.len);
    bkimg_1 = bkimg.scaled(bkimg.width()/2, bkimg.height()/2);
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    bkimg_1.save(&buffer, "JPG"); // scale image into ba in JPG format

    free(it->second.data);
    cache_map.erase(it);
    qDebug() << "move " << id <<" to " <<id_1;

    cache_list[scale+1].push_back(id_1);
    Bkimg img;
    img.data = (unsigned char *)malloc(ba.size()); //is it correct?
    img.len = ba.size();
    memcpy(img.data, ba.data(), ba.size());
    img.plist = cache_list[scale+1].end();
    img.plist--;
    Q_ASSERT(*img.plist == id_1);
    cache_map[id_1] = img;
    remove_cache_list(scale+1);
}

void RenderImage::timerEvent( QTimerEvent *event)
{
    qDebug("Receive timer");
}
