#include "clientthread.h"
#include "communication.hpp"

RakNet::RakPeerInterface *rak_peer =NULL;
RakNet::SystemAddress server_addr;
ClientThread::ClientThread()
{
    finish = false;
}

void ClientThread::end()
{
    finish = true;
}

void ClientThread::run()
{

    RakNet::SocketDescriptor sd;
    sd.socketFamily = AF_INET;
    RakNet::Packet *packet;
    qInfo("Client Start Raknet");

    rak_peer = RakNet::RakPeerInterface::GetInstance();
    rak_peer->Startup(1,&sd, 1);
    rak_peer->Connect("127.0.0.1", SERVER_PORT, 0,0);

    while (!finish)
    {
        for (packet=rak_peer->Receive(); packet; packet=rak_peer->Receive())
        {
            switch (packet->data[0])
            {
            case ID_CONNECTION_REQUEST_ACCEPTED:
                qInfo("Server %s accept my connection.", packet->systemAddress.ToString());
                server_addr = packet->systemAddress;
                emit server_connected();
                break;
            case ID_NO_FREE_INCOMING_CONNECTIONS:
                qWarning("Server is full.");
                break;
            case ID_DISCONNECTION_NOTIFICATION:
                qInfo("Server %s disconnected.", packet->systemAddress.ToString());
                server_addr = RakNet::SystemAddress();
                emit server_disconnected();
                break;
            case ID_CONNECTION_LOST:
                qInfo("lost connection to server %s.", packet->systemAddress.ToString());
                server_addr = RakNet::SystemAddress();
                emit server_disconnected();
                break;

            case ID_RESPONSE_BG_IMG:                
                emit bkimg_packet_arrive((void*) packet);
                break;

            default:
                qCritical("Message identifier %i has arrived.\n", packet->data[0]);
                break;
            }
            if (packet->data[0]!=ID_RESPONSE_BG_IMG)
                rak_peer->DeallocatePacket(packet);
        }
    }

    RakNet::RakPeerInterface::DestroyInstance(rak_peer);

}
