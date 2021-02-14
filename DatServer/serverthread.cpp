#include "serverthread.h"
#include <QFileInfo>
#include <stdio.h>
#include <stdlib.h>
#include "RakSleep.h"
#include "RakNetStatistics.h"
#include "communication.hpp"

RakNet::RakPeerInterface *rak_peer = NULL;

static void packet_del(RakNet::Packet * packet)
{
	switch (packet->data[0])
	{
	case ID_REQUIRE_OBJ_SEARCH:
		qDebug("Delete obj search request packet");
		break;
	}
	rak_peer->DeallocatePacket(packet);
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
    rak_peer->SetPerConnectionOutgoingBandwidthLimit(10000000);
    last_printtime = RakNet::GetTimeMS();

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
				server = new ServerPerClient(packet->systemAddress);
                server_pools[packet->guid.g] = server;
                break;
            case ID_DISCONNECTION_NOTIFICATION:                
				if (server != NULL)
					delete server;
				else
					qCritical("Unfound server for client %s when receive disconnect", packet->systemAddress.ToString());
				qInfo("Client %s disconnected.", packet->systemAddress.ToString());
                server_pools.erase(packet->guid.g);
                break;
            case ID_CONNECTION_LOST:
                qInfo("Lost connection to Client %s.", packet->systemAddress.ToString());
                if (server!=NULL)
                    delete server;
				else
					qCritical("Unfound server for client %s when receive lost connection", packet->systemAddress.ToString());
                server_pools.erase(packet->guid.g);
                break;

            case ID_REQUIRE_OBJ_SEARCH:
                need_delete=false;
                if (server == NULL) {
                    qCritical("Connected Client can't find ServerPerClient %s", packet->systemAddress.ToString());
                    server = new ServerPerClient(packet->systemAddress);
                    server_pools[packet->guid.g] = server;
                }
				server->handle_client_req(QSharedPointer<RakNet::Packet>(packet, packet_del));
                break;

            default:
                qCritical("Message %i arrived from %s.\n", packet->data[0], packet->systemAddress.ToString());
                break;
            }
            if (need_delete)
                rak_peer->DeallocatePacket(packet);
        }
		RakSleep(10);
        RakNet::TimeMS current_time = RakNet::GetTimeMS();
		/*
        if (current_time -last_printtime>=6000000){
            last_printtime = current_time;
            char text[2048];
            RakNet::RakNetStatistics rss;
            rak_peer->GetStatistics(RakNet::UNASSIGNED_SYSTEM_ADDRESS, &rss);
            if (rss.bytesInResendBuffer >0) {
                RakNet::StatisticsToString(&rss, text, 2);
                qDebug("%s", text);
            }
        }*/
    }
	
    RakNet::RakPeerInterface::DestroyInstance(rak_peer);
}
