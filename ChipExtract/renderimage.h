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
#define MAX_SCALE 5
typedef enum {
    RETURN_UNTIL_ALL_READY,
    RETURN_WHEN_PART_READY,
    NO_NEED_RETURN
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
    QImage pre_img;
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
    map<MapID, RakNet::TimeMS> req_pkt_queue;
    int req_score, max_score;
    list <MapID> cache_list[MAX_SCALE];
    map<MapID, Bkimg> cache_map;
    map<const QObject *, RenderRequest> view_request;
};

#endif // RENDERIMAGE_H
