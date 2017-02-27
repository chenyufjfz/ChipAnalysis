#include "clientthread.h"
#include "communication.hpp"
#include "RakSleep.h"

RakNet::RakPeerInterface *rak_peer =NULL;


static void packet_del(RakNet::Packet * packet)
{
	switch (packet->data[0])
	{
	case ID_CONNECTION_REQUEST_ACCEPTED:
	case ID_NO_FREE_INCOMING_CONNECTIONS:
		qDebug("Delete Server connect packet");
		break;
		 
	case ID_DISCONNECTION_NOTIFICATION:
	case ID_CONNECTION_LOST:
		qDebug("Delete Server disconnect packet");
		break;

	case ID_RESPONSE_OBJ_SEARCH:
		qDebug("Delete obj search response packet");
		break;
	}
	rak_peer->DeallocatePacket(packet);
}

ClientThread::ClientThread()
{
    finish = false;    
}

void ClientThread::end()
{
    finish = true;
}

int ClientThread::connect_to_server(string server_name, RakNet::SystemAddress & server_addr)
{	
	RakNet::ConnectionAttemptResult rst = rak_peer->Connect(server_name.c_str(), SERVER_PORT, 0, 0);
	server_addr.FromStringExplicitPort(server_name.c_str(), SERVER_PORT);

	QMutexLocker locker(&mutex);
	map <RakNet::SystemAddress, ActiveConect>::iterator iter = active_conect.find(server_addr);
	switch (rst) {
	case RakNet::INVALID_PARAMETER:
	case RakNet::CANNOT_RESOLVE_DOMAIN_NAME:
		qCritical("connect_to_server %s is invalid param", server_name.c_str());
		return INVALID_NAME;

	case RakNet::ALREADY_CONNECTED_TO_ENDPOINT:
	case RakNet::CONNECTION_ATTEMPT_ALREADY_IN_PROGRESS:
		if (iter == active_conect.end()) { //Not found
			qCritical("New connect exist in Raknet's connection not in active_connect, check connect/disconnect mismatch");
			ActiveConect new_active_connect;
			new_active_connect.active_num = 1;
			active_conect[server_addr] = new_active_connect;
		}
		else {
			iter->second.active_num++;
			qInfo("Use exist connect to %s, addr=%s, active=%d", server_name.c_str(), server_addr.ToString(), iter->second.active_num);
		}
		if (rst == RakNet::ALREADY_CONNECTED_TO_ENDPOINT)
			return CONNECTED;
		else
			return CONNECT_IN_PROGRESS;

	case RakNet::CONNECTION_ATTEMPT_STARTED:
		if (iter == active_conect.end()) { //Not found
			qInfo("New connect to %s, addr=%s", server_name.c_str(), server_addr.ToString());
			ActiveConect new_active_connect;
			new_active_connect.active_num = 1;
			active_conect[server_addr] = new_active_connect;
		}
		else {
			qCritical("New connect not exist in Raknet's connection but in active_connect, check connect/disconnect mismatch");		
			iter->second.active_num = 1;
		}
		return CONNECT_IN_PROGRESS;
	default:
		qCritical("connect_to_server %s unknow error", server_name.c_str());
		return INVALID_NAME;
	}
}

void ClientThread::disconnect_server(RakNet::SystemAddress & server_addr)
{
	QMutexLocker locker(&mutex);
	map <RakNet::SystemAddress, ActiveConect>::iterator iter = active_conect.find(server_addr);
	if (iter == active_conect.end()) { //not found
		qCritical("Disconnect %s not exist in active_connect, check connect/disconnect mismatch", server_addr.ToString());
		return;
	}
	else {
		if (iter->second.active_num==0)
			qCritical("Disconnect %s, but activer=0, check connect/disconnect mismatch", server_addr.ToString());
		else {
			iter->second.active_num--;
			qInfo("reduce server %s active num=%d", server_addr.ToString(), iter->second.active_num);
			if (iter->second.active_num==0)
				iter->second.last_inactive_time = RakNet::GetTimeMS(); //record latest active time for CloseConnection
		}
	}
}

void ClientThread::run()
{
    //char server_ip[] = "127.0.0.1"; //"10.233.140.185";

    RakNet::SocketDescriptor sd;
    sd.socketFamily = AF_INET;    
    qInfo("ClientThread Start net");

    rak_peer = RakNet::RakPeerInterface::GetInstance();
    rak_peer->Startup(15, &sd, 1);    
	check_active_time = RakNet::GetTimeMS();

    while (1)
    {		
		for (RakNet::Packet * packet = rak_peer->Receive(); packet; packet = rak_peer->Receive())
        {
			bool need_delete = false;
            switch (packet->data[0])
            {
            case ID_CONNECTION_REQUEST_ACCEPTED:
			case ID_ALREADY_CONNECTED: 
			case ID_CONNECTION_ATTEMPT_FAILED:
			case ID_NO_FREE_INCOMING_CONNECTIONS:
				emit server_connected(QSharedPointer<RakNet::Packet>(packet, packet_del));
                break;

            case ID_DISCONNECTION_NOTIFICATION:
			case ID_CONNECTION_LOST:
				emit server_disconnected(QSharedPointer<RakNet::Packet>(packet, packet_del));
                break;

            case ID_RESPONSE_OBJ_SEARCH:                
                qInfo("Receive obj search response %d", packet->data[1]);
				emit search_packet_arrive(QSharedPointer<RakNet::Packet>(packet, packet_del));
                break;

            default:
                qCritical("Message identifier %i has arrived.\n", packet->data[0]);
                break;
            }
			if (need_delete)
                rak_peer->DeallocatePacket(packet);
        }
		if (finish && active_conect.empty())
			break;
        RakSleep(10);
		RakNet::TimeMS current_time = RakNet::GetTimeMS();
		if (current_time - check_active_time >= 1000) { //check inactive connection (active_num=0) and close them
			QMutexLocker locker(&mutex);
			map <RakNet::SystemAddress, ActiveConect>::iterator iter;
			for (iter = active_conect.begin(); iter != active_conect.end(); iter++) 
				if (iter->second.active_num==0 && current_time - iter->second.last_inactive_time >= 10000) {
					//if inactive time exceed threshold, close it 
					RakNet::AddressOrGUID server;
					server.systemAddress = iter->first;
					qInfo("close server %s connect", iter->first.ToString());
					rak_peer->CloseConnection(server, true);
					active_conect.erase(iter);
					break;
				}
		}
		if (finish) { //close all connection
			QMutexLocker locker(&mutex);
			map <RakNet::SystemAddress, ActiveConect>::iterator iter;
			for (iter = active_conect.begin(); iter != active_conect.end(); iter++) {
				RakNet::AddressOrGUID server;
				server.systemAddress = iter->first;
				qInfo("close server %s connect", iter->first.ToString());
				rak_peer->CloseConnection(server, true);
			}
			active_conect.clear();
		}
    }

    RakNet::RakPeerInterface::DestroyInstance(rak_peer);
}
