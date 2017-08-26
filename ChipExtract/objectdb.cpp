#include "objectdb.h"
#include <QMutexLocker>
#include <stdio.h>
ObjectDB odb;

bool ElementObj::intersect_rect(const QRect & r)
{
	switch (type) {
	case OBJ_AREA:
        return r.intersects(QRect(p0, p1));

	case OBJ_LINE:
        return r.intersects(QRect(p0, p1));

	case OBJ_POINT:
		return r.contains(p0);
	}
    return false;
}

void AreaObjLink::get_objects(unsigned char t1, unsigned long long t2_mask, const QRect &r, vector<ElementObj *> & rst)
{
    for (int i = 0; i < (int) objs.size(); i++) {
		if (t1 == objs[i]->type && (t2_mask >> objs[i]->type2) & 1)
			if (objs[i]->intersect_rect(r))
				rst.push_back(objs[i]);
	}
}

void AreaObjLink::link_object(ElementObj * pobj)
{
	objs.push_back(pobj);
}

void AreaObjLink::delink_object(ElementObj * pobj)
{
    for (int i = 0; i < (int) objs.size(); i++)
		if (objs[i] == pobj) {
			objs.erase(objs.begin() + i);
			break;
		}
	qCritical("Delete obj (%d,%d) pointer not exist", pobj->type, pobj->type2);

}

void AreaObjLink::clear_all()
{
    for (int i=0; i<(int) objs.size(); i++)
        delete objs[i];
    objs.clear();
    for (int i=0; i<(int) areas.size(); i++)
        areas[i]->clear_all();
}

ObjectDB::ObjectDB()
{
    for (unsigned i = 0; i < (int) sizeof(layer_wire_info) / sizeof(layer_wire_info[0]); i++)
		layer_wire_info[i] = NULL;
    for (unsigned i = 0; i < (int) sizeof(layer_via_info) / sizeof(layer_via_info[0]); i++)
		layer_via_info[i] = NULL;
	root_area = new AreaObjLink;
	root_cell = new AreaObjLink;
	root_wire = new AreaObjLink;
	root_via = new AreaObjLink;
}

void ObjectDB::get_objects(unsigned char t1, unsigned long long t2_mask, const QRect &r, vector<ElementObj *> & rst)
{
	rst.clear();
    QMutexLocker locker(&mutex);
	switch (t1) {
	case OBJ_PARA:
		if (t2_mask & PARA_LINE_WIDTH_MASK) {
            for (unsigned i = 0; i < sizeof(layer_wire_info) / sizeof(layer_wire_info[0]); i++)
				if (layer_wire_info[i])
					rst.push_back(layer_wire_info[i]);
		}
		if (t2_mask & PARA_VIA_RADIUS_MASK) {
            for (unsigned i = 0; i < sizeof(layer_via_info) / sizeof(layer_via_info[0]); i++)
				if (layer_via_info[i])
					rst.push_back(layer_via_info[i]);
		}
		break;

	case OBJ_AREA:
		root_area->get_objects(t1, t2_mask, r, rst);
		break;

	case OBJ_LINE:
        if (t2_mask & (LINE_NORMAL_WIRE0_MASK | LINE_NORMAL_WIRE1_MASK | LINE_NORMAL_WIRE2_MASK
                       | LINE_NORMAL_WIRE3_MASK | LINE_WIRE_AUTO_EXTRACT_MASK))
			root_wire->get_objects(t1, t2_mask, r, rst);
		break;

	case OBJ_POINT:
		if (t2_mask & POINT_CELL_MASK)
			root_cell->get_objects(t1, t2_mask, r, rst);
        if (t2_mask & (POINT_NORMAL_VIA0_MASK | POINT_NORMAL_VIA1_MASK | POINT_NORMAL_VIA2_MASK |
                       POINT_NORMAL_VIA3_MASK | POINT_VIA_AUTO_EXTRACT_MASK))
			root_via->get_objects(t1, t2_mask, r, rst);
		break;
	}
	
	
}

