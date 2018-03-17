#ifndef SEARCHOBJECT_H
#define SEARCHOBJECT_H

#include <QObject>
#include <vector>
#include <list>
#include <QtCore/QMetaType>
#include <QtCore/QSharedPointer>
#include <QRect>
#include <QAtomicInteger>
#include "communication.hpp"
#include "markobj.h"
#include "RakPeerInterface.h"
#include "extractparam.h"
using namespace std;

struct SearchResults {
    vector<MarkObj> objs;
};
Q_DECLARE_METATYPE(QSharedPointer<SearchResults>)

struct SearchRects {
    vector<unsigned char> dir;
    vector<QRect> rects;
};
Q_DECLARE_METATYPE(QSharedPointer<SearchRects>)

struct VWSearchRequest {
    vector<ParamItem> lpa;
};
Q_DECLARE_METATYPE(QSharedPointer<VWSearchRequest>)

Q_DECLARE_METATYPE(QSharedPointer<RakNet::Packet>)

/*
One view have one SearchObject, 
*/
class SearchObject : public QObject
{
    Q_OBJECT
public:
	typedef enum ChooseServerPolicy {
		RemoteSpecifyLocal,
		RemoteSpecify,
		RemoteLocal,
		RemoteOnly,
		SpecifyRemoteLocal,		
		SpecifyRemote,
		SpecifyLocal,
		SpecifyOnly,
		LocalOnly
	} ChooseServerPolicy;
	typedef enum SearchState {
		Connecting,
		ConnectedToServer
	} SearchState;
	/*
	New created SearchObject is child of pobj, all SearchObject function run in creator thread which call register_new_window
	*/
	static void register_new_window(QObject * pobj, ChooseServerPolicy policy = SpecifyRemoteLocal);
    explicit SearchObject(QObject *parent = 0);
	~SearchObject();

public:
	void set_connect_policy(ChooseServerPolicy policy);
	ChooseServerPolicy get_connect_policy();
	void set_prefer_server(string server);
	string get_prefer_server();

signals:
    void extract_cell_done(QSharedPointer<SearchResults>);
    void extract_wire_via_done(QSharedPointer<SearchResults>);
    void extract_single_wire_done(QSharedPointer<SearchResults>);
	void server_found();
	void server_lost();

public slots:
	void server_connected(QSharedPointer<RakNet::Packet> packet);
	void server_disconnected(QSharedPointer<RakNet::Packet> packet);
	void search_packet_arrive(QSharedPointer<RakNet::Packet> packet);
	/*
	Input: prj, if prj name change, SearchObject may reselect server.
	*/
	void train_cell(string prj, unsigned char l0, unsigned char l1, unsigned char l2, unsigned char l3,
                    unsigned char dir, const QRect rect, float param1, float param2, float param3);
	void extract_cell(string prj, unsigned char l0, unsigned char l1, unsigned char l2, unsigned char l3,
                    QSharedPointer<SearchRects> prect, float param1, float param2, float param3);
	void extract_wire_via(string prj, QSharedPointer<VWSearchRequest> preq, const QRect rect);
    void extract_single_wire(string prj, int layer, int wmin, int wmax, int ihigh, int opt, int gray_th, int channel, int scale, int x, int y);

protected:
	static bool inited;
	static 	QAtomicInteger<unsigned> global_token;
	void send_disconnect();
	/*
	Input: resel, reselect server?
	return 0 means connect, return 1 means in process, return -1 means server not found
	*/
	int try_server(bool resel);	
	void process_req_queue();

protected:	
	struct ReqSearch {
		QSharedPointer <ReqSearchPkt> req_pkt;
		unsigned req_len;
	};
	list <ReqSearch> req_queue;
	RakNet::SystemAddress server_addr;
	ChooseServerPolicy connect_policy;
	string prj_host;
	string prefer_server;
	vector<string> cad_server;
	unsigned cad_idx;
	unsigned token; //toke=0 means not connect to server, after connect to server, token is given a non_zero value
};

#endif // SEARCHOBJECT_H
