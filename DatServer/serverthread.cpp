#include "serverthread.h"
#include "communication.hpp"
#include <QFileInfo>
#include <stdio.h>
#include <stdlib.h>
#include "iclayer.h"
#include "RakSleep.h"
#include "RakNetStatistics.h"
#include "GetTime.h"
RakNet::RakPeerInterface *rak_peer;

RakNet::TimeMS last_printtime;

ICLayerWr bk_img("../../M6.dat");
CHECKSUM_TYPE get_bg_img(std::vector<uchar> & data, unsigned char layer, unsigned char scale,
               unsigned short x, unsigned short y, unsigned reserved)
{
    int idx =y;
    idx = idx*11+x;   
    bk_img.getRawImgByIdx(data, idx, scale, reserved);
    CHECKSUM_TYPE cksum=0;
#if ENABLE_CHECK_SUM & 1
    if (!data.empty()) {
        CHECKSUM_TYPE *p = (CHECKSUM_TYPE *) &data[reserved];
        for (unsigned i=reserved; i+sizeof(CHECKSUM_TYPE)<data.size(); p++, i+=sizeof(CHECKSUM_TYPE))
            cksum = cksum ^ *p;
    }
#endif
    return cksum;
}

ServerThread::ServerThread() :QThread(NULL)
{
    finish = false;
    server_port = SERVER_PORT;
    max_user = 64;
    rak_peer = NULL;
}

void ServerThread::end()
{
    finish = true;
}

void ServerThread::run()
{
    RakNet::SocketDescriptor sd(server_port,0);
    sd.socketFamily = AF_INET; // Only IPV4 supports broadcast on 255.255.255.255
    RakNet::Packet *packet;
    qInfo("Server Start net");

    rak_peer = RakNet::RakPeerInterface::GetInstance();
    rak_peer->Startup(max_user, &sd, 1);
    rak_peer->SetMaximumIncomingConnections(max_user);
    rak_peer->SetPerConnectionOutgoingBandwidthLimit(5000000);
    last_printtime = RakNet::GetTimeMS();
    while (!finish)
    {
        for (packet=rak_peer->Receive(); packet; packet=rak_peer->Receive())
        {
            switch (packet->data[0])
            {
            case ID_NEW_INCOMING_CONNECTION:
                qInfo("Another client %s is connected.", packet->systemAddress.ToString());
                break;
            case ID_DISCONNECTION_NOTIFICATION:
                qInfo("Client %s disconnected.", packet->systemAddress.ToString());
                break;
            case ID_CONNECTION_LOST:
                qInfo("Lost connection to Client %s.", packet->systemAddress.ToString());
                break;

            case ID_REQUIRE_BG_IMG:
                {
                    ReqBkImgPkt * req_pkt = (ReqBkImgPkt *) packet->data;
                    if  (packet->length != sizeof(ReqBkImgPkt))
                        qFatal("Client %s send wrong ID_REQUIRE_BG_IMG.",  packet->systemAddress.ToString());
                    else {
                        std::vector<uchar> data;
                        CHECKSUM_TYPE cksum;
                        cksum = get_bg_img(data, req_pkt->layer, req_pkt->scale,
                                         req_pkt->x, req_pkt->y, sizeof(RspBkImgPkt));
                        if (!data.empty()) {
                            RspBkImgPkt * rsp_pkt = (RspBkImgPkt *) &data[0];
                            rsp_pkt->typeId = ID_RESPONSE_BG_IMG;
                            rsp_pkt->scale = req_pkt->scale;
                            rsp_pkt->x = req_pkt->x;
                            rsp_pkt->y = req_pkt->y;
                            rsp_pkt->layer = req_pkt->layer;
                            rsp_pkt->len = (unsigned)data.size() - sizeof(RspBkImgPkt);
#if ENABLE_CHECK_SUM & 1
                            rsp_pkt->check_sum = cksum;
#endif
                            qInfo("Send bg_img l=%d, (%d,%d,%d), pkt_size=%d.", req_pkt->layer, req_pkt->scale,
                                  req_pkt->y, req_pkt->x, data.size());
                            rak_peer->Send((char*) &data[0], (int)data.size(), static_cast<PacketPriority> (req_pkt->priority),
                                           RELIABLE_ORDERED, 0, packet->systemAddress, false);
                        }
                    }
                }
                break;

            default:
                qCritical("Message %i arrived from %s.\n", packet->data[0], packet->systemAddress.ToString());
                break;
            }
            rak_peer->DeallocatePacket(packet);            
        }
        RakSleep(5);
        {
            char text[2048];
            RakNet::RakNetStatistics rss;
            rak_peer->GetStatistics(RakNet::UNASSIGNED_SYSTEM_ADDRESS, &rss);
            if (rss.bytesInResendBuffer >0) {
                RakNet::StatisticsToString(&rss, text, 2);
                qDebug("%s", text);
            }
        }
    }


    RakNet::RakPeerInterface::DestroyInstance(rak_peer);
}
