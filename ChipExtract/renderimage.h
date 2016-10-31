#ifndef RENDERIMAGE_H
#define RENDERIMAGE_H
#include "communication.hpp"
#include "GetTime.h"
#include <QImage>
#include <QRect>
#include <vector>
#include <map>
#include <list>
using namespace std;

typedef unsigned long long MapID;
#define INVALID_MAP_ID 0xffffffffffffff

typedef enum {
    RETURN_UNTIL_ALL_READY,  //Return image when all clear subimages are ready, request may be sent to server and wait all clear subimage ready, caller get only one return, and may wait several seconds.
    RETURN_WHEN_PART_READY,  //Return image immediately, if clear subimage is not ready use blur image instead, and then request may be sent to server and wait all subimage ready, and then return more clear image to caller. So caller may get two or more return. One return is immediate blur image, Another return is clear image after several seconds.
    NO_NEED_RETURN			 //Caller don't need return, request may be sent to server, so caller can use this to preload, next time when caller call RETURN_UNTIL_ALL_READY or RETURN_WHEN_PART_READY, reply time is less
} RenderType;

typedef struct {
    list <MapID>::iterator plist;
    unsigned char * data;
    unsigned int len;
} Bkimg;

typedef struct {
    unsigned char layer; //changed when render request
    QRect rect; //changed when render request
    QSize screen; //changed when render request
    RenderType rt; //changed when render request
    list <MapID> load_queue; //changed when render request
    list <MapID> preload_queue; //changed when render request
    RakNet::TimeMS last_render_time;  //changed when render request
    list <MapID>::iterator rrp_load; //used by send packet
    bool update;  //indicate if load_queue filled by receive packet
    map <MapID, unsigned int> preimg_map;
    int prev_w, prev_h;
    QImage pre_img; //if possible use previous image to save decode time
} RenderRequest;


class RenderImage : public QObject
{
    Q_OBJECT
public:
    explicit RenderImage(QObject *parent = 0);
    static MapID sxy2mapid(unsigned char layer, unsigned char scale, unsigned short x, unsigned short y) {
        MapID ret;
        ret = layer;
        ret = ret <<8;
        ret |= scale;
        ret = ret <<16;
        ret |= x;
        ret = ret <<16;
        ret |= y;
        return ret;
    }

    static void mapid2sxy(MapID m, unsigned char &layer, unsigned char & scale, unsigned short & x, unsigned short &y)
    {
        layer = (m >>40) & 0xff;
        scale = (m >>32) & 0xff;
        x = (m>>16) & 0xffff;
        y = m & 0xffff;
    }

signals:
    void render_bkimg_done(const unsigned char layer, const QRect rect, const QSize screen,
                           QImage image, bool finish, const QObject * view);

public slots:
    void server_connected();
    void server_disconnected();
    void bkimg_packet_arrive(void * p);
    void render_bkimg(const unsigned char layer, const QRect rect,
                      const QSize screen, RenderType rt, const QObject * view, bool preload_enable);

protected:
    void timerEvent( QTimerEvent *event );
    void send_server_req(const QObject * view);
    void remove_cache_list(unsigned char scale);

protected:
    bool call_from_packet_arrivce, connect_to_server;
    int self_test, timer_id;
	//each view's request is in RenderRequest's load_queue and preload_queue, 
	//if req_pkt_queue is within threshold, load_queue and preload_queue is add to req_pkt_queue
    map<MapID, RakNet::TimeMS> req_pkt_queue; 
    int req_score, max_score;
    list <MapID> cache_list[IMAGE_MAX_SCALE]; //cache_list and cache_map makes back_image local cache
    map<MapID, Bkimg> cache_map;
    map<const QObject *, RenderRequest> view_request;
};

#endif // RENDERIMAGE_H