void ObjectDB::add_object(ElementObj & obj)
{
    ElementObj * p_obj = new ElementObj(obj); //TODO add self allocation,
    QMutexLocker locker(&mutex);
	switch (obj.type) {
	case OBJ_AREA:
		root_area->link_object(p_obj);
		break;

	case OBJ_LINE:
        if (obj.type2 >= LINE_NORMAL_WIRE0 && obj.type2 <= LINE_WIRE_AUTO_EXTRACT)
			root_wire->link_object(p_obj);
		break;

	case OBJ_POINT:
		if (obj.type2 == POINT_CELL)
			root_cell->link_object(p_obj);
        if (obj.type2 >= POINT_NORMAL_VIA0 && obj.type2 <= POINT_VIA_AUTO_EXTRACT)
			root_via->link_object(p_obj);
		break;

	case OBJ_PARA:
		switch (obj.type2) {
		case PARA_LINE_WIDTH:
			layer_wire_info[obj.type3] = p_obj;
			break;
		case PARA_VIA_RADIUS:
			layer_via_info[obj.type3] = p_obj;
			break;
		}	
		break;		
    }
}

void ObjectDB::del_object(ElementObj * p_obj)
{
    QMutexLocker locker(&mutex);
	switch (p_obj->type) {
	case OBJ_AREA:
		root_area->delink_object(p_obj);
		break;

	case OBJ_LINE:
		if (p_obj->type2 >= LINE_NORMAL_WIRE0 && p_obj->type2 <= LINE_NORMAL_WIRE3)
			root_wire->delink_object(p_obj);
		break;
	case OBJ_POINT:
		if (p_obj->type2 == POINT_CELL)
			root_cell->delink_object(p_obj);
		if (p_obj->type2 >= POINT_NORMAL_VIA0 && p_obj->type2 <= POINT_NORMAL_VIA3)
			root_via->delink_object(p_obj);
		break;
		
	case OBJ_PARA:
		switch (p_obj->type2) {
		case PARA_LINE_WIDTH:
			if (layer_wire_info[p_obj->type3] != p_obj)
				qCritical("Delete PARA_LINE_WIDTH obj pointer not exist, %d", p_obj->type3);
			else
				layer_wire_info[p_obj->type3] = NULL;
			break;
		case PARA_VIA_RADIUS:
			if (layer_via_info[p_obj->type3] != p_obj)
				qCritical("Delete PARA_VIA_RADIUS obj pointer not exist, %d", p_obj->type3);
			else
				layer_via_info[p_obj->type3] = NULL;
			break;
		}
		break;
	}
	delete p_obj;
}

void ObjectDB::clear_all()
{
    root_area->clear_all();
    root_cell->clear_all();
    root_via->clear_all();
    root_wire->clear_all();
    for (unsigned i = 0; i < (int) sizeof(layer_wire_info) / sizeof(layer_wire_info[0]); i++)
        if (layer_wire_info[i] != NULL) {
            delete layer_wire_info[i];
            layer_wire_info[i] = NULL;
        }
    for (unsigned i = 0; i < (int) sizeof(layer_via_info) / sizeof(layer_via_info[0]); i++)
        if (layer_via_info[i] != NULL) {
            delete layer_via_info[i];
            layer_via_info[i] = NULL;
        }
}

int ObjectDB::load_objets(string file_name)
{
    FILE *fp = fopen(file_name.c_str(), "rt");
    if (fp==NULL)
        return -1;
    ElementObj obj;
    while (!feof(fp)) {
        unsigned t;
        int x0, y0, x1, y1;
        fscanf(fp, "t=%d, (%d,%d)->(%d,%d)\n", &t, &x0, &y0, &x1, &y1);
        obj.p0.setX(x0);
        obj.p0.setY(y0);
        obj.p1.setX(x1);
        obj.p1.setY(y1);
        obj.type3 = t & 0xff;
        obj.type3 = obj.type3+1;//TODO repair me
        obj.type2 = t >> 8 & 0xff;
        obj.type = t >> 16 & 0xff;
        add_object(obj);
    }
    fclose(fp);
    return 0;
}

unsigned ObjectDB::get_wire_width(unsigned char type, unsigned char layer)
{
	if (layer >= 64 || type > 3) {
		qCritical("get_wire_width, invalid layer %d or type %d", layer, type);
		return 0;
	}
	unsigned index = layer * 4 + type;
	if (layer_wire_info[index] == NULL) {
		qCritical("get_wire_width, wire info [%d,%d] ==NULL", layer, type);
		return 0;
	}
		
	return layer_wire_info[index]->un.attach & 0xffff;
}

unsigned ObjectDB::get_via_radius(unsigned char type, unsigned char layer)
{
	if (layer >= 64 || type > 3) {
		qCritical("get_via_radius, invalid layer %d or type %d error", layer, type);
		return 0;
	}
	unsigned index = layer * 4 + type;
	if (layer_via_info[index] == NULL){
		qCritical("get_via_radius, via info [%d,%d] ==NULL", layer, type);
		return 0;
	}
	return layer_via_info[index]->un.attach & 0xffff;
}
