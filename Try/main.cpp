#ifdef _DEBUG  
#define New   new(_NORMAL_BLOCK, __FILE__, __LINE__)  
#endif  
#define CRTDBG_MAP_ALLOC    
#include <stdlib.h>    
#include <crtdbg.h>

#include "iclayer.h"
#include <iostream>
#include <QRect>
#include <stdio.h>
#include <QBuffer>


using namespace std;
int test_raknet();
int test_lmdb();
int test_element0();
int test_element1();

int test_element_db0();
int test_element_db1();
int test_element_db2();
int test_cross();
int test_element_draw0();
int test_element_draw1();
void test_iclayer()
{
    QString str = "../../M6.dat";
    ICLayerWr layer(str.toStdString());
    cout << "width:" << layer.getBlockWidth() << endl;

    std::vector<Point> corners;
    layer.getCorners(corners);
    cout << "nums:" << corners.size() << endl;
    for (int i = 0; i < corners.size(); i++)
    {
        cout << "x:" << corners[i].x << "," << corners[i].y<<endl;
    }

    QImage qimage;
    layer.getImgByIdx(qimage, 0, 0);
    qimage.save("a00.jpg", "JPG");
    cout<<"bytes before scale:"<<qimage.byteCount();
    qimage = qimage.scaled(qimage.width()/2, qimage.height()/2);
    cout<<", bytes after scale:"<<qimage.byteCount()<<endl;
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    qimage.save(&buffer, "JPG");
    FILE * fp = fopen("a01.jpg", "wb");
    fwrite(ba.data(), 1, ba.size(), fp);
    fclose(fp);
}

int main()
{
	//_CrtSetBreakAlloc(174);
    //test_lmdb();
    //test_raknet();
    //test_iclayer();
	/*
	test_element1();
	test_element0();	
	test_element_db0();
	test_element_db1();
	test_element_db2();
	test_cross();*/
	
	test_element_draw1();
	_CrtDumpMemoryLeaks();
	getchar();
}
