#include "clientthread.h"
#include "communication.hpp"
#include "RakSleep.h"
#include "globalconst.h"
RakNet::RakPeerInterface *rak_peer =NULL;
RakNet::SystemAddress server_addr;
extern GlobalConst gcst;

ClientThread::ClientThread()
{
    finish = false;
    connect_state = NO_CONNECT;
}

void ClientThread::end()
{
    finish = true;
}

void ClientThread::run()
{
    char server_ip[] = "127.0.0.1"; //"10.233.140.185";

    RakNet::SocketDescriptor sd;
    sd.socketFamily = AF_INET;
    RakNet::Packet *packet;
    qInfo("Client Start net");

    rak_peer = RakNet::RakPeerInterface::GetInstance();
    rak_peer->Startup(1,&sd, 1);
    rak_peer->Connect(server_ip, SERVER_PORT, 0,0);

    while (!finish)
    {
        for (packet=rak_peer->Receive(); packet; packet=rak_peer->Receive())
        {
			bool need_delete = true;
            switch (packet->data[0])
            {
            case ID_CONNECTION_REQUEST_ACCEPTED:
                qInfo("Server %s accept my connection.", packet->systemAddress.ToString());
                server_addr = packet->systemAddress;
                connect_state = ACQUIRE_IMG_INFO;
                ReqImgInfoPkt req_img_info;
                req_img_info.typeId = ID_REQUIRE_IMG_INFO;
                rak_peer->Send((char *)&req_img_info, sizeof(req_img_info), IMMEDIATE_PRIORITY,
                                   RELIABLE_ORDERED, BKIMAGE_STREAM, server_addr, false);
                break;

            case ID_NO_FREE_INCOMING_CONNECTIONS:
                qWarning("Server is full.");
                connect_state = NO_CONNECT;
                break;

            case ID_DISCONNECTION_NOTIFICATION:
                qInfo("Server %s disconnected.", packet->systemAddress.ToString());
                server_addr = RakNet::SystemAddress();
                if (connect_state == CONNECT) {
                    connect_state = NO_CONNECT;
                    emit server_disconnected();
                }
                connect_state = NO_CONNECT;
                rak_peer->Connect(server_ip, SERVER_PORT, 0,0);
                break;

            case ID_CONNECTION_LOST:
                qInfo("lost connection to server %s.", packet->systemAddress.ToString());
                server_addr = RakNet::SystemAddress();
                if (connect_state == CONNECT) {
                    connect_state = NO_CONNECT;
                    emit server_disconnected();
                }
                connect_state = NO_CONNECT;
                rak_peer->Connect(server_ip, SERVER_PORT, 0,0);
                break;

            case ID_RESPONSE_IMG_INFO:
                RspImgInfoPkt * rsp_info;
                rsp_info = (RspImgInfoPkt *) packet->data;
                qInfo("Receive bg_img info l=%d,w=%d,h=%d, (%d*%d)", rsp_info->num_layer, rsp_info->num_block_x,
                      rsp_info->num_block_y, rsp_info->img_block_h, rsp_info->img_block_w);
                gcst.set(rsp_info->img_block_w, rsp_info->img_block_h, rsp_info->num_block_x,
                         rsp_info->num_block_y, rsp_info->num_layer);
                connect_state = CONNECT;
                emit server_connected();
                break;

            case ID_RESPONSE_BG_IMG:
                if (connect_state != CONNECT)
                    qFatal("Receive BG_IMG Response before connect");
				need_delete = false;
                emit bkimg_packet_arrive((void*) packet);
                break;

            case ID_RESPONSE_OBJ_SEARCH:
                if (connect_state != CONNECT)
                    qFatal("Receive OBJ Response before connect");
                need_delete = false;
                qInfo("Receive obj search response %d", packet->data[1]);
                emit search_packet_arrive((void*) packet);
                break;

            default:
                qCritical("Message identifier %i has arrived.\n", packet->data[0]);
                break;
            }
			if (need_delete)
                rak_peer->DeallocatePacket(packet);
        }
        RakSleep(5);
    }

    RakNet::RakPeerInterface::DestroyInstance(rak_peer);

}
