#ifndef OBJECTDB_H
#define OBJECTDB_H
#include "markobj.h"
#include "searchobject.h"
#include <vector>
#include <QRect>
#include <QMutex>
#include <string>
using namespace std;

#define GET_FIELD(var, field) (((var) & field##_MASK) >> field)
#define SET_FIELD(var, field, num) var = (var & ~field##_MASK) | ((unsigned long long)(num) << field & field##_MASK)

struct ElementObj : public MarkObj
{
    union {
        unsigned long long attach;
        void * ptr;
    } un;

	ElementObj() {

	}

    ElementObj(MarkObj &o) : MarkObj(o) {
        un.attach = 0;
    }

    ElementObj(ElementObj &o) {
        *this = o;
    }

	bool intersect_rect(const QRect & r);
};


class AreaObjLink {
protected:
    vector<ElementObj *> objs;
    vector<AreaObjLink *> areas;

public:
	void get_objects(unsigned char t1, unsigned long long t2_mask, const QRect &r, vector<ElementObj *> & rst);
    void get_objects(unsigned char t1, unsigned long long t2_mask, float prob, vector<ElementObj *> & rst);
    void del_objects(unsigned char t1, unsigned long long t2_mask, float prob);
    void link_object(ElementObj * pobj);
    void delink_object(ElementObj * opbj);
    void clear_all();
    ~AreaObjLink();
};

class ObjectDB
{
protected:
    AreaObjLink * root_cell;
	AreaObjLink * root_wire;
	AreaObjLink * root_via;
	AreaObjLink * root_area;
	AreaObjLink * root_train;
    QMutex mutex;

protected:
    ElementObj * layer_wire_info[256];
    ElementObj * layer_via_info[256];

public:
    ObjectDB();
    ~ObjectDB();
	void get_objects(unsigned char t1, unsigned long long t2_mask, const QRect &r, vector<ElementObj *> & rst);
    void get_objects(unsigned char t1, unsigned long long t2_mask, float prob, vector<ElementObj *> & rst);
    void del_objects(unsigned char t1, unsigned long long t2_mask, float prob);
    void add_object(ElementObj & obj);
    void del_object(ElementObj * p_obj);
    void clear_all();
    int load_objets(string file_name);
    unsigned get_wire_width(unsigned char type, unsigned char layer);
    unsigned get_via_radius(unsigned char type, unsigned char layer);
};


#endif // OBJECTDB_H
