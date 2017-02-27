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

struct LayerParam {
    int layer; //which layer
    int wire_wd; //wire width
    int via_rd; //via radius
    int grid_wd; //grid width
    float param1; //via th, close to 1, higher threshold
    float param2; //wire th, close to 1, higher threshold
    float param3; //via_cred vs wire_cred, if via_cred> wire_cred, beta>1; else <1
    float param4;
    unsigned long long rule; //rule affect bbfm
    unsigned long long warn_rule; //warnrule affect report
    LayerParam(int _layer, int _wire_wd, int _via_rd, unsigned long long _rule, unsigned long long _warn_rule,
               int _grid_wd, float _param1, float _param2, float _param3, float _param4) {
        layer = _layer;
        wire_wd = _wire_wd;
        via_rd = _via_rd;
        grid_wd = _grid_wd;
        rule = _rule;
        warn_rule = _warn_rule;
        param1 = _param1;
        param2 = _param2;
        param3 = _param3;
        param4 = _param4;
    }
};

struct VWSearchRequest {
    vector<LayerParam> lpa;
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
