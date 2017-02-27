#ifndef CLIENTTHREAD_H
#define CLIENTTHREAD_H
#include <QThread>
#include <QtCore/QSharedPointer>
#include "RakPeerInterface.h"
#include <string>
#include <map>
#include <QMutex>
#include "GetTime.h"
using namespace std;

class ClientThread : public QThread
{
    Q_OBJECT

public:
	enum ConnectState {
		CONNECT_IN_PROGRESS,
		CONNECTED,
		INVALID_NAME
	};
	struct ActiveConect {
		int active_num;
		RakNet::TimeMS last_inactive_time;
	};
    ClientThread();
	/*It is running in caller thread, not ClientThread
	Input: server_name, it can be "10.2.2.1" or "yuchen"
	Output: server_addr, contain IP addr and port
	*/
	int connect_to_server(string server_name, RakNet::SystemAddress & server_addr);
	/*It is running in caller thread, not ClientThread
	Input:server_addr, server for disconnect
	ClientThread maintain a reference counter, when a connection reference counter is 0, 
	connection will be closed after 10 seconds if it is not used again.
	*/
	void disconnect_server(RakNet::SystemAddress & server_addr);
	//It is running in caller thread, not ClientThread
    void end();

signals:
	void search_packet_arrive(QSharedPointer<RakNet::Packet> packet);
	void server_connected(QSharedPointer<RakNet::Packet> packet);
	void server_disconnected(QSharedPointer<RakNet::Packet> packet);	

protected:
    bool finish;
	//active_conect contains all SystemAddress in RakPeer's RemoteSystemStruct
	map <RakNet::SystemAddress, ActiveConect> active_conect;
	QMutex mutex; //protect active_conect
	RakNet::TimeMS check_active_time;
    void run() Q_DECL_OVERRIDE;	
};

#endif // CLIENTTHREAD_H
