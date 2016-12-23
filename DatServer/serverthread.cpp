#include "serverthread.h"
#include <QFileInfo>
#include <stdio.h>
#include <stdlib.h>
#include "RakSleep.h"
#include "RakNetStatistics.h"
#include "GetTime.h"
#include "communication.hpp"

RakNet::RakPeerInterface *rak_peer = NULL;

RakNet::TimeMS last_printtime;

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
    rak_peer->SetPerConnectionOutgoingBandwidthLimit(10000000);
    last_printtime = RakNet::GetTimeMS();
#ifndef WIN64
    imgdb.add_new_layer("../../PL.dat");
    imgdb.add_new_layer("../../M1.dat");
    imgdb.add_new_layer("../../M2.dat");
    imgdb.add_new_layer("../../M3.dat");
    imgdb.add_new_layer("../../M4.dat");
#else
    imgdb.add_new_layer("F:/chenyu/work/ChipStitch/data/hanzhou/M1/PL.dat");
    imgdb.add_new_layer("F:/chenyu/work/ChipStitch/data/hanzhou/M1/M1.dat");
    imgdb.add_new_layer("F:/chenyu/work/ChipStitch/data/hanzhou/M1/M2.dat");
    imgdb.add_new_layer("F:/chenyu/work/ChipStitch/data/hanzhou/M1/M3.dat");
    imgdb.add_new_layer("F:/chenyu/work/ChipStitch/data/hanzhou/M1/M4.dat");
#endif
    while (!finish)
    {
        for (packet=rak_peer->Receive(); packet; packet=rak_peer->Receive())
        {
            ServerPerClient * server;
            map<unsigned long long, ServerPerClient*>::iterator server_it;
            server_it = server_pools.find(packet->guid.g);
            if (server_it == server_pools.end())
                server = NULL;
            else
                server = server_it->second;
            bool need_delete = true;
            switch (packet->data[0])
            {
            case ID_NEW_INCOMING_CONNECTION:
                qInfo("Another client %s is connected.", packet->systemAddress.ToString());
                if (server != NULL) {
                    qWarning("Raknet internal error, new connect already have server");
                    delete server;
                }
                server = new ServerPerClient;
                server_pools[packet->guid.g] = server;
                break;
            case ID_DISCONNECTION_NOTIFICATION:
                qInfo("Client %s disconnected.", packet->systemAddress.ToString());
                if (server!=NULL)
                    delete server;
                server_pools.erase(packet->guid.g);
                break;
            case ID_CONNECTION_LOST:
                qInfo("Lost connection to Client %s.", packet->systemAddress.ToString());
                if (server!=NULL)
                    delete server;
                server_pools.erase(packet->guid.g);
                break;

            case ID_REQUIRE_IMG_INFO:
                RspImgInfoPkt rsp_info;
                 int num_wide, num_high;
                rsp_info.typeId = ID_RESPONSE_IMG_INFO;
                rsp_info.img_block_h = imgdb.get_layer(0)->getBlockWidth();
                rsp_info.img_block_w = imgdb.get_layer(0)->getBlockWidth();
                imgdb.get_layer(0)->getBlockNum(num_wide, num_high);
                rsp_info.num_block_x = num_wide;
                rsp_info.num_block_y = num_high;
                rsp_info.num_layer = imgdb.get_layer_num();
                server->prepare(&imgdb, packet->systemAddress);
                qInfo("Send bg_img info l=%d,w=%d,h=%d, (%d*%d)", rsp_info.num_layer, rsp_info.num_block_x,
                      rsp_info.num_block_y, rsp_info.img_block_h, rsp_info.img_block_w);
                rak_peer->Send((char*) &rsp_info, sizeof(RspImgInfoPkt), IMMEDIATE_PRIORITY,
                    RELIABLE_ORDERED, BKIMAGE_STREAM, packet->systemAddress, false);
                break;

            case ID_REQUIRE_BG_IMG:
            case ID_REQUIRE_OBJ_SEARCH:
                need_delete=false;
                if (server == NULL)
                    qFatal("Connected Client can't find ServerPerClient");
                server->handle_client_req(packet);
                break;

            default:
                qCritical("Message %i arrived from %s.\n", packet->data[0], packet->systemAddress.ToString());
                break;
            }
            if (need_delete)
                rak_peer->DeallocatePacket(packet);
        }
        RakSleep(5);
        RakNet::TimeMS current_time = RakNet::GetTimeMS();
        if (current_time -last_printtime>=8000){
            last_printtime = current_time;
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
