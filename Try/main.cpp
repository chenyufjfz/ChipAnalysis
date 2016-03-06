#include "iclayer.h"
#include <iostream>
#include <QRect>
#include <stdio.h>
#include <QBuffer>

using namespace std;
int test_raknet();
int test_lmdb();
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
    //test_lmdb();
    //test_raknet();
    test_iclayer();
}
